#include <DirectX12/DirectX12-GraphicPipeline.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    GraphicPipeline::GraphicPipeline(RHI& rhi)
        : BaseGraphicPipiline(rhi)
    {
    }

    bool GraphicPipeline::build(Device& device)
    {

        // Empty root signature
        D3D12_ROOT_SIGNATURE_DESC rootDesc{};
        rootDesc.NumParameters = 0;
        rootDesc.pParameters = nullptr;
        rootDesc.NumStaticSamplers = 0;
        rootDesc.pStaticSamplers = nullptr;
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        // Serialize and create the root signature
        MComPtr<ID3DBlob> signatureBlob;
        MComPtr<ID3DBlob> errorBlob;

        HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        if (FAILED(hr))
            return false;

        hr = device.getNativeDevice()->CreateRootSignature(
            0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

        if (FAILED(hr))
            return false;

        // Graphics pipeline
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

        desc.pRootSignature = m_rootSignature.Get();

        const BinaryModule* vertexModule = nullptr;

        if (!getModule<ShaderStage::Vertex>(&vertexModule))
            return false;

        const BinaryModule* pixel = nullptr;

        if (!getModule<ShaderStage::Pixel>(&pixel))
            return false;

        desc.VS = {vertexModule->data.get(), vertexModule->size};

        desc.PS = {pixel->data.get(), pixel->size};

        desc.InputLayout = {nullptr, 0};

        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        desc.RasterizerState.FrontCounterClockwise = FALSE;
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

        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.DSVFormat = DXGI_FORMAT_UNKNOWN;

        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        return SUCCEEDED(device.getNativeDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pipelineState)));
    }
} // namespace TiRHI::DirectX12
