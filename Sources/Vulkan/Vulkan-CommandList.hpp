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
#include <Vulkan/Private/UploadManager.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class RenderPassDescriptor;
    class GraphicPipeline;
    class DescriptorSet;

    class CommandList : public BaseCommandList<CommandList, RHI, Vulkan::RenderTargets, Vulkan::RenderPassDescriptor,
                                               Vulkan::GraphicPipeline, DescriptorSet>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        CommandList() = delete;
        ~CommandList();
        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;
        CommandList(CommandList&&) noexcept = default;
        CommandList& operator=(CommandList&& other) noexcept;

        struct Private
        {
            static std::array<Vulkan::Private::UploadBatch, MaxFramesInFlight>& getUploadBatch(CommandList& commandlist)
            {
                return commandlist.m_pendingUploads;
            }

            static void setLastSubmissionValue(CommandList& commandlist, uint64_t lastSubmissionValue)
            {
                commandlist.m_lastSubmissionValue = lastSubmissionValue;
            }
        };

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

        void bindDescriptorSet(const DescriptorSet& descriptorSet, uint32_t set);

        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                         uint32_t firstInstance);

        void pushConstant(void* data, uint32_t size);

        void upload2D(Device& device, Texture& texture, std::span<const std::byte> data);

        vk::CommandBuffer getcurrentFrameCmb() const;

    private:
        friend Private;

        vk::UniqueCommandPool m_commandPool;

        std::vector<vk::UniqueCommandBuffer> m_commandBuffer;

        // Keep dispatch functions valid when the Device wrapper moves.
        std::optional<VolkDeviceTable> m_volkTable;

        struct VulkanStorage
        {
            std::vector<vk::ClearValue> clearValues;
        } m_vulkanStorage;

        Device* m_device{nullptr}; // TODO fix this ugly

        std::array<Vulkan::Private::UploadBatch, MaxFramesInFlight> m_pendingUploads;

        uint64_t m_lastSubmissionValue = 0;

        static vk::ImageMemoryBarrier makeImageBarrier(Texture& texture, ResourceState current, ResourceState target);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_COMMAND_LIST_H
