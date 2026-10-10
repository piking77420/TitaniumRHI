#ifndef TITANIUM_SWAPCHAIN_H
#define TITANIUM_SWAPCHAIN_H

#include <concepts>
#include <format>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI, typename TDevice, typename TRenderPassDescriptor, typename TRenderTarget>
    class BaseSwapChain : public Object<T, TRHI>
    {
    public:
        using _Derived = T;
        using _RHI = TRHI;
        using _Device = TDevice;

        BaseSwapChain() = delete;
        ~BaseSwapChain() = default;
        RHI_MOVE_CONSTRUCT_ONLY(BaseSwapChain)
        BaseSwapChain(TRHI& rhi)
            : Object<T, TRHI>(rhi)
            , m_renderPassDescriptor(rhi)
        {
            static_assert(std::derived_from<T, BaseSwapChain<T, TRHI, TDevice, TRenderPassDescriptor, TRenderTarget>>);
        }

        bool getVsync() const noexcept
        {
            return m_vsync;
        }

        _Derived& setVsync(bool newVsync) noexcept
        {
            m_vsync = newVsync;
            return static_cast<_Derived&>(*this);
        }

        uint32_t getWidth() const noexcept
        {
            return m_width;
        }

        _Derived& setWidth(uint32_t newWidth) noexcept
        {
            m_width = newWidth;
            return static_cast<_Derived&>(*this);
        }

        uint32_t getHeight() const noexcept
        {
            return m_height;
        }

        _Derived& setHeight(uint32_t newHeight)
        {
            m_height = newHeight;
            return static_cast<_Derived&>(*this);
        }

        Extend2DUi getExtend() const noexcept
        {
            return Extend2DUi{.width = getWidth(), .height = getHeight()};
        }

        _Derived& setExtend(Extend2DUi newExtend) noexcept
        {
            m_width = newExtend.width;
            m_height = newExtend.height;
            return static_cast<_Derived&>(*this);
        }

        uint32_t getImageCount() const noexcept
        {
            return m_imageCount;
        }

        _Derived& setImageCount(uint32_t newImageCount) noexcept
        {
            m_imageCount = newImageCount;
            return static_cast<_Derived&>(*this);
        }

        const TRenderPassDescriptor& getRenderPassDescriptor() const noexcept
        {
            return m_renderPassDescriptor;
        }

        bool build() noexcept
        {
            m_renderPassDescriptor.setName(std::format("{} renderPassDescriptor", this->getName()));
            return true;
        }

        bool createRenderPassDescriptor(_Device& device) noexcept
        {
            AttachmentDescriptor attachement{};
            attachement.setFormat(m_format)
                .setSampleCount(SampleCount::Count1)
                .setLoadOp(LoadOp::Clear)
                .setStoreOp(StoreOp::Store)
                .setStencilLoadOp(LoadOp::DontCare)
                .setStencilStoreOp(StoreOp::DontCare)
                .setInitialState(ResourceState::Undefined) // TO DO to put present need an resource tracker
                .setRenderState(ResourceState::RenderTarget)
                .setFinalState(ResourceState::Present);

            const std::array attachments{attachement};

            return m_renderPassDescriptor.setColorAttachements(attachments)
                .setAcceptedPipelineType(PipelineType::Graphics)
                .build(device);
        }

    protected:
        bool m_vsync = false;

        uint32_t m_width = 0;

        uint32_t m_height = 0;

        uint32_t m_imageCount = 1;

        Format m_format = Format::B8G8R8A8_UNorm;

        TRenderPassDescriptor m_renderPassDescriptor;

        std::vector<TRenderTarget> m_renderTargets;
    };

} // TiRHI

#endif // TITANIUM_SWAPCHAIN_H
