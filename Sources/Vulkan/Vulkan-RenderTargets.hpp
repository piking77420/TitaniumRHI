#ifndef TITANIUM_VULKAN_RENDER_TARGETS_H
#define TITANIUM_VULKAN_RENDER_TARGETS_H

#include <Titanium/RHI-BaseRenderTargets.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Private/FrameTracked.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class Texture;

    class RenderTargets : public BaseRenderTargets<RenderTargets, Device, Vulkan::RenderPassDescriptor, Vulkan::Texture>,
                          public Private::FrameTracked<RenderTargets>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        RenderTargets() = delete;
        ~RenderTargets();
        RenderTargets(const RenderTargets&) = delete;
        RenderTargets& operator=(const RenderTargets&) = delete;
        RenderTargets(RenderTargets&&) noexcept = default;
        RenderTargets& operator=(RenderTargets&& other) noexcept = default;

        RenderTargets(Device& device)
            : BaseRenderTargets(device)
        {
        }

        struct Private
        {
            static vk::Framebuffer getFrameBuffer(const RenderTargets& renderTargets)
            {
                return renderTargets.m_frameBuffer.get();
            }
        };

        bool build();

    private:
        friend Private;

        vk::UniqueFramebuffer m_frameBuffer;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_RENDER_TARGETS_H
