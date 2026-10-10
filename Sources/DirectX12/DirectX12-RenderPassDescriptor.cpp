#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    RenderPassDescriptor::RenderPassDescriptor(RHI& rhi)
        : BaseRenderPassDescriptor(rhi)
    {
    }

    bool RenderPassDescriptor::build([[maybe_unused]] Device&)
    {
        return true;
    }

} // TiRHI::DirectX12
