#ifndef TITANIUM_VULKAN_RHI_H
#define TITANIUM_VULKAN_RHI_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-BaseRHI.hpp>

#include <Vulkan-Instance.hpp>
#include <Vulkan-Device.hpp>

namespace TiRHI::Vulkan
{
    class RHI : public TiRHI::BaseRHI<RHI>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        RHI(const RhiCreate& rhiCreate);
        ~RHI() = default;

        Device newDevice();

        Instance& getInstance()
        {
            return m_instance;
        }

        vk::Instance getNativeInstance()
        {
            return m_instance.getInstance();
        }

        std::vector<vk::PhysicalDevice> getValidPhysicalDevices();

        [[nodiscard]] RhiApi getApi() const
        {
            return RhiApi::Vulkan;
        }

    private:
        Instance m_instance;

        void enumerateAvailableAdatper();

        bool isPhysicalDeviceValid(vk::PhysicalDevice physicalDevice);
    };
}

#endif // TITANIUM_VULKAN_RHI_H
