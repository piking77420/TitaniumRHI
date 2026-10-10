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
        explicit AcquiredFrame(bool success)
            : BaseAcquiredFrame(success)
        {
        }

        uint32_t getSwapChainImageIndex() const
        {
            return m_swapChainImageIndex;
        }

        AcquiredFrame& setSwapChainImageIndex(uint32_t imageIndex)
        {
            m_swapChainImageIndex = imageIndex;
            return *this;
        }

        bool operator()() const
        {
            return BaseAcquiredFrame::operator bool();
        }

    private:
        uint32_t m_swapChainImageIndex = 0;
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_ACQUIRE_FRAME_H
