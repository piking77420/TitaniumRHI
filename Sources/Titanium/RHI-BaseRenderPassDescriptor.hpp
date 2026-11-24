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
        BaseRenderPassDescriptor(const BaseRenderPassDescriptor&) = delete;
        BaseRenderPassDescriptor& operator=(const BaseRenderPassDescriptor&) = delete;
        BaseRenderPassDescriptor(BaseRenderPassDescriptor&&) noexcept = default;
        BaseRenderPassDescriptor& operator=(BaseRenderPassDescriptor&&) noexcept = default;
        BaseRenderPassDescriptor(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseRenderPassDescriptor<T, TRHI>>);
        }

        T& setColorAttachments(const std::span<const AttachmentDescriptor> attachmentDescriptors)
        {
            m_colorAttachements.clear();
            m_colorAttachements.reserve(attachmentDescriptors.size());
            for (const auto& att : attachmentDescriptors)
            {
                m_colorAttachements.emplace_back(att);
            }
            return static_cast<T&>(*this);
        }

        std::span<const AttachmentDescriptor> getColorAttachements() const
        {
            return std::span(m_colorAttachements);
        }

        T& setDepthAttachement(AttachmentDescriptor attachmentDescriptor)
        {
            m_depthAttachement.reset();
            m_depthAttachement.emplace(attachmentDescriptor);
            return static_cast<T&>(*this);
        }

        const std::optional<AttachmentDescriptor>& getDepthAttachement() const
        {
            return m_depthAttachement;
        }

        T& setAcceptedPipelineType(PipelineType newAcceptedPipelineType)
        {
            m_acceptedPipelineType = newAcceptedPipelineType;
            return static_cast<T&>(*this);
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

        PipelineType m_acceptedPipelineType = PipelineType::Graphics;
    };

}

#endif // TITANIUM_BASE_RENDER_PASS_DESCRIPTOR_H
