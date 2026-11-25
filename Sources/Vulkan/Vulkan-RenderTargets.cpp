#include <Vulkan/Vulkan-RenderTargets.hpp>

#include <vulkan/vulkan.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Vulkan/Vulkan-Texture.hpp>

namespace TiRHI::Vulkan
{
    RenderTargets::~RenderTargets()
    {
        auto& deferredDestroy = Device::Private::getDeferredDeletionQueue(getDevice());
        deferredDestroy.retire(getLastFrameUsed(),
                               [frameBuffer = std::move(m_frameBuffer)]() mutable
                               {
                                  
                               });
    }

    bool RenderTargets::build()
    {
        if (!BaseRenderTargets::build())
            return false;

        const RenderPassDescriptor* renderPassDescriptor = getRenderPassDescriptor();

        if (!getRenderPassDescriptor())
            return false;


        std::vector<vk::ImageView> views; 
        views.reserve(renderPassDescriptor->getAttachementCount());

        for (const auto& color : getColorAttachements())
        {
            if (color)
                views.emplace_back(Texture::Private::getImageView(*color));
        }

        if (const std::optional<const Texture*> texture = getDepthAttachement(); texture)
        {
            views.emplace_back(Texture::Private::getImageView(**texture));
        }

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.setRenderPass(getRenderPassDescriptor()->getNativeRenderPass())
            .setAttachments(views)
            .setWidth(getWidth())
            .setHeight(getHeight())
            .setLayers(1);

        m_frameBuffer = getDevice().getNativeDevice().createFramebufferUnique(framebufferInfo);

        if (!m_frameBuffer)
        {
            RHI_LOG_ERROR(std::format(L"Failed to crate Framebuffer {}", getNameW()), RhiApi::Vulkan);
            return false;
        }

        vk::DebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.objectType = vk::ObjectType::eFramebuffer;
        nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkFramebuffer>(m_frameBuffer.get()));
        nameInfo.pObjectName = getName().data();

        getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);
            
        return m_frameBuffer.get();
    }

} // TiRHI::Vulkan
