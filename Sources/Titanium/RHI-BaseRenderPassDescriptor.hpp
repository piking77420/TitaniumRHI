#ifndef TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H
#define TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H

#include <optional>
#include <span>
#include <vector>

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Object.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseRenderPassDescriptor : public Object<T, TRHI>
    {
    public:
        BaseRenderPassDescriptor() = delete;
        ~BaseRenderPassDescriptor() = default;
        RHI_MOVE_CONSTRUCT_ONLY(BaseRenderPassDescriptor)
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

        size_t getAttachementCount() const
        {
            return m_colorAttachements.size() + (m_depthAttachement ? 1 : 0);
        }

    private:
        std::vector<AttachmentDescriptor> m_colorAttachements;

        std::optional<AttachmentDescriptor> m_depthAttachement;

        PipelineType m_acceptedPipelineType;
    };

}

#endif // TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H
