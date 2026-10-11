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

    constexpr vk::PipelineStageFlags getPipelineStage(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::Undefined:
            return vk::PipelineStageFlagBits::eTopOfPipe;

        case ResourceState::Common:
            return vk::PipelineStageFlagBits::eAllCommands;

        case ResourceState::VertexBuffer:
        case ResourceState::IndexBuffer:
            return vk::PipelineStageFlagBits::eVertexInput;

        case ResourceState::ConstantBuffer:
        case ResourceState::ShaderResource:
        case ResourceState::UnorderedAccess:
            return vk::PipelineStageFlagBits::eAllCommands;

        case ResourceState::RenderTarget:
            return vk::PipelineStageFlagBits::eColorAttachmentOutput;

        case ResourceState::DepthWrite:
        case ResourceState::DepthRead:
            return vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

        case ResourceState::CopySource:
        case ResourceState::CopyDestination:
            return vk::PipelineStageFlagBits::eTransfer;

        case ResourceState::IndirectArgument:
            return vk::PipelineStageFlagBits::eDrawIndirect;

        case ResourceState::Present:
            return vk::PipelineStageFlagBits::eBottomOfPipe;
        }
        return vk::PipelineStageFlagBits::eAllCommands;
    }

    vk::AccessFlags getAccessMask(ResourceState state);

    vk::ImageLayout getImageLayout(ResourceState state);

} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H
