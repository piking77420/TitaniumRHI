#ifndef TITANIUM_VULKAN_FUNCTION_H
#define TITANIUM_VULKAN_FUNCTION_H

#include <vulkan/vulkan.hpp>

namespace TiRHI::Vulkan
{
    struct VulkanFunctions
    {
        PFN_vkCmdBeginDebugUtilsLabelEXT beginDebugLabel = nullptr;
        PFN_vkCmdEndDebugUtilsLabelEXT endDebugLabel = nullptr;
        PFN_vkCmdInsertDebugUtilsLabelEXT insertDebugLabel = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT setObjectName = nullptr;
    };

} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_FUNCTION_H
