#ifndef TITANIUM_VULKAN_DEVICE_H
#define TITANIUM_VULKAN_DEVICE_H

#include <vector>

#include <Titanium/RHI-BaseDevice.hpp>
#include <Titanium/RHI-Adapter.hpp>

#include <vulkan/vulkan.hpp>

namespace TiRHI::Contract
{
    struct DeviceContractAccess;

} // TiRHI::Contract

namespace TiRHI::Vulkan
{
    class Instance;

    class Device : public BaseDevice
    {
    public:
        Device() = default;
        ~Device() = default;
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) noexcept = default;
        Device& operator=(Device&&) noexcept = default;
        Device(Instance& instance, const std::vector<Adapter>& adapters,
               const std::vector<vk::PhysicalDevice>& devices);
        Device(Instance& instance, const std::vector<Adapter>& adapters, const std::vector<vk::PhysicalDevice>& devices,
               size_t index);

        void wait();

        vk::Device getNativeHandle() noexcept
        {
            return m_device.get();
        }

    private:
        friend Contract::DeviceContractAccess;

        std::vector<vk::PhysicalDevice> m_physicalDevices;

        vk::UniqueDevice m_device;

        void choosePhysicalDeviceIndex(Instance& instance, const std::vector<Adapter>& adapters,
                                       const std::vector<vk::PhysicalDevice>& devices);

        void createLogicalDevice(vk::PhysicalDevice physicalDevice, const Adapter& adapter);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
