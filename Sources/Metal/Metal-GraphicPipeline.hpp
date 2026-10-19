#ifndef TITANIUM_METAL_GRAPHIC_PIPELINE_H
#define TITANIUM_METAL_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-BasePipeline.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class RenderPassDescriptor;

    class GraphicPipiline : public BaseGraphicPipiline<GraphicPipiline, RHI, RenderPassDescriptor>
    {
    public:
        GraphicPipiline() = delete;
        ~GraphicPipiline();
        RHI_MOVE_ONLY(GraphicPipiline)
        GraphicPipiline(RHI& rhi);

        bool build();

    private:
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_GRAPHIC_PIPELINE_H