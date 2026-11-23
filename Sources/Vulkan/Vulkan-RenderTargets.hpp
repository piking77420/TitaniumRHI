#ifndef TITANIUM_VULKAN_RENDER_TARGETS_H
#define TITANIUM_VULKAN_RENDER_TARGETS_H

#include <Titanium/RHI-BaseRenderTargets.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class Texture;

    class RenderTargets : public BaseRenderTargets<RenderTargets, Device, Vulkan::RenderPassDescriptor, Vulkan::Texture>
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

        void setSubmissionValue(uint64_t value)
        {
            m_lastSubmitedValue = value;
        }

    private:
        friend Private;

        vk::UniqueFramebuffer m_frameBuffer;

        uint64_t m_lastSubmitedValue = 0;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_RENDER_TARGETS_H
