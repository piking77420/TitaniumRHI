#ifndef TITANIUM_VULKAN_HEADER_H
#define TITANIUM_VULKAN_HEADER_H

#include <vulkan/vulkan.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan
{
    [[nodiscard]] constexpr const char* getResultDescription(vk::Result result)
    {
        switch (result)
        {
        case vk::Result::eSuccess:
            return "Command successfully completed";

        case vk::Result::eNotReady:
            return "A fence or query has not yet completed";

        case vk::Result::eTimeout:
            return "A wait operation has not completed in the specified time";

        case vk::Result::eEventSet:
            return "An event is signaled";

        case vk::Result::eEventReset:
            return "An event is unsignaled";

        case vk::Result::eIncomplete:
            return "A return array was too small for the result";

        case vk::Result::eErrorOutOfHostMemory:
            return "A host memory allocation has failed";

        case vk::Result::eErrorOutOfDeviceMemory:
            return "A device memory allocation has failed";

        case vk::Result::eErrorInitializationFailed:
            return "Initialization of an object failed";

        case vk::Result::eErrorDeviceLost:
            return "The logical or physical device has been lost";

        case vk::Result::eErrorMemoryMapFailed:
            return "Mapping of a memory object failed";

        case vk::Result::eErrorLayerNotPresent:
            return "A requested layer is not present";

        case vk::Result::eErrorExtensionNotPresent:
            return "A requested extension is not supported";

        case vk::Result::eErrorFeatureNotPresent:
            return "A requested feature is not supported";

        case vk::Result::eErrorIncompatibleDriver:
            return "The Vulkan driver is incompatible";

        case vk::Result::eErrorTooManyObjects:
            return "Too many objects of this type have been created";

        case vk::Result::eErrorFormatNotSupported:
            return "The requested format is not supported";

        case vk::Result::eErrorFragmentedPool:
            return "Descriptor pool allocation failed due to fragmentation";

        case vk::Result::eErrorUnknown:
            return "An unknown Vulkan error occurred";

        case vk::Result::eSuboptimalKHR:
            return "The swapchain no longer matches the surface exactly but can still be used";

        case vk::Result::eErrorSurfaceLostKHR:
            return "The Vulkan surface has been lost";

        case vk::Result::eErrorNativeWindowInUseKHR:
            return "The native window is already in use";

        case vk::Result::eErrorOutOfDateKHR:
            return "The swapchain is out of date";

        default:
            return "Unknown Vulkan result";
        }
    }

    bool VulkanCheckErrorStatus(vk::Result result)
    {
        if (result == vk::Result::eSuccess)
            return false;

        RHI_LOG_ERROR(std::format(L"Vulkan error: {}", getResultDescription(result)), RhiApi::Vulkan);

        return true;
    }

    bool VulkanCheckErrorStatus(VkResult result)
    {
        return VulkanCheckErrorStatus(static_cast<vk::Result>(result));
    }

}

#endif // TITANIUM_VULKAN_HEADER_H
