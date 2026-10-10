#include <Private/RHIToVulkan.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan::Private
{
    vk::Format toVulkan(TiRHI::Format format)
    {
        switch (format)
        {
        case TiRHI::Format::R8_UNorm:
            return vk::Format::eR8Unorm;

        case TiRHI::Format::R8G8_UNorm:
            return vk::Format::eR8G8Unorm;

        case TiRHI::Format::R8G8B8A8_UNorm:
            return vk::Format::eR8G8B8A8Unorm;

        case TiRHI::Format::R8G8B8A8_UNorm_SRGB:
            return vk::Format::eR8G8B8A8Srgb;

        case TiRHI::Format::B8G8R8A8_UNorm:
            return vk::Format::eB8G8R8A8Unorm;

        case TiRHI::Format::B8G8R8A8_UNorm_SRGB:
            return vk::Format::eB8G8R8A8Srgb;

        case TiRHI::Format::R16_Float:
            return vk::Format::eR16Sfloat;

        case TiRHI::Format::R16G16_Float:
            return vk::Format::eR16G16Sfloat;

        case TiRHI::Format::R16G16B16A16_Float:
            return vk::Format::eR16G16B16A16Sfloat;

        case TiRHI::Format::R32_Float:
            return vk::Format::eR32Sfloat;

        case TiRHI::Format::R32G32_Float:
            return vk::Format::eR32G32Sfloat;

        case TiRHI::Format::R32G32B32A32_Float:
            return vk::Format::eR32G32B32A32Sfloat;

        case TiRHI::Format::R32_UInt:
            return vk::Format::eR32Uint;

        case TiRHI::Format::R32G32_UInt:
            return vk::Format::eR32G32Uint;

        case TiRHI::Format::R32G32B32A32_UInt:
            return vk::Format::eR32G32B32A32Uint;

        case TiRHI::Format::D16_UNorm:
            return vk::Format::eD16Unorm;

        case TiRHI::Format::D24_UNorm_S8_UInt:
            return vk::Format::eD24UnormS8Uint;

        case TiRHI::Format::D32_Float:
            return vk::Format::eD32Sfloat;

        case TiRHI::Format::D32_Float_S8_UInt:
            return vk::Format::eD32SfloatS8Uint;

        default:
            RHI_LOG_ERROR(L"Unsupported Vulkan format", RhiApi::Vulkan);
            return vk::Format::eUndefined;
        }
    }

    vk::ImageLayout toVulkanImageLayout(ResourceState resourceState)
    {
        switch (resourceState)
        {
        case ResourceState::Undefined:
            return vk::ImageLayout::eUndefined;

        case ResourceState::Common:
            return vk::ImageLayout::eGeneral;

        case ResourceState::ShaderResource:
            return vk::ImageLayout::eShaderReadOnlyOptimal;

        case ResourceState::UnorderedAccess:
            return vk::ImageLayout::eGeneral;

        case ResourceState::RenderTarget:
            return vk::ImageLayout::eColorAttachmentOptimal;

        case ResourceState::DepthWrite:
            return vk::ImageLayout::eDepthStencilAttachmentOptimal;

        case ResourceState::DepthRead:
            return vk::ImageLayout::eDepthStencilReadOnlyOptimal;

        case ResourceState::CopySource:
            return vk::ImageLayout::eTransferSrcOptimal;

        case ResourceState::CopyDestination:
            return vk::ImageLayout::eTransferDstOptimal;

        case ResourceState::Present:
            return vk::ImageLayout::ePresentSrcKHR;

        // Buffer-only states: invalid for VkImageLayout
        case ResourceState::VertexBuffer:
        case ResourceState::IndexBuffer:
        case ResourceState::ConstantBuffer:
        case ResourceState::IndirectArgument:
            assert(false && "Buffer ResourceState has no Vulkan image layout");
            RHI_LOG_ERROR(L"Buffer ResourceState has no Vulkan image layout", RhiApi::Vulkan);
            return vk::ImageLayout::eUndefined;

        default:
            assert(false && "Unsupported ResourceState");
            RHI_LOG_ERROR(
                std::format(L"Unsupported ResourceState {}", std::to_wstring(static_cast<int>(resourceState))),
                RhiApi::Vulkan);
            return vk::ImageLayout::eUndefined;
        }
    }

    vk::AttachmentLoadOp toVulkanAttachementLoadOp(LoadOp loadOp)
    {
        switch (loadOp)
        {
        case TiRHI::LoadOp::LoadOp:
            return vk::AttachmentLoadOp::eLoad;
        case TiRHI::LoadOp::Clear:
            return vk::AttachmentLoadOp::eClear;
        case TiRHI::LoadOp::DontCare:
            return vk::AttachmentLoadOp::eDontCare;
        default:
            RHI_LOG_ERROR(L"Unsupported Vulkan AttachmentLoadOp", RhiApi::Vulkan);
            return vk::AttachmentLoadOp::eNone;
        }

        return vk::AttachmentLoadOp::eNone;
    }

    vk::AttachmentStoreOp toVulkanAttachementStoreOp(StoreOp storeOp)
    {
        switch (storeOp)
        {
        case TiRHI::StoreOp::Store:
            return vk::AttachmentStoreOp::eStore;
        case TiRHI::StoreOp::DontCare:
            return vk::AttachmentStoreOp::eNone;
        case TiRHI::StoreOp::None:
            return vk::AttachmentStoreOp::eNone;
        default:
            RHI_LOG_ERROR(L"Unsupported Vulkan AttachmentLoadOp", RhiApi::Vulkan);
            return vk::AttachmentStoreOp::eNone;
        }

        return vk::AttachmentStoreOp::eNone;
    }

    vk::PipelineBindPoint toPipelineBindPoint(PipelineType type)
    {
        switch (type)
        {
        case PipelineType::Graphics:
            return vk::PipelineBindPoint::eGraphics;

        case PipelineType::Compute:
            return vk::PipelineBindPoint::eCompute;

        case PipelineType::RayTracing:
            return vk::PipelineBindPoint::eRayTracingKHR;
        }

        RHI_LOG_ERROR(std::format(L"Unsupported Vulkan PipelineBindPoint {}", static_cast<int>(type)), RhiApi::Vulkan);
        return vk::PipelineBindPoint::eGraphics;
    }
}
