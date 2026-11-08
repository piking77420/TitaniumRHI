#include <DirectX12/DirectX12-GraphicPipeline.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12
{
    GraphicPipeline::GraphicPipeline(RHI& rhi)
        : BaseGraphicPipiline(rhi)
    {
    }

    bool GraphicPipeline::build(Device& device)
    {
        if (!device.isValid())
            return false;

        std::span<const BindingLayout> bindingsLayout = getLayouts();
        auto& descriptorManager = Device::Private::getDescriptorManager(device);

        if (!bindingsLayout.empty())
        {
            m_D3D12DescriptorRootSignatureLayout =
                descriptorManager.getOrCreateD3D12DescriptorRootSignatureLayout(bindingsLayout);
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

        desc.VS = {vertexModule->data.get(), vertexModule->size};

        desc.PS = {pixel->data.get(), pixel->size};

        desc.InputLayout = {nullptr, 0};

        desc.PrimitiveTopologyType = DirectX12::Private::toDirectX12TopologyType(getPrimitiveTopology());
        desc.RasterizerState.FillMode = DirectX12::Private::toDirectX12(getPolygoneMode());
        desc.RasterizerState.CullMode = DirectX12::Private::toDirectX12(getCullFlagMode());
        desc.RasterizerState.FrontCounterClockwise = DirectX12::Private::toDirectX12(getCullFlagMode());
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

        desc.DepthStencilState.DepthEnable = FALSE;
        desc.DepthStencilState.StencilEnable = FALSE;

        // TODO use color attachelment

        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.DSVFormat = DXGI_FORMAT_UNKNOWN;

        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        return SUCCEEDED(device.getNativeDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState)));
    }
} // namespace TiRHI::DirectX12
