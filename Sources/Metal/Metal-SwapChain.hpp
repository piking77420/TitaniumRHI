#ifndef TITANIUM_METAL_SWAPCHAIN_H
#define TITANIUM_METAL_SWAPCHAIN_H

#include <Titanium/RHI-BaseSwapChain.hpp>
#include <Metal/Metal-RenderPassDescriptor.hpp>
#include <Metal/Metal-RenderTargets.hpp>

namespace CA
{
    class MetalLayer;
} // namespace CA

namespace TiRHI::Metal
{
    class RHI;
    class Device;
    class Texture;
    class Surface;

    class SwapChain : public BaseSwapChain<SwapChain, RHI, Device, RenderPassDescriptor, RenderTargets, Texture>
    {
    public:
        SwapChain() = delete;
        ~SwapChain();
        RHI_MOVE_ONLY(SwapChain)
        SwapChain(RHI& rhi);

        bool build(Device& device, Surface& surface);

    private:
        CA::MetalLayer* m_metalLayer{nullptr};
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_SWAPCHAIN_H