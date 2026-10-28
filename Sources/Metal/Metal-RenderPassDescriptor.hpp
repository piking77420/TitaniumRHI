#ifndef TITANIUM_METAL_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_METAL_RENDER_PASS_DESCRIPTOR_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseRenderPassDescriptor.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class RenderPassDescriptor;

    class RenderPassDescriptor : public BaseRenderPassDescriptor<RenderPassDescriptor, RHI>
    {
    public:
        RenderPassDescriptor() = delete;
        ~RenderPassDescriptor();
        RHI_MOVE_ONLY(RenderPassDescriptor);
        RenderPassDescriptor(RHI& rhi);

        bool build();

    private:
    };

} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_RENDER_PASS_DESCRIPTOR_H