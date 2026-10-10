#ifndef TITANIUM_DIRECTX12_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_DIRECTX12_RENDER_PASS_DESCRIPTOR_H

#include <Titanium/RHI-BaseRenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class RenderPassDescriptor : public BaseRenderPassDescriptor<RenderPassDescriptor, RHI>
    {
    public:
        RenderPassDescriptor() = delete;
        ~RenderPassDescriptor() = default;
        RHI_MOVE_ONLY(RenderPassDescriptor)
        RenderPassDescriptor(RHI& rhi);

        bool build(Device&);

    private:
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_RENDER_PASS_DESCRIPTOR_H
