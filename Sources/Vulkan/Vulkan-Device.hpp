#ifndef TITANIUM_VULKAN_DEVICE_H
#define TITANIUM_VULKAN_DEVICE_H

#include <vector>

#include <Titanium/RHI-BaseDevice.hpp>
#include <Titanium/RHI-Adapter.hpp>

#include <vulkan/vulkan.hpp>
#include <Private/DeviceQueueFamily.hpp>

namespace TiRHI::Contract
{
    struct DeviceContractAccess;

} // TiRHI::Contract

namespace TiRHI::Vulkan
{
    class Instance;
    class RHI;
    class Surface;

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

        bool build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                   std::optional<size_t> index = {});

        void wait();

        vk::Device getNativeDevice() noexcept
        {
            return m_device.get();
        }

        const Private::DeviceQueueProperties& getQueueProperties() const
        {
            return m_queueProperties;
        }

        vk::Queue getNativeGraphicQueue() const
        {
            return m_graphicQueue.get();
        }

        vk::Queue getNativePresentQueue() const
        {
            return m_presentQueue;
        }

        vk::PhysicalDevice getNativePhysicalDevice() const
        {
            return m_physicalDevice;
        }

    private:
        vk::UniqueDevice m_device;

        vk::PhysicalDevice m_physicalDevice;

        Private::DeviceQueueProperties m_queueProperties;

        vk::UniqueQueue m_graphicQueue;

        vk::Queue m_presentQueue;

        bool createDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Adapter& adapter);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
