#ifndef TITANIUM_VULKAN_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_VULKAN_RENDER_PASS_DESCRIPTOR_H

#include <vulkan/vulkan.hpp>
#include <Titanium/RHI-BaseRenderPassDescriptor.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class RenderPassDescriptor : public BaseRenderPassDescriptor<RenderPassDescriptor, RHI>
    {
    public:
        RenderPassDescriptor() = delete;
        ~RenderPassDescriptor() = default;
        explicit RenderPassDescriptor(RHI& rhi);

        bool build(Device& device);

        vk::RenderPass getNativeRenderPass() const
        {
            return m_renderPass.get();
        }

    private:
        vk::UniqueRenderPass m_renderPass;

        bool createRenderPass(vk::Device device);
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_RENDER_PASS_DESCRIPTOR_H
