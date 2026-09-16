#ifndef TITANIUM_VULKAN_DEVICE_H
#define TITANIUM_VULKAN_DEVICE_H

namespace TiRHI::Vulkan
{
    class Instance;

    class Device
    {
    public:
        Device(Instance& instance);
        ~Device();

        vk::PhysicalDevice getPhysicalDevice() noexcept
        {
            return m_physicalDevice;
        }

    private:
        vk::PhysicalDevice m_physicalDevice;

        void choosePhysicalDevice(Instance& instance);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEVICE_H
