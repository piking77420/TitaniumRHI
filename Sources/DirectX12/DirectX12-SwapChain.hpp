#ifndef TITANIUM_DIRECTX12_SWAPCHAIN_H
#define TITANIUM_DIRECTX12_SWAPCHAIN_H

#include <array>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <Titanium/RHI-BaseSwapChain.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-AcquireFrame.hpp>

namespace TiRHI::DirectX12
{
    class Device;
    class RHI;
    class Surface;

    class SwapChain : public BaseSwapChain<SwapChain, RHI>
    {
    public:
        SwapChain() = delete;
        ~SwapChain();
        SwapChain(RHI& rhi);

        bool build(Device& device, Surface& surface);

        AcquiredFrame acquireNextImage();

        bool present();

        bool recreateSwapChain(Device& device, Surface& surface);

        // TODO TO DELTE
        ID3D12Resource* getNativeCurrentBackBuffer() const;

        D3D12_CPU_DESCRIPTOR_HANDLE getRtv() const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

            rtv.ptr += m_swapchainFrameIndex * m_rtvDescriptorSize;

            return rtv;
        }
        //
    private:
        MComPtr<IDXGISwapChain3> m_swapchain;
        uint32_t m_swapchainFrameIndex = 0u;

        std::vector<MComPtr<ID3D12Resource>> m_images;

        MComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        UINT m_rtvDescriptorSize = 0;

        bool createSwapChain(Device& device, Surface& surface);

        bool queryBuffer();

        bool initSynchronisation(Device& device);

        bool createRenderTarget(Device& device);
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_SWAPCHAIN_H
