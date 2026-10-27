#ifndef TITANIUM_METAL_GRAPHIC_PIPELINE_H
#define TITANIUM_METAL_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-BasePipeline.hpp>

namespace MTL
{
    class RenderPipelineState;
} // namespace MTL
 
namespace TiRHI::Metal
{
    class RHI;
    class Device;
    class RenderPassDescriptor;

    class GraphicPipeline : public BaseGraphicPipiline<GraphicPipeline, RHI, RenderPassDescriptor>
    {
    public:
        GraphicPipeline() = delete;
        ~GraphicPipeline();
        RHI_MOVE_ONLY(GraphicPipeline)
        GraphicPipeline(RHI& rhi);

        struct Private
        {
            static MTL::RenderPipelineState* getRenderPipelineState(GraphicPipeline& graphicPipeline)
            {
                return graphicPipeline.m_metalRenderPSO;
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        MTL::RenderPipelineState* m_metalRenderPSO{nullptr};
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_GRAPHIC_PIPELINE_H