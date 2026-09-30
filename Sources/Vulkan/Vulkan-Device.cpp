#include <Vulkan-Device.hpp>

#include <string>
#include <Titanium/Log.hpp>
#include <Vulkan-Instance.hpp>

namespace TiRHI::Vulkan
{
    std::vector<Adapter::Features> getDeviceExt(const vk::PhysicalDevice& physicalDevice,
                                                const vk::PhysicalDeviceProperties& properties,
                                                Device::Extension& extensionSupported)
    {
        std::vector<Adapter::Features> result;
        const std::vector<vk::ExtensionProperties> deviceExtensionProperties =
            physicalDevice.enumerateDeviceExtensionProperties();

        extensionSupported = Device::Extension(deviceExtensionProperties);

        if (extensionSupported.supportsRaytracing)
            result.push_back(Adapter::Features::RayTracing);

        if (extensionSupported.supportsMeshShader)
            result.push_back(Adapter::Features::MeshShader);

        return result;
    }

    int getPhyscialDeviceScrore(const vk::PhysicalDevice& physicalDevice,
                                const vk::PhysicalDeviceProperties& properties,
                                const Device::Extension& extensionSupported)
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

        score += extensionSupported.supportsSwapchain ? 1000 : -1000;
        score += extensionSupported.supportsRaytracing ? 100 : -100;
        score += extensionSupported.supportsMultiview ? 10 : -10;
        score += extensionSupported.supportsMeshShader ? 100 : -100;

        return score;
    }

    Device::Device(Instance& instance, std::vector<Adapter>& adapter)
    {
        adapter = queryPhysicalDeviceAvailable(instance);
    }

    void Device::createDevice(const std::vector<Adapter>& adapter)
    {
        choosePhysicalDevice(adapter);
        createLogicalDevice(adapter[m_currentPhysicalDeviceIndex]);
    }

    void Device::createDevice(size_t adapterIndex, const std::vector<Adapter>& adapters)
    {
        assert(adapterIndex >= 0 && adapterIndex < adapters.size());
        m_currentPhysicalDeviceIndex = adapterIndex;
        assert(m_currentPhysicalDeviceIndex != std::numeric_limits<size_t>::max() &&
               m_currentPhysicalDeviceIndex < m_physicalDevices.size());

        createLogicalDevice(adapters[m_currentPhysicalDeviceIndex]);
    }

    std::vector<Adapter> Device::queryPhysicalDeviceAvailable(Instance& instance)
    {
        std::vector<Adapter> result;

        vk::Instance vkInstance = instance.getInstance();
        m_physicalDevices = vkInstance.enumeratePhysicalDevices();
        m_extension.resize(m_physicalDevices.size());
        result.reserve(m_physicalDevices.size());

        for (size_t i = 0; i < m_physicalDevices.size(); i++)
        {
            const vk::PhysicalDevice& physicalDevice = m_physicalDevices[i];
            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            const std::string_view name{properties.deviceName.data()};
            const std::wstring wideName(name.begin(), name.end());

            RHI_LOG_VERBOSE(std::format(L"GPU: {}", wideName), RhiApi::Vulkan);
            result.emplace_back(
                Adapter(name, getDeviceExt(physicalDevice, properties, m_extension[i]), 0ull, properties.vendorID));
        }

        return result;
    }

    void Device::choosePhysicalDevice(const std::vector<Adapter>& adapter)
    {
        uint32_t physicalDeviceCount = 0;

        m_extension.resize(adapter.size());
        size_t lastBestPhysicalDeviceIndex = 0;
        int lastBestScore = std::numeric_limits<int>::min();
        assert(m_physicalDevices.size() == adapter.size());
        assert(m_physicalDevices.size() == m_extension.size());
        for (size_t i = 0; i < adapter.size(); i++)
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
            RHI_LOG_ERROR(L"Failed to choose a physical device", RhiApi::Vulkan);
            return;
        }

        if (!m_extension[lastBestPhysicalDeviceIndex].supportsSwapchain)
        {
            RHI_LOG_ERROR(L"Choosen physical device dont support swap chain", RhiApi::Vulkan);
            return;
        }

        const vk::PhysicalDeviceProperties properties = m_physicalDevices[lastBestPhysicalDeviceIndex].getProperties();
        m_currentPhysicalDeviceIndex = lastBestPhysicalDeviceIndex;
        const std::string_view name{properties.deviceName.data()};
        const std::wstring wideName(name.begin(), name.end());
        RHI_LOG_VERBOSE(std::format(L"Device Choosen: {}", wideName), RhiApi::Vulkan);

        return;
    }

    void Device::createLogicalDevice(const Adapter& adapter)
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
            RHI_LOG_ERROR(L"Failed to find an valid queu", RhiApi::Vulkan);
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

        m_device = getPhysicalDevice().createDeviceUnique(deviceCreateInfo);
        RHI_LOG_INFO(std::format(L"Create Device success {}",
                                 [&]()
                                 {
                                     const std::string_view name = adapter.getName();
                                     return std::wstring(name.begin(), name.end());
                                 }()),
                     RhiApi::Vulkan);
    }

} // namespace TiRHI::Vulkan
