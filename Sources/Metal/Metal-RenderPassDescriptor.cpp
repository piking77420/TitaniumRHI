#include <Metal/Metal-RenderPassDescriptor.hpp>
#include <Metal/Metal-RHI.hpp>

namespace TiRHI::Metal
{
    RenderPassDescriptor::~RenderPassDescriptor() = default;
    RenderPassDescriptor::RenderPassDescriptor(RHI& rhi)
        : BaseRenderPassDescriptor(rhi)
    {
    }

} // namespace TiRHI::Metal