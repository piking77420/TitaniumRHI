#ifndef TITANIUM_VULKAN_RHI_H
#define TITANIUM_VULKAN_RHI_H

#include <Vulkan-Instance.hpp>

namespace TiRHI
{
    class RHI
    {
    public:
        RHI();
        ~RHI();

        void waitForDeviceIdle();

    private:
        Vulkan::Instance m_instance;
    };
}

#endif // TITANIUM_VULKAN_RHI_H
