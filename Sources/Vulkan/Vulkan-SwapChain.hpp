#ifndef TITANIUM_VULKAN_SWAPCHAIN_H
#define TITANIUM_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.hpp>
#include <Titanium/RHI-BaseSwapChain.hpp>
#include <Vulkan/Vulkan-AcquiredFrame.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class Surface;
    class CommandList;

    class SwapChain : public BaseSwapChain<SwapChain, RHI, Vulkan::RenderPassDescriptor>
    {
    public:
        SwapChain() = delete;
        ~SwapChain() = default;

        explicit SwapChain(RHI& rhi);

        bool build(Device& device, Surface& surface);

        AcquiredFrame acquireNextImage();

        bool present();

        bool recreateSwapChain(Device& device, Surface& surface);

        bool beginRenderTargets(CommandList& commandList);

        void endRenderTargets(CommandList& commandList);

        uint32_t getImageIndex() const
        {
            return m_imageIndex;
        }

        vk::Semaphore getNativeImageAvailableSemaphore() const;

        vk::Semaphore getNativeRenderFinishedSemaphore() const;

        // TODO DELTE

        vk::Framebuffer getNativeFrameBuffer() const;
        //
    private:
        vk::UniqueSwapchainKHR m_swapchain;

        std::vector<vk::Image> m_images;

        std::vector<vk::UniqueImageView> m_imageViews;

        std::vector<vk::UniqueFramebuffer> m_frameBuffers;

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

        bool createRenderPassDescriptor(Device& device);

        bool createFrameBuffer(Device& device);

        bool createSyncObjects(vk::Device device);
    };

} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SWAPCHAIN_H
