#ifndef TITANIUM_VULKAN_ACQUIRE_FRAME_H
#define TITANIUM_VULKAN_ACQUIRE_FRAME_H

#include <vulkan/vulkan.hpp>
#include <Titanium/RHI-BaseAcquiredFrame.hpp>

namespace TiRHI::Vulkan
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

        vk::Semaphore getImageAvailableSemaphore() const
        {
            return m_imageAvailabe;
        }

        AcquiredFrame& setImageAvailableSemaphore(vk::Semaphore semaphore)
        {
            m_imageAvailabe = semaphore;
            return *this;
        }

        vk::Semaphore getRenderFinishSemaphore() const
        {
            return m_renderFinishSemaphore;
        }

        AcquiredFrame& setRenderFinishSemaphore(vk::Semaphore semaphore)
        {
            m_renderFinishSemaphore = semaphore;
            return *this;
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

        explicit operator bool() const
        {
            return BaseAcquiredFrame::operator bool();
        }

    private:
        vk::Semaphore m_renderFinishSemaphore;

        vk::Semaphore m_imageAvailabe;

        uint32_t m_swapChainImageIndex = 0;
    };
} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_ACQUIRE_FRAME_H
