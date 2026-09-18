#include <Vulkan-Device.hpp>

#include <print>
#include <string>
#include <Vulkan-Instance.hpp>

namespace TiRHI::Vulkan
{
    int getPhyscialDeviceScrore(const vk::PhysicalDevice& physicalDevice,
                                const vk::PhysicalDeviceProperties& properties, Device::Extension& extensionSupported)
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

        extensionSupported = Device::Extension(deviceExtensionProperties);

        score += extensionSupported.supportsSwapchain ? 1000 : -1000;
        score += extensionSupported.supportsRaytracing ? 100 : -100;
        score += extensionSupported.supportsMultiview ? 10 : -10;
        score += extensionSupported.supportsMeshShader ? 100 : -100;

        return score;
    }

    Device::Device(Instance& instance)
    {
        choosePhysicalDevice(instance);
        createDevice();
    }

    Device::~Device()
    {
        std::println("Destroying Device");
    }

    void Device::choosePhysicalDevice(Instance& instance)
    {
        vk::Instance vkInstance = instance.getInstance();
        uint32_t physicalDeviceCount = 0;
        m_physicalDevices = vkInstance.enumeratePhysicalDevices();
        if (m_physicalDevices.empty())
        {
            std::println("Failed to enumerate physical devices");
            return;
        }

        std::println("Available physical device : ");
        for (const vk::PhysicalDevice& physicalDevice : m_physicalDevices)
        {
            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            std::println("GPU: {}", properties.deviceName.data());
        }
        std::println();

        m_extension.resize(m_physicalDevices.size());

        size_t lastBestPhysicalDeviceIndex = 0;
        int lastBestScore = std::numeric_limits<int>::min();
        for (size_t i = 0; i < m_physicalDevices.size(); i++)
        {
            const vk::PhysicalDevice& physicalDevice = m_physicalDevices[i];
            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            int currentScore = getPhyscialDeviceScrore(physicalDevice, properties, m_extension[i]);

            if (currentScore > lastBestScore)
            {
                lastBestScore = currentScore;
                lastBestPhysicalDeviceIndex = i;
            }
        }

        if (lastBestPhysicalDeviceIndex == std::numeric_limits<int>::min() && lastBestScore == 0)
        {
            std::println("Failed to choose a physical device");
            return;
        }

        if (!m_extension[lastBestPhysicalDeviceIndex].supportsSwapchain)
        {
            std::println("choosen physical device dont support swap chain");
            return;
        }

        const vk::PhysicalDeviceProperties properties = m_physicalDevices[lastBestPhysicalDeviceIndex].getProperties();
        m_currentPhysicalDeviceIndex = lastBestPhysicalDeviceIndex;
        std::println("Device Choose: {}", properties.deviceName.data());
        return;
    }

    void Device::createDevice()
    {
        const std::vector<vk::QueueFamilyProperties> queueFamilyPropertie =
            getPhysicalDevice().getQueueFamilyProperties();

        size_t allPropertiesQueuIndex = std::numeric_limits<size_t>::max();

        for (size_t i = 0; i < queueFamilyPropertie.size(); i++)
        {
            if (queueFamilyPropertie[i].queueFlags &
                (vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer))
            {
                allPropertiesQueuIndex = i;
                break;
            }
        }

        if (allPropertiesQueuIndex == std::numeric_limits<size_t>::max())
        {
            std::println("Failed to find an valid queu");
            return;
        }

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo = {};
        std::array<float, 1> queuePriority = {1.f};
        queueCreateInfo.resize(1);

        for (uint32_t i = 0; i < 1; i++)
        {
            queueCreateInfo[i].sType = vk::StructureType::eDeviceQueueCreateInfo;
            queueCreateInfo[i].queueFamilyIndex = allPropertiesQueuIndex;
            queueCreateInfo[i].queueCount = 1;
            queueCreateInfo[i].pQueuePriorities = queuePriority.data();
        }

        std::vector<const char*> getDeviceExtensionName = m_extension[m_currentPhysicalDeviceIndex].getExtensionName();

        vk::DeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = vk::StructureType::eDeviceCreateInfo;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());
        deviceCreateInfo.pEnabledFeatures = nullptr;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(getDeviceExtensionName.size());
        deviceCreateInfo.ppEnabledExtensionNames = getDeviceExtensionName.data();

        std::println("Create Device");
        m_device = getPhysicalDevice().createDeviceUnique(deviceCreateInfo);
    }

} // namespace TiRHI::Vulkan
