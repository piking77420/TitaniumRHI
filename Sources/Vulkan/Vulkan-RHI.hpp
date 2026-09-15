#ifndef TITANIUM_VULKAN_RHI_H
#define TITANIUM_VULKAN_RHI_H

#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI
{
    class RHI
    {
    public:
        RHI();
        ~RHI();

        void waitForDeviceIdle();

    private:
    };
}

#endif // TITANIUM_VULKAN_RHI_H
