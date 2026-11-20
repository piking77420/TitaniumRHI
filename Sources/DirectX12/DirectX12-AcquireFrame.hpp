#ifndef TITANIUM_DIRECTX12_ACQUIRE_FRAME_H
#define TITANIUM_DIRECTX12_ACQUIRE_FRAME_H

#include <Titanium/RHI-BaseAcquiredFrame.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class AcquiredFrame : public BaseAcquiredFrame
    {
    public:
        explicit AcquiredFrame() = default;
        ~AcquiredFrame() = default;
        explicit AcquiredFrame(bool success, IDXGISwapChain3* swapChainNativeHandle, bool vsync)
            : BaseAcquiredFrame(success)
            , m_swapChainNativeHandle(swapChainNativeHandle)
            , m_vsync(vsync)
        {
        }

        IDXGISwapChain3* getSwapChainNativeHandle() const
        {
            return m_swapChainNativeHandle;
        }

        AcquiredFrame& setSwapChainNativeHandle(IDXGISwapChain3* iDXGISwapChain3)
        {
            m_swapChainNativeHandle = iDXGISwapChain3;
            return *this;
        }

        bool getVsync() const
        {
            return m_vsync;
        }

        AcquiredFrame& setVsync(bool newVsync)
        {
            m_vsync = newVsync;
            return *this;
        }

        bool operator()() const
        {
            return BaseAcquiredFrame::operator bool();
        }

    private:
        uint32_t m_swapChainImageIndex = 0;

        IDXGISwapChain3* m_swapChainNativeHandle{nullptr};

        bool m_vsync = false;
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_ACQUIRE_FRAME_H
