#ifndef TITANIUM_VULKAN_SWAPCHAIN_H
#define TITANIUM_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.hpp>
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

    class SwapChain : public BaseSwapChain<SwapChain, RHI, Device, Vulkan::RenderPassDescriptor, Vulkan::RenderTargets,
                                           Vulkan::Texture>
    {
    public:
        SwapChain() = delete;
        ~SwapChain() = default;
        RHI_MOVE_ONLY(SwapChain)
        explicit SwapChain(RHI& rhi);

        bool build(Device& device, Surface& surface);

        AcquiredFrame acquireNextImage();

        bool present();

        bool recreateSwapChain(Device& device, Surface& surface);

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
        vk::UniqueSwapchainKHR m_swapchain;

        std::vector<vk::UniqueImageView> m_imageViews;

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
        };

        std::vector<Synchronisation> m_synchronisations;

        uint32_t m_imageIndex = 0;

        vk::Queue m_presentQueue = VK_NULL_HANDLE;

        vk::Device m_device = VK_NULL_HANDLE;

        vk::SurfaceFormatKHR getSurfaceFormat() const noexcept;

        vk::PresentModeKHR getPresentMode() const noexcept;

        vk::Extent2D getExtent2D() const noexcept;

        vk::SwapchainCreateInfoKHR getSwapChainCreateInfo(Device& device, Surface& surface) const;

        bool createRenderTargets(Device& device);

        bool createSyncObjects(vk::Device device);
    };

} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SWAPCHAIN_H
