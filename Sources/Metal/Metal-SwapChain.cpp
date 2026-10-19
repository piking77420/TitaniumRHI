#include <Metal/Metal-SwapChain.hpp>
#include <Metal/Metal-RHI.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-Texture.hpp>

namespace TiRHI::Metal
{
    SwapChain::SwapChain(RHI& rhi) 
        : BaseSwapChain(rhi)
    {

    }  

    SwapChain::~SwapChain() = default;

} // namespace TiRHI::Metal