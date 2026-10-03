#include <Vulkan/Vulkan-SwapChain.hpp>
#include <vulkan/vulkan.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <vulkan/Vulkan-Instance.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Vulkan-Surface.hpp>

namespace TiRHI::Vulkan
{
    SwapChain::SwapChain(RHI& rhi)
        : BaseSwapChain(rhi)
    {
    }

    bool SwapChain::build(Device& device, Surface& surface)
    {
        return recreateSwapChain(device, surface) && createFrameBuffer(device.getNativeDevice());
    }

    bool SwapChain::beginFrame()
    {
    }

    bool SwapChain::present(Device& device)
    {
    }

    vk::SwapchainCreateInfoKHR SwapChain::getSwapChainCreateInfo(Device& device, Surface& surface) const
    {
        vk::Device vkDevice = device.getNativeDevice();
        vk::PhysicalDevice physicalDevice = device.getNativePhysicalDevice();
        vk::SurfaceKHR vkSurface = surface.getSurfaceNative();

        vk::SurfaceFormatKHR format = getSurfaceFormat();
        vk::PresentModeKHR presentMode = getPresentMode();
        vk::Extent2D getExtend = getExtent2D();

        vk::SwapchainCreateInfoKHR vkSwapChainCreateInfo{};
        vkSwapChainCreateInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        // clang-format off
        vkSwapChainCreateInfo
            .setSurface(vkSurface)
            .setMinImageCount(m_imageCount)
            .setImageFormat(format.format)
            .setImageColorSpace(format.colorSpace)
            .setImageExtent(1)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(
                device.getNativePresentQueue() == device.getNativeGraphicQueue() 
                ? vk::SharingMode::eExclusive 
                : vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(1)
            .setPreTransform(m_swapChainSupportDetails.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(vk::True);

        // clang-format on

        return vkSwapChainCreateInfo;
    }

    bool SwapChain::createRenderPass(vk::Device device)
    {
        m_renderPassState.currentFormat = m_currentFormat;

        vk::AttachmentDescription attachmentDescription{};
        attachmentDescription.format = m_renderPassState.currentFormat.format;
        attachmentDescription.samples = vk::SampleCountFlagBits::e1;
        attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
        attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
        attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
        attachmentDescription.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpassDesc{};
        subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorRef;

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDesc;

        m_renderPassState.renderPass = device.createRenderPassUnique(renderPassInfo);

        RHI_LOG_VERBOSE(L"Create Swapchain RenderPass", RhiApi::Vulkan);

        return true;
    }

    bool SwapChain::recreateSwapChain(Device& device, Surface& surface)
    {
        vk::Device vkDevice = device.getNativeDevice();
        vk::PhysicalDevice physicalDevice = device.getNativePhysicalDevice();
        vk::SurfaceKHR vkSurface = surface.getSurfaceNative();

        m_swapChainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(vkSurface);
        m_swapChainSupportDetails.formats = physicalDevice.getSurfaceFormatsKHR(vkSurface);
        m_swapChainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(vkSurface);

        m_imageCount = std::max(m_imageCount, m_swapChainSupportDetails.capabilities.minImageCount);

        if (m_swapChainSupportDetails.capabilities.maxImageCount > 0)
        {
            m_imageCount = std::min(m_imageCount, m_swapChainSupportDetails.capabilities.maxImageCount);
        }

        if (m_renderPassState.currentFormat != m_currentFormat)
        {
            if (!createRenderPass(device.getNativeDevice()))
                return false;
        }

        vk::SwapchainCreateInfoKHR createInfo = getSwapChainCreateInfo(device, surface);
        vk::Device vkDevice = device.getNativeDevice();

        m_swapchain.reset(vkDevice.createSwapchainKHR(createInfo));

        if (!m_swapchain)
        {
            RHI_LOG_ERROR(std::format(L"Failed to create SwapChain {}", getNameW()), RhiApi::Vulkan);
            return false;
        }

        m_images = vkDevice.getSwapchainImagesKHR(*m_swapchain);
        m_imageViews.reserve(m_images.size());

        for (vk::Image image : m_images)
        {
            vk::ImageViewCreateInfo viewInfo{};

            viewInfo.setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(createInfo.imageFormat)
                .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

            m_imageViews.push_back(vkDevice.createImageViewUnique(viewInfo));
        }

        return true;
    }

    vk::SurfaceFormatKHR SwapChain::getSurfaceFormat() const noexcept
    {
        for (const auto& availableFormat : m_swapChainSupportDetails.formats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT)
            {
                return availableFormat;
            }
        }

        return m_swapChainSupportDetails.formats[0];
    }

    vk::PresentModeKHR SwapChain::getPresentMode() const noexcept
    {
        for (const auto& presentMode : m_swapChainSupportDetails.presentModes)
        {
            if (getVsync() && presentMode == vk::PresentModeKHR::eMailbox)
                return presentMode;
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D SwapChain::getExtent2D() const noexcept
    {
        vk::Extent2D ext;

        ext.width = std::clamp(getWidth(), m_swapChainSupportDetails.capabilities.minImageExtent.width,
                               m_swapChainSupportDetails.capabilities.maxImageExtent.width);
        ext.height = std::clamp(getHeight(), m_swapChainSupportDetails.capabilities.minImageExtent.height,
                                m_swapChainSupportDetails.capabilities.maxImageExtent.height);
        return ext;
    }

    bool SwapChain::createFrameBuffer(vk::Device device)
    {
        const vk::Extent2D ext = getExtent2D();
        m_frameBuffers.reserve(m_imageViews.size());
        for (size_t i = 0; i < m_imageViews.size(); ++i)
        {
            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.setRenderPass(m_renderPassState.renderPass.get())
                .setAttachments(m_imageViews[i].get())
                .setWidth(ext.width)
                .setHeight(ext.height)
                .setLayers(1);
            auto& frameBuffer = m_frameBuffers.emplace_back(device.createFramebufferUnique(framebufferInfo));

            if (!frameBuffer)
            {
                RHI_LOG_ERROR(std::format(L"Failed to crate Framebuffer {}", i), RhiApi::Vulkan);
                return false;
            }
        }

        return true;
    }

} // TiRHI::Vulkan
