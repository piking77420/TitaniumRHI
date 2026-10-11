#include <Vulkan/Vulkan-RenderTargets.hpp>

#include <vulkan/vulkan.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Vulkan/Vulkan-Texture.hpp>

namespace TiRHI::Vulkan
{
    bool RenderTargets::build(Device& device)
    {
        if (!BaseRenderTargets::build())
            return false;

        // m_renderPassDescriptor shoudl be valid
        assert(getRenderPassDescriptor());

        std::vector<vk::ImageView> views;
        views.push_back(Texture::Private::getImageView(*getColorAttachements()[0]));

        // TODO multiple attachement
        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.setRenderPass(getRenderPassDescriptor()->getNativeRenderPass())
            .setAttachments(views)
            .setWidth(getWidth())
            .setHeight(getHeight())
            .setLayers(1);

        m_frameBuffer = device.getNativeDevice().createFramebufferUnique(framebufferInfo);

        if (!m_frameBuffer)
        {
            RHI_LOG_ERROR(std::format(L"Failed to crate Framebuffer {}", getNameW()), RhiApi::Vulkan);
            return false;
        }

        return m_frameBuffer.get();
    }

} // TiRHI::Vulkan
