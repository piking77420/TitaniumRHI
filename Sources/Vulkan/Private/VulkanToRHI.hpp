#ifndef TITANIUM_VULKAN_PRIVATE_VULKAN_TO_RHI_H
#define TITANIUM_VULKAN_PRIVATE_VULKAN_TO_RHI_H

#include <vulkan/vulkan.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::Vulkan::Private
{
    Format toRhi(vk::Format format);
} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_VULKAN_TO_RHI_H
