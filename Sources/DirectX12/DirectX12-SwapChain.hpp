#ifndef TITANIUM_DIRECTX12_SWAPCHAIN_H
#define TITANIUM_DIRECTX12_SWAPCHAIN_H

#include <array>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <Titanium/RHI-BaseSwapChain.hpp>
#include <DirectX12/DirectX12-Header.hpp>

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

        bool beginFrame();

        bool present(Device& device);

    private:
        MComPtr<IDXGISwapChain3> m_swapchain;
        uint32_t m_swapchainFrameIndex = 0u;

        std::array<MComPtr<ID3D12Resource>, BufferCount> m_images;
        std::array<uint64_t, BufferCount> swapchainFenceValues{0u};

        struct Synchronisation
        {
            HANDLE swapchainFenceEvent = nullptr;
            MComPtr<ID3D12Fence> swapchainFence;
        } m_synchronisation;

        bool createSwapChain(Device& device, WindowHandle windowHandle);

        bool queryBuffer();

        bool initSynchronisation(Device& device);
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_SWAPCHAIN_H
