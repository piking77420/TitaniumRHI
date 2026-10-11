#ifndef TITANIUM_DIRECTX12_SWAPCHAIN_H
#define TITANIUM_DIRECTX12_SWAPCHAIN_H

#include <array>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <Titanium/RHI-BaseSwapChain.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-AcquireFrame.hpp>
#include <DirectX12/DirectX12-RenderTargets.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>
#include <DirectX12/DirectX12-Texture.hpp>

namespace TiRHI::DirectX12
{
    class Device;
    class RHI;
    class Surface;

    class SwapChain : public BaseSwapChain<SwapChain, RHI, Device, RenderPassDescriptor, RenderTargets, Texture>
    {
    public:
        SwapChain() = delete;
        ~SwapChain();
        RHI_MOVE_ONLY(SwapChain)
        SwapChain(RHI& rhi);

        bool build(Device& device, Surface& surface);

        AcquiredFrame acquireNextImage();

        bool present();

        bool recreateSwapChain(Device& device, Surface& surface);

        const RenderTargets& getCurrentRenderTargets() const;

        RenderTargets& getCurrentRenderTargets();

        const Texture& getCurrentSwapChainTexture() const;

        Texture& getCurrentSwapChainTexture();

    private:
        MComPtr<IDXGISwapChain3> m_swapchain;
        uint32_t m_swapchainFrameIndex = 0u;

        bool createSwapChain(Device& device, Surface& surface);

        bool queryBuffer(Device& device);

        bool createRenderTarget(Device& device);
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_SWAPCHAIN_H
