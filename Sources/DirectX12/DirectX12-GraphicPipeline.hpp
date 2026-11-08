#ifndef TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H
#define TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BasePipeline.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-DescriptorSet.hpp>

namespace TiRHI::DirectX12::Private
{
    class D3D12DescriptorRootSignatureLayout;
}

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class RenderPassDescriptor;

    class GraphicPipeline : public BaseGraphicPipiline<GraphicPipeline, RHI, DescriptorSet, RenderPassDescriptor>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        GraphicPipeline() = delete;
        ~GraphicPipeline() = default;
        RHI_MOVE_ONLY(GraphicPipeline);
        explicit GraphicPipeline(RHI& rhi);

        struct Private
        {
            static DirectX12::Private::D3D12DescriptorRootSignatureLayout*
            getD3D12DescriptorRootSignatureLayout(const GraphicPipeline& graphicPipeline)
            {
                return graphicPipeline.m_D3D12DescriptorRootSignatureLayout;
            }

            static ID3D12PipelineState* getPipelineState(const GraphicPipeline& graphicPipeline)
            {
                return graphicPipeline.m_pipelineState.Get();
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        DirectX12::Private::D3D12DescriptorRootSignatureLayout* m_D3D12DescriptorRootSignatureLayout{nullptr};

        MComPtr<ID3D12PipelineState> m_pipelineState;
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H
