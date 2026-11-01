#ifndef TITANIUM_METAL_SWAPCHAIN_H
#define TITANIUM_METAL_SWAPCHAIN_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseSwapChain.hpp>
#include <Metal/Metal-RenderPassDescriptor.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-AcquiredFrame.hpp>

namespace CA
{
    class MetalLayer;
    class MetalDrawable;
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
        RHI_MOVE_ONLY(SwapChain);
        SwapChain(RHI& rhi);

        bool build(Device& device, Surface& surface);

        AcquiredFrame acquireNextImage(Device& device);

        bool present();

        bool recreateSwapChain(Device& device, Surface& surface);

        const RenderTargets& getCurrentRenderTargets() const;

        RenderTargets& getCurrentRenderTargets();

        const Texture& getCurrentSwapChainTexture() const;

        Texture& getCurrentSwapChainTexture();

    private:
        CA::MetalLayer* m_metalLayer{nullptr};

        CA::MetalDrawable* m_drawable{nullptr};

        bool initRenderPassDescriptor();

        bool initRenderTargets();
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_SWAPCHAIN_H