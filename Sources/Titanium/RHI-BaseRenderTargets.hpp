#ifndef TITANIUM_BASE_RENDER_TARGETS_H
#define TITANIUM_BASE_RENDER_TARGETS_H

#include <format>
#include <optional>
#include <span>
#include <vector>

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Object.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI, typename TRenderPassDescriptor>
    class BaseRenderTargets : public Object<T, TRHI>
    {
    public:
        BaseRenderTargets() = delete;
        ~BaseRenderTargets() = default;
        RHI_MOVE_ONLY(BaseRenderTargets);
        BaseRenderTargets(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseRenderTargets<T, TRHI, TRenderPassDescriptor>>);
        }

        uint32_t getWidth() const
        {
            return m_width;
        }

        T& setWidth(uint32_t newWidth)
        {
            m_width = newWidth;
            return reinterpret_cast<T&>(*this);
        }

        uint32_t getHeight() const
        {
            return m_height;
        }

        T& setHeight(uint32_t newHeight)
        {
            m_height = newHeight;
            return reinterpret_cast<T&>(*this);
        }

        const TRenderPassDescriptor* getRenderPassDescriptor() const
        {
            return m_renderPassDescriptor;
        }

        T& setRenderPassDescriptor(const TRenderPassDescriptor* newRenderPassDescriptor)
        {
            m_renderPassDescriptor = newRenderPassDescriptor;
            return reinterpret_cast<T&>(*this);
        }

        bool build();

    private:
        uint32_t m_width = 0;

        uint32_t m_height = 0;

        const TRenderPassDescriptor* m_renderPassDescriptor{nullptr};
    };

    template<typename T, typename TRHI, typename TRenderPassDescriptor>
    inline bool BaseRenderTargets<T, TRHI, TRenderPassDescriptor>::build()
    {
        if (!m_renderPassDescriptor)
        {
            RHI_LOG_ERROR(std::format(L"missing Render target {}", this->getNameW()),
                          RhiApi::None); // TODO allow rhiapi or expose api in template

            return false;
        }

        return true;
    }

} // TiRHI

#endif // TITANIUM_BASE_RENDER_TARGETS_H
