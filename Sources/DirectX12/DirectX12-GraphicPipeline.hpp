#ifndef TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H
#define TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BasePipeline.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-DescriptorSet.hpp>
#include <Titanium/Private/FrameTracked.hpp>

namespace TiRHI::DirectX12::Private
{
    class D3D12DescriptorRootSignatureLayout;
}

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class RenderPassDescriptor;

    class GraphicPipeline : public BaseGraphicPipiline<GraphicPipeline, Device, DescriptorSet, RenderPassDescriptor>,
                            public TiRHI::Private::FrameTracked<GraphicPipeline>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        GraphicPipeline() = delete;
        ~GraphicPipeline();
        RHI_MOVE_ONLY(GraphicPipeline);
        explicit GraphicPipeline(Device& device);

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

        bool build();

        std::span<const uint32_t> getStridesPerBinding() const
        {
            return m_stridesPerBinding;
        }

    private:
        friend Private;

        DirectX12::Private::D3D12DescriptorRootSignatureLayout* m_D3D12DescriptorRootSignatureLayout{nullptr};

        MComPtr<ID3D12PipelineState> m_pipelineState;

        std::vector<uint32_t> m_stridesPerBinding;

        std::vector<D3D12_INPUT_ELEMENT_DESC> getInputElement() const;
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H
