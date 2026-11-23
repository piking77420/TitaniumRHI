#ifndef TITANIUM_VULKAN_SWAPCHAIN_H
#define TITANIUM_VULKAN_SWAPCHAIN_H

#include <Vulkan/Vulkan-Header.hpp>
#include <Titanium/RHI-BaseSwapChain.hpp>
#include <Vulkan/Vulkan-AcquiredFrame.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Vulkan-RenderTargets.hpp>
#include <Vulkan/Vulkan-Texture.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class Surface;
    class CommandList;

    class SwapChain : public BaseSwapChain<SwapChain, Device, Vulkan::RenderPassDescriptor,
                                           Vulkan::RenderTargets,
                                           Vulkan::Texture>
    {
    public:
        SwapChain() = delete;
        ~SwapChain();
        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;
        SwapChain(SwapChain&&) noexcept;
        SwapChain& operator=(SwapChain&&) noexcept;
        explicit SwapChain(Device& device);

        bool build(Surface& surface);

        AcquiredFrame acquireNextImage();

        bool recreateSwapChain(Surface& surface);

        const RenderTargets& getCurrentRenderTargets() const;

        RenderTargets& getCurrentRenderTargets();

        const Texture& getCurrentSwapChainTexture() const;

        Texture& getCurrentSwapChainTexture();

        uint32_t getImageIndex() const
        {
            return m_imageIndex;
        }

        vk::Semaphore getNativeImageAvailableSemaphore() const;

        vk::Semaphore getNativeRenderFinishedSemaphore() const;

    private:
        Device* m_device{nullptr};

        vk::UniqueSwapchainKHR m_swapchain;

        vk::SurfaceFormatKHR m_currentFormat;

        struct SwapChainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        } m_swapChainSupportDetails;

        struct Synchronisation
        {
            vk::UniqueSemaphore imageAvailableSemaphore;
            vk::UniqueSemaphore renderFinishedSemaphore;
            vk::Fence imageInFlight;
        };

        std::vector<Synchronisation> m_synchronisations;

        uint32_t m_imageIndex = 0;

        vk::Queue m_presentQueue = VK_NULL_HANDLE;

        uint64_t m_submisionValue = 0;

        vk::SurfaceFormatKHR getSurfaceFormat() const noexcept;

        vk::PresentModeKHR getPresentMode() const noexcept;

        vk::Extent2D getExtent2D() const noexcept;

        vk::SwapchainCreateInfoKHR getSwapChainCreateInfo(Surface& surface) const;

        bool createRenderTargets();

        bool createSyncObjects();
    };

} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SWAPCHAIN_H
