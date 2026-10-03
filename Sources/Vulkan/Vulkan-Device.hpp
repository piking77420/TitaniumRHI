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
    class RHI;

    class Device : public BaseDevice<Device, RHI>
    {
    public:
        Device() = delete;
        ~Device() = default;
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) noexcept = default;
        Device& operator=(Device&&) noexcept = default;
        Device(RHI& rhi);

        bool build(RHI& rhi, const std::span<const Adapter>& adapters, std::optional<size_t> index = {});

        void wait();

        vk::Device getNativeDevice() noexcept
        {
            return m_device.get();
        }

    private:
        vk::UniqueDevice m_device;

        bool createDevice(vk::PhysicalDevice physicalDevice, const Adapter& adapter);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
