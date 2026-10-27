#include <Metal/Metal-SwapChain.hpp>
#include <Metal/Metal-RHI.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-Surface.hpp>
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
        MTL::Device* mtlDevice = Device::Private::getDevice(device);
        if (!mtlDevice)
            return false;

        CA::MetalLayer* metalLayer = CA::MetalLayer::layer();

        metalLayer->setDevice(mtlDevice);
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        metalLayer->setFramebufferOnly(true);
        metalLayer->setMaximumDrawableCount(m_imageCount);
        metalLayer->setDisplaySyncEnabled(m_vsync);

        // Must be a native NSWindow*
        id cocoaWindow =
            reinterpret_cast<id>(
                Surface::Private::getWindowHandle(surface)
            );

        if (!cocoaWindow)
            return false;

        auto getContentView =
            reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);

        id view = getContentView(
            cocoaWindow,
            sel_registerName("contentView")
        );

        if (!view)
            return false;

        auto setWantsLayer =
            reinterpret_cast<void (*)(id, SEL, BOOL)>(objc_msgSend);

        auto setLayer =
            reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend);

        setWantsLayer(
            view,
            sel_registerName("setWantsLayer:"),
            YES
        );

        setLayer(
            view,
            sel_registerName("setLayer:"),
            reinterpret_cast<id>(metalLayer)
        );

        metalLayer->setDrawableSize(CGSizeMake(
            static_cast<CGFloat>(getWidth()),
            static_cast<CGFloat>(getHeight())
        ));

        m_metalLayer = metalLayer;
        m_metalLayer->retain();

        return true;
    }

} // namespace TiRHI::Metal