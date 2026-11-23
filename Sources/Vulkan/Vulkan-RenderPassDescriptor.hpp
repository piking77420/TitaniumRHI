#ifndef TITANIUM_VULKAN_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_VULKAN_RENDER_PASS_DESCRIPTOR_H

#include <Vulkan/Vulkan-Header.hpp>
#include <Titanium/RHI-BaseRenderPassDescriptor.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class RenderPassDescriptor : public BaseRenderPassDescriptor<RenderPassDescriptor, Device>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        RenderPassDescriptor() = delete;
        ~RenderPassDescriptor() = default;
        RenderPassDescriptor(const RenderPassDescriptor&) = delete;
        RenderPassDescriptor& operator=(const RenderPassDescriptor&) = delete;
        RenderPassDescriptor(RenderPassDescriptor&&) noexcept = default;
        RenderPassDescriptor& operator=(RenderPassDescriptor&&) noexcept = default;
        explicit RenderPassDescriptor(Device& device);

        bool build();

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
