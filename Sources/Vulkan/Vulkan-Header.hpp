#ifndef TITANIUM_VULKAN_HEADER_H
#define TITANIUM_VULKAN_HEADER_H

#include <vulkan/vulkan.hpp>
#include <string_view>
#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan
{
    [[nodiscard]] constexpr inline std::string_view getResultDescription(vk::Result result)
    {
        using namespace std::literals;

        switch (result)
        {
        case vk::Result::eSuccess:
            return "Command successfully completed"sv;

        case vk::Result::eNotReady:
            return "A fence or query has not yet completed"sv;

        case vk::Result::eTimeout:
            return "A wait operation has not completed in the specified time"sv;

        case vk::Result::eEventSet:
            return "An event is signaled"sv;

        case vk::Result::eEventReset:
            return "An event is unsignaled"sv;

        case vk::Result::eIncomplete:
            return "A return array was too small for the result"sv;

        case vk::Result::eErrorOutOfHostMemory:
            return "A host memory allocation has failed"sv;

        case vk::Result::eErrorOutOfDeviceMemory:
            return "A device memory allocation has failed"sv;

        case vk::Result::eErrorInitializationFailed:
            return "Initialization of an object failed"sv;

        case vk::Result::eErrorDeviceLost:
            return "The logical or physical device has been lost"sv;

        case vk::Result::eErrorMemoryMapFailed:
            return "Mapping of a memory object failed"sv;

        case vk::Result::eErrorLayerNotPresent:
            return "A requested layer is not present"sv;

        case vk::Result::eErrorExtensionNotPresent:
            return "A requested extension is not supported"sv;

        case vk::Result::eErrorFeatureNotPresent:
            return "A requested feature is not supported"sv;

        case vk::Result::eErrorIncompatibleDriver:
            return "The Vulkan driver is incompatible"sv;

        case vk::Result::eErrorTooManyObjects:
            return "Too many objects of this type have been created"sv;

        case vk::Result::eErrorFormatNotSupported:
            return "The requested format is not supported"sv;

        case vk::Result::eErrorFragmentedPool:
            return "Descriptor pool allocation failed due to fragmentation"sv;

        case vk::Result::eErrorUnknown:
            return "An unknown Vulkan error occurred"sv;

        case vk::Result::eSuboptimalKHR:
            return "The swapchain no longer matches the surface exactly but can still be used"sv;

        case vk::Result::eErrorSurfaceLostKHR:
            return "The Vulkan surface has been lost"sv;

        case vk::Result::eErrorNativeWindowInUseKHR:
            return "The native window is already in use"sv;

        case vk::Result::eErrorOutOfDateKHR:
            return "The swapchain is out of date"sv;

        default:
            return "Unknown Vulkan result"sv;
        }
    }

    [[nodiscard]] constexpr inline std::wstring_view getResultDescriptionW(vk::Result result)
    {
        using namespace std::literals;

        switch (result)
        {
        case vk::Result::eSuccess:
            return L"Command successfully completed"sv;

        case vk::Result::eNotReady:
            return L"A fence or query has not yet completed"sv;

        case vk::Result::eTimeout:
            return L"A wait operation has not completed in the specified time"sv;

        case vk::Result::eEventSet:
            return L"An event is signaled"sv;

        case vk::Result::eEventReset:
            return L"An event is unsignaled"sv;

        case vk::Result::eIncomplete:
            return L"A return array was too small for the result"sv;

        case vk::Result::eErrorOutOfHostMemory:
            return L"A host memory allocation has failed"sv;

        case vk::Result::eErrorOutOfDeviceMemory:
            return L"A device memory allocation has failed"sv;

        case vk::Result::eErrorInitializationFailed:
            return L"Initialization of an object failed"sv;

        case vk::Result::eErrorDeviceLost:
            return L"The logical or physical device has been lost"sv;

        case vk::Result::eErrorMemoryMapFailed:
            return L"Mapping of a memory object failed"sv;

        case vk::Result::eErrorLayerNotPresent:
            return L"A requested layer is not present"sv;

        case vk::Result::eErrorExtensionNotPresent:
            return L"A requested extension is not supported"sv;

        case vk::Result::eErrorFeatureNotPresent:
            return L"A requested feature is not supported"sv;

        case vk::Result::eErrorIncompatibleDriver:
            return L"The Vulkan driver is incompatible"sv;

        case vk::Result::eErrorTooManyObjects:
            return L"Too many objects of this type have been created"sv;

        case vk::Result::eErrorFormatNotSupported:
            return L"The requested format is not supported"sv;

        case vk::Result::eErrorFragmentedPool:
            return L"Descriptor pool allocation failed due to fragmentation"sv;

        case vk::Result::eErrorUnknown:
            return L"An unknown Vulkan error occurred"sv;

        case vk::Result::eSuboptimalKHR:
            return L"The swapchain no longer matches the surface exactly but can still be used"sv;

        case vk::Result::eErrorSurfaceLostKHR:
            return L"The Vulkan surface has been lost"sv;

        case vk::Result::eErrorNativeWindowInUseKHR:
            return L"The native window is already in use"sv;

        case vk::Result::eErrorOutOfDateKHR:
            return L"The swapchain is out of date"sv;

        default:
            return L"Unknown Vulkan result"sv;
        }
    }

    inline bool VulkanCheckErrorStatus(vk::Result result)
    {
        if (result == vk::Result::eSuccess)
            return false;

        RHI_LOG_ERROR(std::format(L"Vulkan error: {}", getResultDescriptionW(result)), RhiApi::Vulkan);

        return true;
    }

    inline bool VulkanCheckErrorStatus(VkResult result)
    {
        return VulkanCheckErrorStatus(static_cast<vk::Result>(result));
    }

}

#endif // TITANIUM_VULKAN_HEADER_H
