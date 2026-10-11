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
    template<typename T, typename TRHI, typename TRenderPassDescriptor, typename TTexture>
    class BaseRenderTargets : public Object<T, TRHI>
    {
    public:
        using _RHI = TRHI;
        using _RenderPassDescriptor = TRenderPassDescriptor;
        using _Texture = TTexture;

        BaseRenderTargets() = delete;
        ~BaseRenderTargets() = default;
        RHI_MOVE_ONLY(BaseRenderTargets);
        BaseRenderTargets(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseRenderTargets<T, TRHI, TRenderPassDescriptor, _Texture>>);
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

        bool build()
        {
            if (!m_renderPassDescriptor)
            {
                RHI_LOG_ERROR(std::format(L"missing Render target {}", this->getNameW()),
                              RhiApi::None); // TODO allow rhiapi or expose api in template

                return false;
            }

            return true;
        }

        const std::span<const _Texture* const>& getColorAttachements() const
        {
            return m_colorAttachements;
        }

        const std::span<_Texture*>& getColorAttachements()
        {
            return m_colorAttachements;
        }

        T& bindColorAttachement(_Texture& textureToBind, size_t bindingSlot)
        {
            if (m_colorAttachements.size() <= bindingSlot)
            {
                m_colorAttachements.resize(bindingSlot + 1);
            }

            m_colorAttachements[bindingSlot] = &textureToBind;
            return reinterpret_cast<T&>(*this);
        }

        const std::optional<const _Texture*>& getDepthAttachement() const
        {
            return m_depthAttachement;
        }

        const std::optional<_Texture*>& getDepthAttachement()
        {
            return m_depthAttachement;
        }

        T& bindDepthAttachement(_Texture& textureToBind, size_t bindingSlot)
        {
            m_depthAttachement = textureToBind;
            return reinterpret_cast<T&>(*this);
        }

        void clear()
        {
            m_colorAttachements.clear();
            m_depthAttachement.reset();
        }

    private:
        uint32_t m_width = 0;

        uint32_t m_height = 0;

        const TRenderPassDescriptor* m_renderPassDescriptor{nullptr};

        std::vector<_Texture*> m_colorAttachements;

        std::optional<_Texture*> m_depthAttachement;
    };

} // TiRHI

#endif // TITANIUM_BASE_RENDER_TARGETS_H
