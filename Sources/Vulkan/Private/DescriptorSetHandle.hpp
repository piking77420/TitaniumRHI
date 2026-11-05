#ifndef TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_SET_HANDLE_H
#define TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_SET_HANDLE_H

#include <array>
#include <cstddef>
#include <Titanium/RHITypes.hpp>
#include <Vulkan/Vulkan-Header.hpp>

namespace TiRHI::Vulkan::Private
{
    struct DescriptorSetHandle
    {
        std::array<vk::DescriptorSet, MaxFramesInFlight> descriptorSets{};
        size_t descriptorCount = 0;
        size_t poolIndex = 0;
    };
} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_SET_HANDLE_H
