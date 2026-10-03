#ifndef TITANIUM_VULKAN_PRIVATE_DEVICE_QUEUE_PROPERTIES_H
#define TITANIUM_VULKAN_PRIVATE_DEVICE_QUEUE_PROPERTIES_H

#include <span>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace TiRHI::Vulkan::Private
{
    class DeviceQueueProperties
    {
    public:
        struct Queue
        {
            bool graphic;
            bool present;
            bool compute;
            bool transfer;
            uint32_t count;
            uint32_t index;
        };

        DeviceQueueProperties() = default;
        DeviceQueueProperties(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
        ~DeviceQueueProperties() = default;

        std::span<const Queue> getQueuProperties() const
        {
            return queues;
        }

    private:
        std::vector<Queue> queues;
    };

} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_DEVICE_QUEUE_PROPERTIES_H
