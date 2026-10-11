#ifndef TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H
#define TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-BasePipeline.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class GraphicPipeline : public BaseGraphicPipiline<GraphicPipeline, RHI>
    {
    public:
        GraphicPipeline() = delete;
        ~GraphicPipeline() = default;
        RHI_MOVE_ONLY(GraphicPipeline)
        explicit GraphicPipeline(RHI& rhi);

        struct Private
        {
            static ID3D12RootSignature* getSignature(const GraphicPipeline& graphicPipeline)
            {
                return graphicPipeline.m_rootSignature.Get();
            }

            static ID3D12PipelineState* getPipelineState(const GraphicPipeline& graphicPipeline)
            {
                return graphicPipeline.m_pipelineState.Get();
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        MComPtr<ID3D12RootSignature> m_rootSignature;

        MComPtr<ID3D12PipelineState> m_pipelineState;
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_GRAPHIC_PIPELINE_H
