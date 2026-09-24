#include <Vulkan-Instance.hpp>

#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#elif defined(__linux__)

#if defined(USE_WAYLAND)
#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>
#elif defined(USE_XCB)
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#elif defined(USE_XLIB)
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif // defined(USE_WAYLAND)

#endif // defined(_WIN32)

#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan
{
    constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    static bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vk::Result r = (vk::enumerateInstanceLayerProperties(&layerCount, nullptr));
        if (r != vk::Result::eSuccess) // TODO add string to error types
        {
            RHI_LOG_ERROR(L"Failed to enumerate enumerate Instance Layer Properties", RhiApi::Vulkan);
            return false;
        }

        std::vector<vk::LayerProperties> availableLayers(layerCount);
        r = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        if (r != vk::Result::eSuccess)
        {
            RHI_LOG_ERROR(L"Failed to enumerate Instance Layer Propertiess", RhiApi::Vulkan);
            return false;
        }
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
                                                 const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 [[maybe_unused]] void* pUserData) noexcept
    {
        if (!pCallbackData)
            return VK_FALSE;

        std::wstring categories;

        const auto addCategory = [&](std::wstring_view category)
        {
            if (!categories.empty())
                categories += L" | ";

            categories += category;
        };

        if (messageTypes & vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
            addCategory(L"General");

        if (messageTypes & vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            addCategory(L"Validation");

        if (messageTypes & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            addCategory(L"Performance");

        if (categories.empty())
            categories = L"Unknown";

        const std::wstring idName =
            pCallbackData->pMessageIdName
                ? std::wstring(pCallbackData->pMessageIdName,
                               pCallbackData->pMessageIdName + std::strlen(pCallbackData->pMessageIdName))
                : L"Unknown";

        const std::wstring description =
            pCallbackData->pMessage
                ? std::wstring(pCallbackData->pMessage, pCallbackData->pMessage + std::strlen(pCallbackData->pMessage))
                : L"(no description)";

        const std::wstring message = std::format(L"Validation Layer\n"
                                                 L"    ID       : {}\n"
                                                 L"    Name     : {}\n"
                                                 L"    Category : {}\n"
                                                 L"    Message  : {}",
                                                 pCallbackData->messageIdNumber, idName, categories, description);

        switch (messageSeverity)
        {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            RHI_LOG_ERROR(message, RhiApi::Vulkan);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            RHI_LOG_WARNING(message, RhiApi::Vulkan);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            RHI_LOG_INFO(message, RhiApi::Vulkan);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            RHI_LOG_VERBOSE(message, RhiApi::Vulkan);
            break;

        default:
            break;
        }

        return VK_FALSE;
    }

    Instance::Instance()
    {
        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(_WIN32)
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

#elif defined(__linux__)

#if defined(USE_WAYLAND)
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined(USE_XCB)
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(USE_XLIB)
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif // defined(__linux__)
#endif // defined(_WIN32)
        };

#if defined(TITANIUM_VALIDATION_LAYER)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        vk::ApplicationInfo appInfo = {};
        appInfo.sType = vk::StructureType::eApplicationInfo;
        appInfo.pApplicationName = "TitaniumRHI";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "TitaniumRHI";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = vk::StructureType::eInstanceCreateInfo;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = {};
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

#if defined(TITANIUM_VALIDATION_LAYER)

        vk::DebugUtilsMessengerCreateInfoEXT debugInfo{};
        if (checkValidationLayerSupport())
        {
            RHI_LOG_INFO(L"Enable validation layer support", RhiApi::Vulkan);
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

            debugInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;

            debugInfo.messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

            debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

            debugInfo.pfnUserCallback = &debugCallback;
            debugInfo.pUserData = nullptr;
        }
        else
        {
            RHI_LOG_ERROR(L"Missing validation layer support", RhiApi::Vulkan);
            instanceCreateInfo.enabledLayerCount = 0u;
            instanceCreateInfo.ppEnabledLayerNames = nullptr;
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)

        m_instance = vk::createInstance(instanceCreateInfo);
        if (m_instance)
        {
            RHI_LOG_INFO(L"Vulkan Instance creation sucess", RhiApi::Vulkan);
        }
        else
        {
            RHI_LOG_ERROR(L"Vulkan Instance creation failed", RhiApi::Vulkan);
        }

        queryVulkanFunctions();

#if defined(TITANIUM_VALIDATION_LAYER)
        const PFN_vkCreateDebugUtilsMessengerEXT createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

        const VkResult result = createMessenger(static_cast<VkInstance>(m_instance),
                                                reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&debugInfo),
                                                nullptr, &messenger);

        if (static_cast<vk::Result>(result) != vk::Result::eSuccess)
            RHI_LOG_FATAL(L"Failed to create Vulkan DebugUtilsMessenger", RhiApi::Vulkan);

        m_debugUtilsMessenger = messenger;
#endif // defined(TITANIUM_VALIDATION_LAYER)
    }

    Instance::~Instance()
    {
#if defined(TITANIUM_VALIDATION_LAYER)
        const PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (!destroyMessenger)
            RHI_LOG_ERROR(L"Failed to query vkDestroyDebugUtilsMessengerEXT CallAddr", RhiApi::Metal);

        if (m_debugUtilsMessenger && destroyMessenger)
        {
            destroyMessenger(static_cast<VkInstance>(m_instance),
                             static_cast<VkDebugUtilsMessengerEXT>(m_debugUtilsMessenger), nullptr);

            RHI_LOG_INFO(L"Destroying Vulkan DebugUtilsMessengerEXT", RhiApi::Vulkan);
            m_debugUtilsMessenger = nullptr;
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)
        m_instance.destroy();
        RHI_LOG_INFO(L"Destroying Vulkan Instance", RhiApi::Vulkan);
        m_instance = nullptr;
    }

    void Instance::queryVulkanFunctions()
    {
        m_vulkanFunctions.beginDebugLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCmdBeginDebugUtilsLabelEXT"));
        if (!m_vulkanFunctions.beginDebugLabel)
            RHI_LOG_ERROR(L"Failed to query vkCmdBeginDebugUtilsLabelEXT callAddr", RhiApi::Vulkan);

        m_vulkanFunctions.endDebugLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCmdEndDebugUtilsLabelEXT"));
        if (!m_vulkanFunctions.endDebugLabel)
            RHI_LOG_ERROR(L"Failed to query vkCmdInsertDebugUtilsLabelEXT callAddr", RhiApi::Vulkan);

        m_vulkanFunctions.insertDebugLabel = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCmdInsertDebugUtilsLabelEXT"));
        if (!m_vulkanFunctions.insertDebugLabel)
            RHI_LOG_ERROR(L"Failed to query vkCmdInsertDebugUtilsLabelEXT callAddr", RhiApi::Vulkan);

        m_vulkanFunctions.setObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT"));
        if (!m_vulkanFunctions.setObjectName)
            RHI_LOG_ERROR(L"Failed to query vkSetDebugUtilsObjectNameEXT callAddr", RhiApi::Vulkan);
    }

} // namespace TiRHI::Vulkan
