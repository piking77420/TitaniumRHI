#ifndef TITANIUM_METAL_RHI_H
#define TITANIUM_METAL_RHI_H

#include <Metal/Metal.hpp>
#include <MetalFX/MetalFX.hpp>
#include <Metal/Metal.hpp>

#include <Titanium/RHI.hpp>

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

#endif // TITANIUM_METAL_RHI_H
