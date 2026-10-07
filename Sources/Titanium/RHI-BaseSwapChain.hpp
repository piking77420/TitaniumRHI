#ifndef TITANIUM_SWAPCHAIN_H
#define TITANIUM_SWAPCHAIN_H

#include <concepts>
#include <format>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI, typename TRenderPassDescriptor>
    class BaseSwapChain : public Object<T, TRHI>
    {
    public:
        using _Derived = T;

        BaseSwapChain() = delete;
        ~BaseSwapChain() = default;
        BaseSwapChain(TRHI& rhi)
            : Object<T, TRHI>(rhi)
            , m_renderPassDescriptor(rhi)
        {
            static_assert(std::derived_from<T, BaseSwapChain<T, TRHI, TRenderPassDescriptor>>);
        }

        bool getVsync() const
        {
            return m_vsync;
        }

        _Derived& setVsync(bool newVsync)
        {
            m_vsync = newVsync;
            return static_cast<_Derived&>(*this);
        }

        uint32_t getWidth() const
        {
            return m_width;
        }

        _Derived& setWidth(uint32_t newWidth)
        {
            m_width = newWidth;
            return static_cast<_Derived&>(*this);
        }

        uint32_t getHeight() const
        {
            return m_height;
        }

        _Derived& setHeight(uint32_t newHeight)
        {
            m_height = newHeight;
            return static_cast<_Derived&>(*this);
        }

        uint32_t getImageCount() const
        {
            return m_imageCount;
        }

        _Derived& setImageCount(uint32_t newImageCount)
        {
            m_imageCount = newImageCount;
            return static_cast<_Derived&>(*this);
        }

        const TRenderPassDescriptor& getRenderPassDescriptor() const
        {
            return m_renderPassDescriptor;
        }

        const TRenderPassDescriptor& getRenderPassDescriptor()
        {
            return m_renderPassDescriptor;
        }

        bool build()
        {
            m_renderPassDescriptor.setName(std::format("{} renderPassDescriptor", this->getName()));
            return true;
        }

    protected:
        bool m_vsync = false;

        uint32_t m_width = 0;

        uint32_t m_height = 0;

        uint32_t m_imageCount = 1;

        Format m_format = Format::B8G8R8A8_UNorm;

        TRenderPassDescriptor m_renderPassDescriptor;
    };

} // TiRHI

#endif // TITANIUM_SWAPCHAIN_H
