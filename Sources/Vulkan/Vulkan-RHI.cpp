#include <Vulkan/Vulkan-RHI.hpp>

#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan
{
    [[nodiscard]] bool supportExtension(const std::vector<vk::ExtensionProperties>& extensions,
                                        const char* extensionName)
    {
        for (const auto& extension : extensions)
        {
            if (std::strcmp(extension.extensionName.data(), extensionName) == 0)
                return true;
        }

        return false;
    }

    std::vector<Adapter::Features> getAdapterFeatures(const std::vector<vk::ExtensionProperties>& extensions)
    {
        std::vector<Adapter::Features> result;

        const bool accelerationStructure = supportExtension(extensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

        if (accelerationStructure && supportExtension(extensions, VK_KHR_RAY_QUERY_EXTENSION_NAME))
        {
            result.push_back(Adapter::Features::RayQuery);
        }

        if (accelerationStructure && supportExtension(extensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
        {
            result.push_back(Adapter::Features::RayTracingPipeline);
        }

        if (supportExtension(extensions, VK_EXT_MESH_SHADER_EXTENSION_NAME))
        {
            result.push_back(Adapter::Features::MeshShader);
        }

        return result;
    }

    Adapter::Properties::Limits getDevicePropertiesLimits(vk::PhysicalDeviceProperties properties)
    {
        Adapter::Properties::Limits limits;
        limits.minUniformBufferOffset = properties.limits.minUniformBufferOffsetAlignment;

        return limits;
    }

    Adapter::Properties::MemoryLimits
    getDevicePropertiesMemoryLimits(const vk::PhysicalDeviceMemoryProperties& memoryProperties)
    {
        Adapter::Properties::MemoryLimits limits{};
        limits.vramMemoryBytes = 0;
        for (size_t i = 0; i < static_cast<size_t>(memoryProperties.memoryHeapCount); i++)
        {
            const auto& heap = memoryProperties.memoryHeaps[i];

            if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                limits.vramMemoryBytes += heap.size;
        }

        return limits;
    }

    Adapter::Properties getDeviceProperty(vk::PhysicalDevice physicalDevice)
    {
        Adapter::Properties rhiProperty{};

        const vk::PhysicalDeviceProperties& vkPhysicalDeviceProperties = physicalDevice.getProperties();
        rhiProperty.limits = getDevicePropertiesLimits(vkPhysicalDeviceProperties);
        rhiProperty.deviceType = [&vkPhysicalDeviceProperties]()
        {
            switch (vkPhysicalDeviceProperties.deviceType)
            {
            case vk::PhysicalDeviceType::eDiscreteGpu:
                return Adapter::Properties::Type::DiscreteGpu;

            case vk::PhysicalDeviceType::eIntegratedGpu:
                return Adapter::Properties::Type::IntegratedGpu;

            case vk::PhysicalDeviceType::eVirtualGpu:
                return Adapter::Properties::Type::VirtualGpu;

            case vk::PhysicalDeviceType::eCpu:
                return Adapter::Properties::Type::Cpu;

            case vk::PhysicalDeviceType::eOther:
            default:
                return Adapter::Properties::Type::Unknow;
            }
        }();
        rhiProperty.memoryLimits = getDevicePropertiesMemoryLimits(physicalDevice.getMemoryProperties());

        return rhiProperty;
    }

    RHI::RHI(const TiRHI::RhiCreate& create)
        : TiRHI::BaseRHI<Vulkan::RHI>(create)
    {
        enumerateAvailableAdatper();
    }

    Device RHI::newDevice()
    {
        return Device(*this);
    }

    std::vector<vk::PhysicalDevice> RHI::getValidPhysicalDevices()
    {
        std::vector<vk::PhysicalDevice> physicalDevices = m_instance.getInstance().enumeratePhysicalDevices();
        for (auto it = physicalDevices.begin(); it != physicalDevices.end();)
        {
            if (!isPhysicalDeviceValid(*it))
                it = physicalDevices.erase(it);
            else
                ++it;
        }

        return physicalDevices;
    }

    void RHI::enumerateAvailableAdatper()
    {
        m_adapters.clear();

        std::vector<vk::PhysicalDevice> physicalDevices = getValidPhysicalDevices();
        m_adapters.reserve(physicalDevices.size());

        for (size_t i = 0; i < physicalDevices.size(); i++)
        {
            const vk::PhysicalDevice& physicalDevice = physicalDevices[i];
            const std::vector<vk::ExtensionProperties> deviceExtensionProperties =
                physicalDevice.enumerateDeviceExtensionProperties();

            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            const std::string_view name{properties.deviceName.data()};
            const std::wstring wideName(name.begin(), name.end());

            RHI_LOG_VERBOSE(std::format(L"GPU: {}", wideName), RhiApi::Vulkan);
            m_adapters.emplace_back(Adapter(name, getAdapterFeatures(deviceExtensionProperties),
                                            getDeviceProperty(physicalDevice), properties.vendorID));
        }

        assert(physicalDevices.size() == m_adapters.size());
        if (physicalDevices.size() != m_adapters.size())
        {
            RHI_LOG_ERROR(L"Something went wrong when enumerate adapter", RhiApi::Vulkan);
        }
    }

    bool RHI::isPhysicalDeviceValid(vk::PhysicalDevice device)
    {
        // should be the clean space for current rhi
        // for exemple check if support compute if compute is mandatory

        const std::vector<vk::ExtensionProperties> deviceExtensionProperties =
            device.enumerateDeviceExtensionProperties();

        if (!supportExtension(deviceExtensionProperties, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            return false;
    }

} // namespace TiRHI::Vulkan
