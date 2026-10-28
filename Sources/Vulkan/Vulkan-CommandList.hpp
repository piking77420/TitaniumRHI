#ifndef TITANIUM_VULKAN_COMMAND_LIST_H
#define TITANIUM_VULKAN_COMMAND_LIST_H

#include <optional>
#include <memory_resource>
#include <map>
#include <Volk/volk.h>
#include <Vulkan/vulkan.hpp>
#include <Titanium/RHI-BaseCommandList.hpp>
#include <Vulkan/Vulkan-RenderTargets.hpp>
#include <Vulkan/Vulkan-Texture.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class RenderPassDescriptor;
    class GraphicPipeline;

    class CommandList : public BaseCommandList<CommandList, RHI, Vulkan::RenderTargets, Vulkan::RenderPassDescriptor,
                                               Vulkan::GraphicPipeline>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;
        CommandList(CommandList&&) noexcept = default;
        CommandList& operator=(CommandList&& other) noexcept;

        explicit CommandList(RHI& rhi);

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

        vk::CommandBuffer getcurrentFrameCmb() const;

    private:
        vk::UniqueCommandPool m_commandPool;

        std::vector<vk::UniqueCommandBuffer> m_commandBuffer;

        // Keep dispatch functions valid when the Device wrapper moves.
        std::optional<VolkDeviceTable> m_volkTable;

        struct VulkanStorage
        {
            std::vector<vk::ClearValue> clearValues;
            std::array<std::vector<size_t>, ResourceStateCount * ResourceStateCount> transitionTexture;
        } m_vulkanStorage;

        static vk::DependencyFlags getTransitionDependencyMask(ResourceState current, ResourceState target);

        static vk::ImageMemoryBarrier makeImageBarrier(Texture& texture, ResourceState current, ResourceState target);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_COMMAND_LIST_H
