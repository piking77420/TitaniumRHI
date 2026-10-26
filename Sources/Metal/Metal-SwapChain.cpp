#include <Metal/Metal-SwapChain.hpp>
#include <Metal/Metal-RHI.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-Texture.hpp>
#include <Metal/Metal-Header.hpp>
#include <Metal/Metal-Device.hpp>

namespace TiRHI::Metal
{
    SwapChain::SwapChain(RHI& rhi) 
        : BaseSwapChain(rhi)
    {

    }  

    SwapChain::~SwapChain() = default;

    bool SwapChain::build(Device& device, Surface& surface)
    {
        /*
        CA::MetalLayer* metalLayer = CA::MetalLayer::layer();
        MTL::Device* mltDevice = Device::Private::getDevice(device); 
        if (!mltDevice)
        {
            return false;
        }
        metalLayer->setDevice(mltDevice);
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        metalLayer->setDrawableSize(CGSizeMake(static_cast<float>(getWidth()), static_cast<float>(getHeight())));

        NS::View* contentView = reinterpret_cast<NS::Window*>->contentView();
#if defined(TITANIUM_VALIDATION_LAYER)
contentView->setWantsLayer(true);
#else
contentView->setWantsLayer(false);
#endif // TITANIUM_VALIDATION_LAYER
        contentView->setLayer(metalLayer);
*/
        return m_metalLayer;
    }

} // namespace TiRHI::Metal