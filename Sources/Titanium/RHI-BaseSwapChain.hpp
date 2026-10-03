#ifndef TITANIUM_SWAPCHAIN_H
#define TITANIUM_SWAPCHAIN_H

#include <concepts>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseSwapChain : public Object<T, TRHI>
    {
    public:
        using _Derived = T;

        BaseSwapChain() = delete;
        ~BaseSwapChain() = default;
        BaseSwapChain(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
        }

        bool getVsync() const
        {
            static_assert(std::derived_from<T, BaseSwapChain<T>>);

            return m_vsync;
        }

        _Derived& setVsync(bool newVsync)
        {
            m_vsync = newVsync;
            return static_cast<_Derived&>(*this);
        }

        uint32_t width() const
        {
            return m_width;
        }

        _Derived& setWidth(uint32_t newWidth)
        {
            m_width = newWidth;
            return static_cast<_Derived&>(*this);
        }

        uint32_t height() const
        {
            return m_height;
        }

        _Derived& setHeight(uint32_t newHeight)
        {
            m_height = newHeight;
            return static_cast<_Derived&>(*this);
        }

    protected:
        bool m_vsync = false;

        uint32_t m_width = 0;

        uint32_t m_height = 0;

        static constexpr size_t BufferCount = 3;
    };

} // TiRHI

#endif // TITANIUM_SWAPCHAIN_H
