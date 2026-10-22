#include <Metal/Metal-CommandList.hpp>
#include <Metal/Metal-Device.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-Texture.hpp>
#include <Metal/Metal-RenderPassDescriptor.hpp>

namespace TiRHI::Metal
{
    CommandList::CommandList(RHI& rhi)
        : BaseCommandList(rhi)
    {
    }

    CommandList::~CommandList() = default;

    bool CommandList::build(Device& device)
    {
        return true;
    }

    bool CommandList::beginRecord()
    {
        return BaseCommandList::onBeginRecord();
    }

    bool CommandList::endRecord()
    {
        BaseCommandList::onEndRecord();
        return true;
    }

    void CommandList::beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color)
    {
    }

    void CommandList::endDebugLabel()
    {
    }

    bool CommandList::beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets)
    {
        return BaseCommandList::onBeginRenderPass(beginRenderPass, renderTargets);
    }

    void CommandList::endRenderPass()
    {
    }

    void CommandList::setViewPort(const Viewport& viewPort)
    {
    }

    void CommandList::setScissors(const Rect2D& rect2d)
    {
    }

    void CommandList::transitionResource(Texture& texture, ResourceState state)
    {
    }

    void CommandList::bindPipeline(const GraphicPipeline& graphicPipeline)
    {
    }

    void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
    }

    void CommandList::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                  int32_t vertexOffset, uint32_t firstInstance)
    {
    }

} // namespace TiRHI::Metal