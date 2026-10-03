#include <Private/DeviceQueueFamily.hpp>

namespace TiRHI::Vulkan::Private
{
    DeviceQueueProperties::DeviceQueueProperties(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    {
        const auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        queues.reserve(queueFamilies.size());

        for (uint32_t i = 0; i < queueFamilies.size(); ++i)
        {
            auto& q = queues.emplace_back();
            q.count = queueFamilies[i].queueCount;
            q.graphic == static_cast<bool>(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics);
            q.present = physicalDevice.getSurfaceSupportKHR(i, surface);
            q.compute = static_cast<bool>(queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute);
            q.transfer = static_cast<bool>(queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer);
            q.index = i;
        }
    }

} // TiRHI::Vulkan::Private
