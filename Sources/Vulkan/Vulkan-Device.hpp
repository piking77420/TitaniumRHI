#ifndef TITANIUM_VULKAN_DEVICE_H
#define TITANIUM_VULKAN_DEVICE_H

#include <vector>

#include <Titanium/RHI-Adapter.hpp>

#include <vulkan/vulkan.hpp>

namespace TiRHI::Vulkan
{
    class Instance;

    struct Extension
    {
        Extension() = default;

        ~Extension() = default;

        Extension(const std::vector<vk::ExtensionProperties> deviceExtensionProperties)
        {
            // clang-format off
                supportsSwapchain = std::ranges::any_of(
                    deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
                    { return std::strcmp(extension.extensionName.data(), VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0; });
                supportsSwapchain = std::ranges::any_of(
                    deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
                    { return std::strcmp(extension.extensionName.data(), VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0; });
                supportsMultiview = std::ranges::any_of(
                    deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
                    { return std::strcmp(extension.extensionName.data(), VK_KHR_MULTIVIEW_EXTENSION_NAME) == 0; });
                supportsMeshShader = std::ranges::any_of(
                    deviceExtensionProperties, [](const vk::ExtensionProperties& extension)
                    { return std::strcmp(extension.extensionName.data(), VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0; });
            // clang-format on
        }

        std::vector<const char*> getExtensionName()
        {
            std::vector<const char*> out;
            if (supportsSwapchain)
                out.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            if (supportsRaytracing)
                out.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            if (supportsMeshShader)
                out.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
            if (supportsMeshShader)
                out.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

            return out;
        }

        bool supportsSwapchain = false;
        bool supportsRaytracing = false;
        bool supportsMultiview = false;
        bool supportsMeshShader = false;
    };

    class Device
    {
    public:
        Device() = delete;
        ~Device() = default;
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) noexcept = default;
        Device& operator=(Device&&) noexcept = default;
        Device(Instance& instance, std::vector<Adapter>& adapter);

        vk::PhysicalDevice getPhysicalDevice() noexcept
        {
            return m_physicalDevices[m_currentPhysicalDeviceIndex];
        }

        vk::Device getDevice() noexcept
        {
            return m_device.get();
        }

        void createDevice(const std::vector<Adapter>& adapter);

        void createDevice(size_t adapterIndex, const std::vector<Adapter>& adapters);

        size_t getSelectPhyscialDeviceIndex() const
        {
            return m_currentPhysicalDeviceIndex;
        }

    private:
        std::vector<vk::PhysicalDevice> m_physicalDevices;

        std::vector<Extension> m_extension;

        size_t m_currentPhysicalDeviceIndex = std::numeric_limits<size_t>::max();

        vk::UniqueDevice m_device;

        [[nodiscard]] std::vector<Adapter> queryPhysicalDeviceAvailable(Instance& instance);

        void choosePhysicalDevice(const std::vector<Adapter>& adapter);

        void createLogicalDevice(const Adapter& adapter);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
