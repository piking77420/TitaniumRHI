#ifndef TITANIUM_VULKAN_DEVICE_H
#define TITANIUM_VULKAN_DEVICE_H

#include <vector>
#include <vulkan/vulkan.hpp>
#include <Volk/volk.h>
#include <Titanium/RHI-BaseDevice.hpp>
#include <Titanium/RHI-Adapter.hpp>
#include <Private/DeviceQueueFamily.hpp>

namespace TiRHI::Vulkan
{
    class Instance;
    class RHI;
    class Surface;
    class CommandList;
    class AcquiredFrame;

    class Device : public BaseDevice<Device, RHI>
    {
    public:
        Device() = delete;
        ~Device() = default;
        RHI_MOVE_ONLY(Device)
        Device(RHI& rhi);

        bool build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                   std::optional<size_t> index = {});

        void wait();

        void submit(std::span<const AcquiredFrame> acquiredFrame, std::span<CommandList*> commandList);

        void beginFrame();

        bool isValid() const
        {
            return m_device.get() != VK_NULL_HANDLE;
        }

        bool operator()() const
        {
            return isValid();
        }

        vk::Device getNativeDevice() const noexcept
        {
            return m_device.get();
        }

        const Private::DeviceQueueProperties& getQueueProperties() const
        {
            return m_queueProperties;
        }

        vk::Queue getNativeGraphicQueue() const
        {
            return m_graphicQueue;
        }

        uint32_t getNativeGraphicQueueIndex() const
        {
            return m_graphicQueueIndex;
        }

        vk::Queue getNativePresentQueue() const
        {
            return m_presentQueue;
        }

        uint32_t getNativePresentQueueIndex() const
        {
            return m_presentQueueIndex;
        }

        vk::PhysicalDevice getNativePhysicalDevice() const
        {
            return m_physicalDevice;
        }

        vk::Fence getNativeInFlightFence() const;

        const VolkDeviceTable& getVolkTable() const
        {
            return m_dispatch;
        }

    private:
        vk::UniqueDevice m_device;

        vk::PhysicalDevice m_physicalDevice;

        Private::DeviceQueueProperties m_queueProperties;

        vk::Queue m_graphicQueue;

        uint32_t m_graphicQueueIndex = 0;

        vk::Queue m_presentQueue;

        uint32_t m_presentQueueIndex = 0;

        struct Synchronisation
        {
            vk::UniqueFence inFlightFence;
        };

        std::vector<Synchronisation> m_synchronisations;

        VolkDeviceTable m_dispatch{};

        bool checkExtensionToEnableValid(const Adapter& adapter, std::vector<Adapter::Features>& finalFeatures);

        bool createDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Adapter& adapter,
                          const std::vector<Adapter::Features>& featuresToEnable);

        bool createSynchronisationPrimitives();

        bool initVolkTable();
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
