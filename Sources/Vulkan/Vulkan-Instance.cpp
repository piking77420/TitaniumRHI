#include <Vulkan-Instance.hpp>

#include <print>
#include <vector>

namespace TiRHI::Vulkan
{
    constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

    static bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vk::Result r = (vk::enumerateInstanceLayerProperties(&layerCount, nullptr));
        if (r != vk::Result::eSuccess)
        {
            std::println("Failed to enumerate enumerate Instance Layer Properties");
            return false;
        }

        std::vector<vk::LayerProperties> availableLayers(layerCount);
        r = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        if (r != vk::Result::eSuccess)
        {
            std::println("Failed to enumerate Instance Layer Properties");
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

    Instance::Instance()
    {
        std::set<std::string> m_requireExtensionNeeded;

        // TODO should be pass by the user
        std::vector<const char*> requiredExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}; // may ask the user to use it

        for (auto& ext : requiredExtensions)
            m_requireExtensionNeeded.insert(ext);

        constexpr bool useDebugScope = true;
        if (useDebugScope)
        {
            m_requireExtensionNeeded.insert(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        std::vector<const char*> extensions;
        for (auto& ext : requiredExtensions)
            extensions.push_back(ext);

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

        if (!checkValidationLayerSupport())
        {
            std::println("missing validation layer support");
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            instanceCreateInfo.enabledLayerCount = 0u;
            instanceCreateInfo.ppEnabledLayerNames = nullptr;
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)

        m_instance = vk::createInstance(instanceCreateInfo);
        if (m_instance)
        {
            std::println("Vulkan Instance creation sucess");
        }
        else
        {
            std::println("Vulkan Instance creation failed");
        }

        if (useDebugScope)
        {
            m_vulkanFunctions.beginDebugLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
                vkGetInstanceProcAddr(m_instance, "vkCmdBeginDebugUtilsLabelEXT"));

            m_vulkanFunctions.endDebugLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
                vkGetInstanceProcAddr(m_instance, "vkCmdEndDebugUtilsLabelEXT"));

            m_vulkanFunctions.insertDebugLabel = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(
                vkGetInstanceProcAddr(m_instance, "vkCmdInsertDebugUtilsLabelEXT"));

            m_vulkanFunctions.setObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT"));
        }
    }

    Instance::~Instance()
    {
        m_instance.destroy();
        std::println("Destroying Vulkan Instance");
        m_instance = nullptr;
    }

} // namespace TiRHI::Vulkan
