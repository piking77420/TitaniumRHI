#ifndef TITANIUM_METAL_RENDER_TARGETS_H
#define TITANIUM_METAL_RENDER_TARGETS_H

#include <Titanium/RHI-BaseRenderTargets.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class RenderPassDescriptor;
    class Texture;

    class RenderTargets : public BaseRenderTargets<RenderTargets, RHI, RenderPassDescriptor, Texture>
    {
    public:
        RenderTargets() = delete;
        ~RenderTargets();
        RenderTargets(RHI& rhi);
    private:
    };

    
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_RENDER_TARGETS_H