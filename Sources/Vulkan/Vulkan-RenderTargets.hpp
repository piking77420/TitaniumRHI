#ifndef TITANIUM_VULKAN_RENDER_TARGETS_H
#define TITANIUM_VULKAN_RENDER_TARGETS_H

#include <Titanium/RHI-BaseRenderTargets.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class RenderTargets : public BaseRenderTargets<RenderTargets, RHI, Vulkan::RenderPassDescriptor>
    {
    public:
        RenderTargets() = delete;
        ~RenderTargets() = default;
        RHI_MOVE_CONSTRUCT_ONLY(RenderTargets);

        RenderTargets(RHI& rhi)
            : BaseRenderTargets(rhi)
        {
        }

        struct Private
        {
            static vk::Framebuffer getFrameBuffer(const RenderTargets& renderTargets)
            {
                return renderTargets.m_frameBuffer.get();
            }

            static bool build(RenderTargets& renderTargets, Device& device, vk::ImageView imageView)
            {
                return renderTargets.build(device, imageView);
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        vk::UniqueFramebuffer m_frameBuffer;

        bool createFrameBuffer(vk::Device device);

        // private builder
        bool build(Device& device, vk::ImageView imageView);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_RENDER_TARGETS_H
