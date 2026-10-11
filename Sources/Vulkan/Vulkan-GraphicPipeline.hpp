#ifndef TITANIUM_VULKAN_GRAPHIC_PIPELINE_H
#define TITANIUM_VULKAN_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-BasePipeline.hpp>
#include <Vulkan/Vulkan-Header.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class RenderPassDescriptor;

    class GraphicPipeline : public BaseGraphicPipiline<GraphicPipeline, RHI, RenderPassDescriptor>
    {
    public:
        GraphicPipeline() = delete;
        ~GraphicPipeline() = default;
        RHI_MOVE_ONLY(GraphicPipeline)
        GraphicPipeline(RHI& rhi)
            : BaseGraphicPipiline(rhi)
        {
        }

        struct Private
        {
            static vk::Pipeline getPipeline(const GraphicPipeline& pipeline)
            {
                return pipeline.m_pipeline;
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        vk::PipelineLayout m_pipelineLayout;

        vk::Pipeline m_pipeline;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_GRAPHIC_PIPELINE_H
