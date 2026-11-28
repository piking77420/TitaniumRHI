#ifndef TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H
#define TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H

#if defined(_WIN32)
#define NOMINMAX
#endif

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
        using Stage = vk::PipelineStageFlagBits;

        switch (state)
        {
        case ResourceState::Undefined:
            return Stage::eTopOfPipe;

        case ResourceState::Common:
            return Stage::eAllCommands;

        case ResourceState::VertexBuffer:
        case ResourceState::IndexBuffer:
            return Stage::eVertexInput;

        case ResourceState::ConstantBuffer:
        case ResourceState::ShaderResource:
        case ResourceState::UnorderedAccess:
            return Stage::eAllGraphics | Stage::eComputeShader;

        case ResourceState::RenderTarget:
            return Stage::eColorAttachmentOutput;

        case ResourceState::DepthWrite:
        case ResourceState::DepthRead:
            return Stage::eEarlyFragmentTests | Stage::eLateFragmentTests;

        case ResourceState::CopySource:
        case ResourceState::CopyDestination:
            return Stage::eTransfer;

        case ResourceState::IndirectArgument:
            return Stage::eDrawIndirect;

        case ResourceState::Present:
            return Stage::eBottomOfPipe;

        default:
            return Stage::eAllCommands;
        }
    }

    vk::AccessFlags getAccessMask(ResourceState state);

    vk::ImageLayout getImageLayout(ResourceState state);

    vk::PrimitiveTopology toVulkan(PrimitiveTopology primitiveTopology);

    vk::PolygonMode toVulkan(PolygonMode polygoneMode);

    vk::CullModeFlags toVulkan(CullModeFlag cullModeFlag);

    vk::FrontFace toVulkan(FrontFace frontFace);

    vk::Filter toVulkan(Filter filter);

    vk::SamplerMipmapMode toVulkanSamplerMipMapMode(Filter filter);

    vk::SamplerAddressMode toVulkan(SamplerAddressMode samplerAddressMode);

    vk::CompareOp toVulkan(CompareOp compareOp);

    vk::ShaderStageFlags toVulkan(ShaderStageFlags shaderStageFlags);

    vk::ImageUsageFlags toVulkan(TextureFlags textureFlags, vk::ImageAspectFlags aspectFlag);

    vk::ImageAspectFlags imageAspectFlagBitsFromFormat(Format format);

    vk::BufferUsageFlags toVulkan(BufferUsage usage);

    vk::VertexInputRate toVulkan(VertexInputRate vertexInputRate);

    vk::VertexInputBindingDescription toVulkan(const VertexBindingDescription& vertexBindingDescription);

    vk::VertexInputAttributeDescription
    toVulkan(const VertexInputAttributeDescription& vertexInputAttributeDescription);

    vk::IndexType toVulkan(IndexType indexType);

} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_RHI_TO_VULKAN_H
