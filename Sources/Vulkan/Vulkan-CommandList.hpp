#ifndef TITANIUM_VULKAN_COMMAND_LIST_H
#define TITANIUM_VULKAN_COMMAND_LIST_H

#include <optional>
#include <memory_resource>
#include <Volk/volk.h>
#include <Vulkan/vulkan.hpp>
#include <Titanium/RHI-BaseCommandList.hpp>
#include <Vulkan/Vulkan-RenderTargets.hpp>
#include <Vulkan/Vulkan-Texture.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class CommandList : public BaseCommandList<CommandList, RHI, Vulkan::RenderTargets>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        RHI_MOVE_ONLY(CommandList)
        explicit CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        bool beginRenderPass(const BeginRenderPass& beginRenderPass, const RenderTargets& renderTargets);

        void endRenderPass();

        void setViewPort(const Viewport& viewPort);

        void setScissors(const Rect2D& rect2d);

        void transitionResource(Texture& texture, ResourceState state) const ;

        vk::CommandBuffer getcurrentFrameCmb() const;

    private:
        vk::UniqueCommandPool m_commandPool;

        std::vector<vk::UniqueCommandBuffer> m_commandBuffer;

        const VolkDeviceTable* m_volkTable = nullptr;

        struct VulkanStorage
        {
            std::vector<vk::ClearValue> clearValues;
        } m_vulkanStorage;

        static vk::PipelineStageFlags getTransitionSrcMask(ResourceState current);

        static vk::PipelineStageFlags getTransitionDstMask(ResourceState target);

        static vk::DependencyFlags getTransitionDependencyMask(ResourceState current, ResourceState target);

        static vk::ImageMemoryBarrier makeImageBarrier(Texture& texture, ResourceState current, ResourceState target);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_COMMAND_LIST_H
