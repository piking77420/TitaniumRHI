#ifndef TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H

#include <optional>
#include <span>
#include <vector>

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Object.hpp>

namespace TiRHI
{
    struct AttachmentDescriptor
    {
        Format format = {};
        SampleCount sampleCount = SampleCount::Count1;

        LoadOp loadOp = LoadOp::DontCare;
        StoreOp storeOp = StoreOp::DontCare;

        LoadOp stencilLoadOp = LoadOp::DontCare;
        StoreOp stencilStoreOp = StoreOp::DontCare;

        ResourceState renderState = ResourceState::Undefined;
        ResourceState finalState = ResourceState::Undefined;

        constexpr AttachmentDescriptor() = default;

        constexpr AttachmentDescriptor& setFormat(Format value) noexcept
        {
            format = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setSampleCount(SampleCount value) noexcept
        {
            sampleCount = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setLoadOp(LoadOp value) noexcept
        {
            loadOp = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setStoreOp(StoreOp value) noexcept
        {
            storeOp = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setStencilLoadOp(LoadOp value) noexcept
        {
            stencilLoadOp = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setStencilStoreOp(StoreOp value) noexcept
        {
            stencilStoreOp = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setRenderState(ResourceState value) noexcept
        {
            renderState = value;
            return *this;
        }

        constexpr AttachmentDescriptor& setFinalState(ResourceState value) noexcept
        {
            finalState = value;
            return *this;
        }
    };

    template<typename T, typename TRHI>
    class BaseRenderPassDescriptor : public Object<T, TRHI>
    {
    public:
        BaseRenderPassDescriptor() = delete;
        ~BaseRenderPassDescriptor() = default;

        BaseRenderPassDescriptor(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseRenderPassDescriptor<T, TRHI>>);
        }

        T& setColorAttachements(const std::span<const AttachmentDescriptor> attachementDescriptor)
        {
            m_colorAttachements.insert(m_colorAttachements.end(), attachementDescriptor.begin(),
                                       attachementDescriptor.end());
            return reinterpret_cast<T&>(*this);
        }

        std::span<const AttachmentDescriptor> getColorAttachements() const
        {
            return std::span(m_colorAttachements);
        }

        const std::optional<AttachmentDescriptor>& getDepthAttachement() const
        {
            return m_depthAttachement;
        }

        T& setAcceptedPipelineType(PipelineType newAcceptedPipelineType)
        {
            m_acceptedPipelineType = newAcceptedPipelineType;
            return reinterpret_cast<T&>(*this);
        }

        PipelineType getAcceptedPipelineType() const
        {
            return m_acceptedPipelineType;
        }

    private:
        std::vector<AttachmentDescriptor> m_colorAttachements;

        std::optional<AttachmentDescriptor> m_depthAttachement;

        PipelineType m_acceptedPipelineType;
    };

}

#endif // TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H
