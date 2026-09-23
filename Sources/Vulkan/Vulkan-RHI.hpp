#ifndef TITANIUM_VULKAN_RHI_H
#define TITANIUM_VULKAN_RHI_H

#include <Vulkan-Instance.hpp>
#include <Vulkan-Device.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    class RHI
    {
    public:
        RHI(const TiRHI::RhiCreate& create);
        ~RHI();

        void waitForDeviceIdle();

    private:
        Vulkan::Instance m_instance;
        Vulkan::Device m_device;
    };
}

#endif // TITANIUM_VULKAN_RHI_H
