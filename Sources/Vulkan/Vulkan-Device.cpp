#include <Vulkan-Device.hpp>

#include <print>
#include <string>
#include <Vulkan-Instance.hpp>

namespace TiRHI::Vulkan
{
    int getPhyscialDeviceScrore(const vk::PhysicalDevice& physicalDevice,
                                const vk::PhysicalDeviceProperties& properties)
    {
        int score = 0;
        // properties.pipelineCacheUUID // TODO take a look at it
        switch (properties.deviceType)
        {
        case vk::PhysicalDeviceType::eDiscreteGpu:
            score += 1000;
            break;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            score += 10;
            break;
        case vk::PhysicalDeviceType::eCpu:
            score -= 10;
            break;
        default:
            break;
        }

        const std::vector<vk::ExtensionProperties> deviceExtensionProperties =
            physicalDevice.enumerateDeviceExtensionProperties();

        std::println("Enumerate vulkan device extension");
        for (const auto& ext : deviceExtensionProperties)
        {
            const std::string_view extName = std::string_view(ext.extensionName);
            std::println("{} - spec version {}", extName, ext.specVersion);
        }
        std::println();

        const bool supportsSwapchain = std::ranges::any_of(
            deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
            { return std::strcmp(extension.extensionName.data(), VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0; });
        const bool supportsRaytracing = std::ranges::any_of(
            deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
            { return std::strcmp(extension.extensionName.data(), VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0; });
        const bool supportsMultiview = std::ranges::any_of(
            deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
            { return std::strcmp(extension.extensionName.data(), VK_KHR_MULTIVIEW_EXTENSION_NAME) == 0; });
        const bool supportsMeshShader = std::ranges::any_of(
            deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
            { return std::strcmp(extension.extensionName.data(), VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0; });

        score += supportsSwapchain ? 1000 : -1000;
        score += supportsRaytracing ? 100 : -100;
        score += supportsMultiview ? 10 : -10;
        score += supportsMeshShader ? 100 : -100;

        return score;
    }

    Device::Device(Instance& instance)
    {
        choosePhysicalDevice(instance);
    }

    Device::~Device()
    {
    }

    void Device::choosePhysicalDevice(Instance& instance)
    {
        vk::Instance vkInstance = instance.getInstance();
        uint32_t physicalDeviceCount = 0;
        const std::vector<vk::PhysicalDevice> physicalDevices = vkInstance.enumeratePhysicalDevices();
        if (physicalDevices.empty())
        {
            std::println("Failed to enumerate physical devices");
            return;
        }

        std::println("Available physical device : ");
        for (const vk::PhysicalDevice& physicalDevice : physicalDevices)
        {
            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            std::println("GPU: {}", properties.deviceName.data());
        }
        std::println();

        std::vector<int> physicalDeviceScores;
        size_t lastBestPhysicalDevice = 0;
        int lastBestScore = std::numeric_limits<int>::min();
        for (const vk::PhysicalDevice& physicalDevice : physicalDevices)
        {
            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            int currentScore = getPhyscialDeviceScrore(physicalDevice, properties);

            if (currentScore > lastBestScore)
            {
                lastBestScore = currentScore;
                lastBestPhysicalDevice = std::distance(&physicalDevice, physicalDevices.data());
            }
        }

        {
            const vk::PhysicalDeviceProperties properties = physicalDevices[lastBestPhysicalDevice].getProperties();
            std::println("Device Choose: {}", properties.deviceName.data());
        }
    }

} // namespace TiRHI::Vulkan
