#ifndef TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H
#define TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H

#include <vulkan/vulkan.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::Vulkan::Private
{
    vk::Format toVulkan(Format format);

    vk::ImageLayout toVulkanImageLayout(ResourceState resourceState);

    vk::AttachmentLoadOp toVulkanAttachementLoadOp(LoadOp loadOp);

    vk::AttachmentStoreOp toVulkanAttachementStoreOp(StoreOp storeOp);

    vk::PipelineBindPoint toPipelineBindPoint(PipelineType type);

    vk::PipelineStageFlags getPipelineStage(ResourceState state);

    vk::AccessFlags getAccessMask(ResourceState state);

    vk::ImageLayout getImageLayout(ResourceState state);

} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H
