#ifndef TITANIUM_BASE_RENDER_TARGETS_H
#define TITANIUM_BASE_RENDER_TARGETS_H

#include <optional>
#include <span>
#include <vector>

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Object.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseRenderTargets : public Object<T, TRHI>
    {
    public:
        BaseRenderTargets() = delete;
        ~BaseRenderTargets() = default;

        BaseRenderTargets(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseRenderTargets<T, TRHI>>);
        }

        uint32_t getWidth() const
        {
            return m_width;
        }
        uint32_t getHeight() const
        {
            return m_height;
        }

    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;
    };

} // TiRHI

#endif // TITANIUM_BASE_RENDER_TARGETS_H
