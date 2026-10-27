#ifndef TITANIUM_METAL_COMMAND_LIST_H
#define TITANIUM_METAL_COMMAND_LIST_H

#include <Titanium/RHI-BaseCommandList.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class Device;
    class RenderTargets;
    class RenderPassDescriptor;
    class GraphicPipeline;
    class Texture;

    class CommandList : public BaseCommandList<CommandList, RHI, RenderTargets, RenderPassDescriptor, GraphicPipeline>
    {
    public:
        CommandList() = delete;
        ~CommandList();
        CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        bool beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets);

        void endRenderPass();

        void setViewPort(const Viewport& viewPort);

        void setScissors(const Rect2D& rect2d);

        // TODO make some overload for batching barriers
        void transitionResource(Texture& texture, ResourceState state);

        void bindPipeline(const GraphicPipeline& graphicPipeline);

        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                         uint32_t firstInstance);

    private:
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_COMMAND_LIST_H