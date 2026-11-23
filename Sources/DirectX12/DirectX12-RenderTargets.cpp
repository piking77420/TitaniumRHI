#include <DirectX12/DirectX12-RenderTargets.hpp>

#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-Texture.hpp>

namespace TiRHI::DirectX12
{
    RenderTargets::RenderTargets(Device& device)
        : BaseRenderTargets(device)
    {
    }

    bool RenderTargets::build()
    {

        return true;
    }

} // TiRHI::DirectX12
