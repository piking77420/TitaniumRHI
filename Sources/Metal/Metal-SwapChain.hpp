#ifndef TITANIUM_METAL_SWAPCHAIN_H
#define TITANIUM_METAL_SWAPCHAIN_H

#include <Titanium/RHI-BaseSwapChain.hpp>
#include <Metal/Metal-RenderPassDescriptor.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class Device;
    class RenderTarget;
    class Texture;

    class SwapChain : public BaseSwapChain<SwapChain, RHI, Device, RenderPassDescriptor, RenderTarget, Texture>
    {
    public:
        SwapChain() = delete;
        ~SwapChain();
        RHI_MOVE_ONLY(SwapChain)
        SwapChain(RHI& rhi);

        bool build();

    private:
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_SWAPCHAIN_H