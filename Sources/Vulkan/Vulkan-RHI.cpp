#include <Vulkan/Vulkan-RHI.hpp>

#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan
{
    std::vector<Adapter::Features> getAdapterFeatures(const std::vector<vk::ExtensionProperties>& extensions)
    {
        std::vector<Adapter::Features> result;

        const auto hasExtension = [&](const char* name)
        {
            return std::ranges::any_of(extensions, [name](const vk::ExtensionProperties& extension)
                                       { return std::strcmp(extension.extensionName.data(), name) == 0; });
        };

        const bool accelerationStructure = hasExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

        if (accelerationStructure && hasExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME))
        {
            result.push_back(Adapter::Features::RayQuery);
        }

        if (accelerationStructure && hasExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME))
        {
            result.push_back(Adapter::Features::RayTracingPipeline);
        }

        if (hasExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME))
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
        queryPhysicalDeviceAvailable();
    }

    Device RHI::createDevice()
    {
        return Device(m_instance, m_adapters, m_physicalDevices);
    }

    Device RHI::createDevice(size_t adapterIndex)
    {
        assert(adapterIndex >= 0 && adapterIndex < m_adapters.size());

        return Device(m_instance, m_adapters, m_physicalDevices, adapterIndex);
    }

    void RHI::queryPhysicalDeviceAvailable()
    {
        m_adapters.clear();

        m_physicalDevices = m_instance.getInstance().enumeratePhysicalDevices();
        m_adapters.reserve(m_physicalDevices.size());

        for (size_t i = 0; i < m_physicalDevices.size(); i++)
        {
            const vk::PhysicalDevice& physicalDevice = m_physicalDevices[i];
            const std::vector<vk::ExtensionProperties> deviceExtensionProperties =
                physicalDevice.enumerateDeviceExtensionProperties();

            const bool supportsSwapchain = std::ranges::any_of(
                deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
                { return std::strcmp(extension.extensionName.data(), VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0; });

            if (!supportsSwapchain)
            {
                m_physicalDevices.erase(m_physicalDevices.begin() + i);
                continue; // support swapchain is mandatory
            }

            const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
            const std::string_view name{properties.deviceName.data()};
            const std::wstring wideName(name.begin(), name.end());

            RHI_LOG_VERBOSE(std::format(L"GPU: {}", wideName), RhiApi::Vulkan);
            m_adapters.emplace_back(Adapter(name, getAdapterFeatures(deviceExtensionProperties),
                                            getDeviceProperty(physicalDevice), properties.vendorID));
        }

        assert(m_physicalDevices.size() == m_adapters.size());
        if (m_physicalDevices.size() != m_adapters.size())
        {
            RHI_LOG_ERROR(L"Something went wrong when enumerate adapter", RhiApi::Vulkan);
        }
    }

} // namespace TiRHI::Vulkan
