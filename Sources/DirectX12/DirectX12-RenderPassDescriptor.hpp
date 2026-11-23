#ifndef TITANIUM_DIRECTX12_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_DIRECTX12_RENDER_PASS_DESCRIPTOR_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseRenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class RenderPassDescriptor : public BaseRenderPassDescriptor<RenderPassDescriptor, Device>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        RenderPassDescriptor() = delete;
        ~RenderPassDescriptor() = default;
        RHI_MOVE_ONLY(RenderPassDescriptor);
        RenderPassDescriptor(Device& device);

        bool build();

    private:
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_RENDER_PASS_DESCRIPTOR_H
