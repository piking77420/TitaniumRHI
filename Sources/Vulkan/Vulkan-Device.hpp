#ifndef TITANIUM_VULKAN_DEVICE_H
#define TITANIUM_VULKAN_DEVICE_H

#include <vector>
#include <vulkan/vulkan.hpp>
#include <Volk/volk.h>
#include <Titanium/RHI-BaseDevice.hpp>
#include <Titanium/RHI-Adapter.hpp>
#include <Private/DeviceQueueFamily.hpp>
#include <vma/vk_mem_alloc.h>

namespace TiRHI::Vulkan
{
    class Instance;
    class RHI;
    class Surface;
    class CommandList;
    class AcquiredFrame;

    class Device : public BaseDevice<Device, RHI, AcquiredFrame, CommandList>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        Device() = delete;
        ~Device();
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) noexcept = default;
        Device& operator=(Device&& other) noexcept;
        Device(RHI& rhi);

        struct Private
        {
            VmaAllocator& getAllocator(Device& device)
            {
                return device.m_vmaAllocator;
            }
        };

        bool build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                   std::optional<size_t> index = {});

        void wait();

        void submitPresent(std::span<const SubmitPresentDesc> submitPresentDesc);

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

        const Vulkan::Private::DeviceQueueProperties& getQueueProperties() const
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
        friend Private;

        vk::UniqueDevice m_device;

        vk::PhysicalDevice m_physicalDevice;

        Vulkan::Private::DeviceQueueProperties m_queueProperties;

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

        VmaAllocator m_vmaAllocator;

        bool checkExtensionToEnableValid(const Adapter& adapter, std::vector<Adapter::Features>& finalFeatures);

        bool createDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Adapter& adapter,
                          const std::vector<Adapter::Features>& featuresToEnable);

        bool createSynchronisationPrimitives();

        bool initVolkTable();

        bool initVmaAllocator(Vulkan::Instance& instance);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
