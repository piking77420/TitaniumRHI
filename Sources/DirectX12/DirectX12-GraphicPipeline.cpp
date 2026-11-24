#include <DirectX12/DirectX12-GraphicPipeline.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12
{
    GraphicPipeline::~GraphicPipeline()
    {
        auto& deferredDeletionQueue = Device::Private::getDeferredDeletionQueue(getDevice());
        deferredDeletionQueue.retire(getLastFrameUsed(),
                                     [pipelineState = std::move(m_pipelineState)]() mutable { pipelineState.Reset(); });
    }

    GraphicPipeline::GraphicPipeline(Device& device)
        : BaseGraphicPipiline(device)
    {
    }

    bool GraphicPipeline::build()
    {
        if (!sanityCheck())
            return false;

        std::span<const DescriptorSetLayoutEntry> descriptorSetLayoutEntries = getDescriptorSetLayoutEntries();
        auto& descriptorManager = Device::Private::getDescriptorManager(getDevice());

        if (!descriptorSetLayoutEntries.empty() || getPushConstant().has_value() ||
            getVertexBindingDescription().empty() || getVertexInputAttributeDescription().empty())
        {
            const bool allowInputAssembler =
                !getVertexBindingDescription().empty() || !getVertexInputAttributeDescription().empty();

            m_D3D12DescriptorRootSignatureLayout = descriptorManager.getOrCreateD3D12DescriptorRootSignatureLayout(
                descriptorSetLayoutEntries, getPushConstant(), allowInputAssembler);
        }
        else
        {
            m_D3D12DescriptorRootSignatureLayout = descriptorManager.getEmptyDescriptorRootSignatureLayout();
        }

        // Graphics pipeline
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
        desc.pRootSignature =
            m_D3D12DescriptorRootSignatureLayout ? m_D3D12DescriptorRootSignatureLayout->rootSignature.Get() : nullptr;

        const BinaryModule* vertexModule = nullptr;

        if (!getModule(&vertexModule, ShaderStage::Vertex))
            return false;

        const BinaryModule* pixel = nullptr;

        if (!getModule(&pixel, ShaderStage::Pixel))
            return false;

        desc.InputLayout = {nullptr, 0};
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements = getInputElement();
        desc.InputLayout.NumElements = static_cast<UINT>(inputElements.size());
        desc.InputLayout.pInputElementDescs = inputElements.data();

        // Compute stride when binding vertexBuffer
        m_stridesPerBinding.clear();
        for (const auto& binding : getVertexBindingDescription())
        {
            if (binding.binding >= m_stridesPerBinding.size())
                m_stridesPerBinding.resize(binding.binding + 1, 0);

            m_stridesPerBinding[binding.binding] = binding.stride;
        }

        desc.VS = {vertexModule->data.get(), vertexModule->size};

        desc.PS = {pixel->data.get(), pixel->size};

        desc.PrimitiveTopologyType = DirectX12::Private::toDirectX12TopologyType(getPrimitiveTopology());
        desc.RasterizerState.FillMode = DirectX12::Private::toDirectX12(getPolygoneMode());
        desc.RasterizerState.CullMode = DirectX12::Private::toDirectX12(getCullFlagMode());
        desc.RasterizerState.FrontCounterClockwise = DirectX12::Private::toDirectX12(getFrontFace());
        desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        desc.RasterizerState.DepthClipEnable = TRUE;

        desc.BlendState.AlphaToCoverageEnable = FALSE;
        desc.BlendState.IndependentBlendEnable = FALSE;

        auto& blend = desc.BlendState.RenderTarget[0];
        blend.BlendEnable = FALSE;
        blend.LogicOpEnable = FALSE;
        blend.SrcBlend = D3D12_BLEND_ONE;
        blend.DestBlend = D3D12_BLEND_ZERO;
        blend.BlendOp = D3D12_BLEND_OP_ADD;
        blend.SrcBlendAlpha = D3D12_BLEND_ONE;
        blend.DestBlendAlpha = D3D12_BLEND_ZERO;
        blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend.LogicOp = D3D12_LOGIC_OP_NOOP;
        blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        desc.SampleMask = UINT_MAX;
        const RenderPassDescriptor* renderPassDescriptor = getRenderPassDescriptor();

        desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        desc.DepthStencilState.DepthEnable = FALSE;
        desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        desc.DepthStencilState.StencilEnable = FALSE;

        if (const auto depthAttachment = renderPassDescriptor->getDepthAttachement())
        {
            desc.DSVFormat = DirectX12::Private::toDirectX12(depthAttachment->format);

            desc.DepthStencilState.DepthEnable = TRUE;
            desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc.DepthStencilState.DepthFunc = DirectX12::Private::toDirectX12(getDepthCompareOp());
        }
        assert(renderPassDescriptor->getColorAttachements().size() < 8);
        desc.NumRenderTargets = static_cast<UINT>(renderPassDescriptor->getColorAttachements().size());
        for (size_t i = 0; i < renderPassDescriptor->getColorAttachements().size(); i++)
        {
            desc.RTVFormats[i] =
                DirectX12::Private::toDirectX12(renderPassDescriptor->getColorAttachements()[i].format);
            // TODO
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
        }

        const HRESULT hr = getDevice().getNativeDevice()->CreateGraphicsPipelineState(
            &desc, IID_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf()));

        return SUCCEEDED(hr);
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> GraphicPipeline::getInputElement() const
    {
        const std::span<const VertexBindingDescription> bindings = getVertexBindingDescription();

        const std::span<const VertexInputAttributeDescription> attributes = getVertexInputAttributeDescription();

        std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
        elements.reserve(attributes.size());

        for (const VertexInputAttributeDescription& attribute : attributes)
        {
            const auto it = std::find_if(bindings.begin(), bindings.end(), [&](const VertexBindingDescription& binding)
                                         { return binding.binding == attribute.binding; });

            if (it == bindings.end())
            {
                RHI_LOG_ERROR(std::format(L"[DirectX12] Graphics pipeline '{}': "
                                          L"Vertex attribute location {} references "
                                          L"undefined binding {}.",
                                          this->getNameW(), attribute.location, attribute.binding),
                              Api);

                return {};
            }

            D3D12_INPUT_ELEMENT_DESC element{};
            element.SemanticName = "TEXCOORD";
            element.SemanticIndex = attribute.location;
            element.Format = DirectX12::Private::toDirectX12(attribute.format);
            element.InputSlot = attribute.binding;
            element.AlignedByteOffset = attribute.offset;
            element.InputSlotClass = DirectX12::Private::toDirectX12(it->vertexInputRate);
            element.InstanceDataStepRate = DirectX12::Private::toDirectX12(it->vertexInputRate);

            elements.push_back(element);
        }

        return elements;
    }

} // namespace TiRHI::DirectX12
