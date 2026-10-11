#include <DirectX12/DirectX12-RenderTargets.hpp>

#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-Texture.hpp>

namespace TiRHI::DirectX12
{
    RenderTargets::RenderTargets(RHI& rhi)
        : BaseRenderTargets(rhi)
    {
    }

    bool RenderTargets::build(Device& device)
    {

        return true;
    }

} // TiRHI::DirectX12
