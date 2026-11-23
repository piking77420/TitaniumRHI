#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    RenderPassDescriptor::RenderPassDescriptor(Device& rhi)
        : BaseRenderPassDescriptor(rhi)
    {
    }

    bool RenderPassDescriptor::build()
    {
        return true;
    }
        
} // TiRHI::DirectX12
