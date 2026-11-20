#include <Metal/Metal-SwapChain.hpp>
#include <Metal/Metal-RHI.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-Surface.hpp>
#include <Metal/Metal-Texture.hpp>
#include <Metal/Metal-Header.hpp>
#include <Metal/Metal-Device.hpp>
#include <Metal/Private/RhiToMetal.hpp>

namespace TiRHI::Metal
{
    SwapChain::SwapChain(RHI& rhi)
        : BaseSwapChain(rhi)
    {
    }

    SwapChain::~SwapChain()
    {
        if (m_metalLayer)
            m_metalLayer->release();
    }

    bool SwapChain::build(Device& device, Surface& surface)
    {
        MTL::Device* mtlDevice = Device::Private::getDevice(device);
        if (!mtlDevice)
            return false;

        CA::MetalLayer* metalLayer = CA::MetalLayer::layer();

        metalLayer->setDevice(mtlDevice);
        metalLayer->setPixelFormat(Private::toMetal(m_format));
        metalLayer->setFramebufferOnly(true);
        metalLayer->setMaximumDrawableCount(m_imageCount);
        metalLayer->setDisplaySyncEnabled(m_vsync);

        // Must be a native NSWindow*
        id cocoaWindow = reinterpret_cast<id>(Surface::Private::getWindowHandle(surface));

        if (!cocoaWindow)
            return false;

        auto getContentView = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);

        id view = getContentView(cocoaWindow, sel_registerName("contentView"));

        if (!view)
            return false;

        auto setWantsLayer = reinterpret_cast<void (*)(id, SEL, BOOL)>(objc_msgSend);

        auto setLayer = reinterpret_cast<void (*)(id, SEL, id)>(objc_msgSend);

        setWantsLayer(view, sel_registerName("setWantsLayer:"), YES);

        setLayer(view, sel_registerName("setLayer:"), reinterpret_cast<id>(metalLayer));

        metalLayer->setDrawableSize(CGSizeMake(static_cast<CGFloat>(getWidth()), static_cast<CGFloat>(getHeight())));

        m_metalLayer = metalLayer;
        m_metalLayer->retain();

        m_textures.emplace_back(getRHI());

        return true;
    }

    AcquiredFrame SwapChain::acquireNextImage(Device& device)
    {
        if (!m_metalLayer)
            return AcquiredFrame(false);

        // Release
        if (m_currentDrawable)
        {
            m_currentDrawable->release();
            m_currentDrawable = nullptr;
        }

        auto* drawable = m_metalLayer->nextDrawable();
        if (!drawable)
            return AcquiredFrame(false);

        drawable->retain();
        m_currentDrawable = drawable;
        return AcquiredFrame(Texture::Private::build(m_textures[0], drawable));
    }

    bool SwapChain::present()
    {
        return true;
    }

    bool SwapChain::recreateSwapChain(Device& device, Surface& surface)
    {
        return true;
    }

    const RenderTargets& SwapChain::getCurrentRenderTargets() const
    {
        return m_renderTargets[0];
    }

    RenderTargets& SwapChain::getCurrentRenderTargets()
    {
        return m_renderTargets[0];
    }

    const Texture& SwapChain::getCurrentSwapChainTexture() const
    {
        return m_textures[0];
    }

    Texture& SwapChain::getCurrentSwapChainTexture()
    {
        return m_textures[0];
    }

    bool SwapChain::initRenderPassDescriptor()
    {
    }

    bool SwapChain::initRenderTargets()
    {
    }

} // namespace TiRHI::Metal