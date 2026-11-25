#ifndef TITANIUM_VULKAN_COMMAND_LIST_H
#define TITANIUM_VULKAN_COMMAND_LIST_H

#include <optional>
#include <memory_resource>
#include <map>

#include <Vulkan/Vulkan-Header.hpp>
#include <Titanium/RHI-BaseCommandList.hpp>
#include <Vulkan/Vulkan-RenderTargets.hpp>
#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Buffer.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class RenderPassDescriptor;
    class GraphicPipeline;
    class DescriptorSet;
    class Sampler;
    using TrackedResource = std::variant<Texture*, Sampler*, std::pair<Buffer*, uint32_t>, RenderTargets*, GraphicPipeline*,
                     std::pair<DescriptorSet*, uint32_t>, Private::StagingBuffer>;


#define RHI_TRANSITION_MODE_LIST(X)                                                                             \
    X(Automatic)                                                                                                \
    X(Manual)

    enum class TransitionMode : uint8_t
    {
#define X(name) name,
        RHI_TRANSITION_MODE_LIST(X)
#undef X
    };
    IMPLEMENT_TO_STRING_TITANIUM(TransitionMode, RHI_TRANSITION_MODE_LIST)


    class CommandList : public BaseCommandList<CommandList, Device, Vulkan::RenderTargets, Vulkan::RenderPassDescriptor,
                                               Vulkan::GraphicPipeline, DescriptorSet>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        CommandList() = delete;
        ~CommandList();
        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;
        CommandList(CommandList&&) noexcept = default;
        CommandList& operator=(CommandList&& other) noexcept = default;

        explicit CommandList(Device& device);

        bool build();

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        bool beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets);

        void endRenderPass();

        void setViewPort(const Viewport& viewPort);

        void setScissors(const Rect2D& rect2d);

        void transitionResource(Texture& texture, ResourceState state);

        void transitionResource(Buffer& buffer, ResourceState state);

        void bindPipeline(GraphicPipeline& graphicPipeline);

        void bindDescriptorSet(DescriptorSet& descriptorSet, uint32_t set);

        void bindVertexBuffer(uint32_t bindingIndex, uint64_t offset, Buffer& buffer);

        void bindIndexBuffer(Buffer& buffer, uint64_t offset, IndexType format);

        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                         uint32_t firstInstance);

        void pushConstant(void* data, uint32_t size);

        void upload2d(Texture& texture, std::span<const std::byte> data,
                      TransitionMode transitionMode = TransitionMode::Automatic);

        void uploadBuffer(Buffer& buffer, uint64_t offset, std::span<const std::byte> data,
                          TransitionMode transitionMode = TransitionMode::Automatic);

        vk::CommandBuffer getcurrentFrameCmb() const;

        void markResourceFrame(uint64_t submisionValue);

    private:
        std::vector<vk::UniqueCommandBuffer> m_commandBuffer;

        std::vector<TrackedResource> m_trackedResources;

        struct VulkanStorage
        {
            std::vector<vk::ClearValue> clearValues;
        } m_vulkanStorage;

        std::array<uint64_t, MaxFramesInFlight> m_lastSubmitedFrames;

        static vk::ImageMemoryBarrier makeImageBarrier(Texture& texture, ResourceState current, ResourceState target);

        static vk::BufferMemoryBarrier makeBufferBarrier(Buffer& buffer, ResourceState current, ResourceState target);
        
        void trackResource(Texture& texture);

        void trackResource(Sampler& sampler);

        void trackResource(Buffer& buffer);

        void trackResource(GraphicPipeline& graphicPipeline);

        void trackResource(RenderTargets& renderTargets);

        void trackResource(DescriptorSet& descriptorSet);

        void trackResource(Private::StagingBuffer&& stagingBuffer);

        void trackResource(Private::StagingBuffer& stagingBuffer) = delete;

        void trackResource(const Private::StagingBuffer& stagingBuffer) = delete;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_COMMAND_LIST_H
