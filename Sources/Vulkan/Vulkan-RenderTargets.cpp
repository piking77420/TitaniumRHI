#include <Vulkan/Vulkan-RenderTargets.hpp>

#include <vulkan/vulkan.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    bool RenderTargets::build(Device& device)
    {

        // TODO

        return true;
    }

    bool RenderTargets::build(Device& device, vk::ImageView imageView)
    {
        if (!BaseRenderTargets::build())
            return false;

        // m_renderPassDescriptor shoudl be valid
        assert(getRenderPassDescriptor());
        assert(imageView);

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.setRenderPass(getRenderPassDescriptor()->getNativeRenderPass())
            .setAttachments(imageView)
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

    bool createFrameBuffer(vk::Device device)
    {
        return true;
    }

} // TiRHI::Vulkan
