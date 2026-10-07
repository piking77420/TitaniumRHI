#ifndef TITANIUM_VULKAN_RENDER_TARGETS_H
#define TITANIUM_VULKAN_RENDER_TARGETS_H

#include <Titanium/RHI-BaseRenderTargets.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class RenderTargets : public BaseRenderTargets<RenderTargets, RHI>
    {
    public:
        RenderTargets() = delete;
        ~RenderTargets() = default;

        RenderTargets(RHI& rhi)
            : BaseRenderTargets<RenderTargets, RHI>(rhi)
        {
        }

        bool build(Device& device, const RenderPassDescriptor& renderPassDescriptor);

    private:
        vk::UniqueFramebuffer m_frameBuffer;

        bool createFrameBuffer(vk::Device device);

    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_RENDER_TARGETS_H
