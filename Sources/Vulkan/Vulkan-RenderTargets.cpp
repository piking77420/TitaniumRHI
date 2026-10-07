#include <Vulkan/Vulkan-RenderTargets.hpp>

#include <vulkan/vulkan.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    bool RenderTargets::build(Device& device, const RenderPassDescriptor& renderPassDescriptor)
    {
        return true;
    }

} // TiRHI::Vulkan
