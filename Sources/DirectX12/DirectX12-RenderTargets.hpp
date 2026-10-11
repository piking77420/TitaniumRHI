#ifndef TITANIUM_DIRECTX12_RENDER_TARGETS_H
#define TITANIUM_DIRECTX12_RENDER_TARGETS_H

#include <span>
#include <vector>
#include <Titanium/RHI-BaseRenderTargets.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class Texture;

    class RenderTargets : public BaseRenderTargets<RenderTargets, RHI, RenderPassDescriptor, Texture>
    {
    public:
        RenderTargets() = delete;
        ~RenderTargets() = default;
        RHI_MOVE_ONLY(RenderTargets)
        RenderTargets(RHI& rhi);

        struct Private
        {
        };

        bool build(Device& device);

    private:
        friend Private;
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_RENDER_TARGETS_H
