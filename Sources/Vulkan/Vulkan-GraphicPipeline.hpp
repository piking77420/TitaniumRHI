#ifndef TITANIUM_VULKAN_GRAPHIC_PIPELINE_H
#define TITANIUM_VULKAN_GRAPHIC_PIPELINE_H

#include <Titanium/RHI-BasePipeline.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Vulkan-DescriptorSet.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class RenderPassDescriptor;

    class GraphicPipeline : public BaseGraphicPipiline<GraphicPipeline, RHI, DescriptorSet, RenderPassDescriptor>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        GraphicPipeline() = delete;
        ~GraphicPipeline();
        GraphicPipeline(const GraphicPipeline&) = delete;
        GraphicPipeline& operator=(const GraphicPipeline&) = delete;
        GraphicPipeline(GraphicPipeline&&) noexcept;
        GraphicPipeline& operator=(GraphicPipeline&&) noexcept;

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

            static vk::PipelineLayout getPipelineLayout(const GraphicPipeline& pipeline)
            {
                return pipeline.m_pipelineLayout;
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        vk::Device m_device = VK_NULL_HANDLE;

        vk::PipelineLayout m_pipelineLayout;

        vk::Pipeline m_pipeline;

        void destroy();
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_GRAPHIC_PIPELINE_H
