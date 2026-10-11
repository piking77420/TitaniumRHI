#ifndef TITANIUM_COMMAND_LIST_H
#define TITANIUM_COMMAND_LIST_H

#include <format>
#include <optional>
#include <array>
#include <Titanium/RHI-Object.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    struct ClearValueDepthStencil
    {
        float depth;
        uint32_t stencil;
    };

    struct ClearValueColor
    {
        std::array<float, 4> color;
    };

    struct BeginRenderPass
    {
        std::span<const ClearValueColor> clearColors;
        std::optional<ClearValueDepthStencil> clearDepthStencil;
        Rect2D renderArea;

        BeginRenderPass& setClearColors(std::span<const ClearValueColor> values) noexcept
        {
            clearColors = values;
            return *this;
        }

        BeginRenderPass& setClearDepthStencil(ClearValueDepthStencil value) noexcept
        {
            clearDepthStencil = value;
            return *this;
        }

        BeginRenderPass& setRenderArea(Rect2D area) noexcept
        {
            renderArea = area;
            return *this;
        }
    };

    template<typename T, typename TRHI, typename TRenderTarget, typename TRenderPassDescriptor>
    class BaseCommandList : public Object<T, TRHI>
    {
    public:
        BaseCommandList() = delete;
        ~BaseCommandList() = default;
        RHI_MOVE_ONLY(BaseCommandList)
        BaseCommandList(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseCommandList<T, TRHI, TRenderTarget, TRenderPassDescriptor>>);
        }

    protected:
        struct RecordState
        {
            bool isRecording = false;
            bool isInRenderPass = false;
            const TRenderPassDescriptor* renderPassDescritptor = nullptr;
            TRenderTarget* renderTargets = nullptr;

            void reset()
            {
                isRecording = false;
                isInRenderPass = false;
                renderPassDescritptor = nullptr;
                renderTargets = nullptr;
            }
        } m_recordState;

        bool onBeginRecord()
        {
            if (m_recordState.isRecording)
            {
                RHI_LOG_ERROR(
                    std::format(
                        L"CommandList {}: Try to record commandList while the command list is already in record state",
                        this->getNameW()),
                    RhiApi::Common);
                return false;
            }

            m_recordState.isRecording = true;

            return true;
        }

        void onEndRecord()
        {
            if (!m_recordState.isRecording)
            {

                RHI_LOG_ERROR(
                    std::format(L"CommandList {}: Try to ending record commandList while the command list was "
                                L"not in record state",
                                this->getNameW()),
                    RhiApi::Common);
                return;
            }

            m_recordState.reset();
        }

        bool onBeginRenderPass(const BeginRenderPass& beginRenderPass, TRenderTarget& renderTargets)
        {
            if (!m_recordState.isRecording)
            {
                RHI_LOG_ERROR(std::format(L"CommandList {}: Cannot begin render pass: command list was not recording.",
                                          this->getNameW()),
                              RhiApi::Common);
                return false;
            }

            if (!renderTargets.getRenderPassDescriptor())
            {
                RHI_LOG_ERROR(
                    std::format(
                        L"CommandList {}: Cannot begin render pass: RenderTargets{} 's render passDescritptor is null.",
                        this->getNameW(), renderTargets.getNameW()),
                    RhiApi::Common);
                return false;
            }
            m_recordState.renderTargets = &renderTargets;
            m_recordState.renderPassDescritptor = renderTargets.getRenderPassDescriptor();
            if (!inspectRenderPassRenderTargetState())
            {
                m_recordState.renderTargets = nullptr;
                m_recordState.renderPassDescritptor = nullptr;
                return false;
            }
            m_recordState.isInRenderPass = true;
            return true;
        }

        void onEndRenderPass()
        {
            if (!m_recordState.isRecording)
            {
                RHI_LOG_ERROR(std::format(L"CommandList {}: Cannot end render pass: command list was not recording.",
                                          this->getNameW()),
                              RhiApi::Common);
                return;
            }

            if (!m_recordState.isInRenderPass)
            {
                RHI_LOG_ERROR(std::format(L"CommandList {}: Cannot end render pass: no render pass is currently active "
                                          L"on this command list.",
                                          this->getNameW()),
                              RhiApi::Common);
                return;
            }

            m_recordState.isInRenderPass = false;
            m_recordState.renderPassDescritptor = nullptr;
            m_recordState.renderTargets = nullptr;
        }

        bool inspectRenderPassRenderTargetState()
        {
            if (!m_recordState.renderTargets || !m_recordState.renderPassDescritptor)
                return false;

            const std::span colorAttachments = m_recordState.renderPassDescritptor->getColorAttachements();
            const std::optional depthAttachment = m_recordState.renderPassDescritptor->getDepthAttachement();

            const std::span colorTextures = m_recordState.renderTargets->getColorAttachements();
            const std::optional depthTextures = m_recordState.renderTargets->getDepthAttachement();
            const size_t attachmentCount = colorAttachments.size() + (depthAttachment.has_value() ? 1 : 0);

            if (colorAttachments.size() != colorTextures.size())
            {
                RHI_LOG_ERROR(
                    std::format(L"CommandList '{}': Color attachment count mismatch. "
                                L"RenderPassDescriptor expects {} attachments, but RenderTargets provides {}.",
                                this->getNameW(), colorAttachments.size(), colorTextures.size()),
                    RhiApi::Common);

                return false;
            }
            if (depthAttachment.has_value() != depthTextures.has_value())
            {
                RHI_LOG_ERROR(std::format(L"CommandList '{}': Depth attachment mismatch. "
                                          L"RenderPassDescriptor requires depth: {}, RenderTargets provides depth: {}.",
                                          this->getNameW(), depthAttachment.has_value(), depthTextures.has_value()),
                              RhiApi::Common);

                return false;
            }

            for (size_t i = 0; i < attachmentCount; i++)
            {
                const bool isDepth = i == (attachmentCount - 1) && depthAttachment;
                typename TRenderTarget::_Texture& texture = isDepth ? *(*depthTextures) : *colorTextures[i];
                const AttachmentDescriptor& descriptor = isDepth ? *depthAttachment : colorAttachments[i];

                if (texture.getState() != descriptor.initialState)
                {
                    RHI_LOG_WARNING(
                        std::format(L"CommandList {}: Texture {} is in state {} but render pass expected {}",
                                    this->getNameW(), texture.getNameW(), toWString(texture.getState()),
                                    toWString(descriptor.initialState)),
                        RhiApi::None);
                }
            }
        }

        void setRenderPassRenderTargetState(bool isSettingRenderState)
        {
            if (!m_recordState.renderTargets || !m_recordState.renderPassDescritptor)
                return;

            const std::span colorAttachments = m_recordState.renderPassDescritptor->getColorAttachements();
            const std::optional depthAttachment = m_recordState.renderPassDescritptor->getDepthAttachement();
            const size_t attachmentCount = colorAttachments.size() + (depthAttachment.has_value() ? 1 : 0);

            const std::span colorTextures = m_recordState.renderTargets->getColorAttachements();
            const std::optional depthTextures = m_recordState.renderTargets->getDepthAttachement();

            for (size_t i = 0; i < attachmentCount; i++)
            {
                const bool isDepth = i == (attachmentCount - 1) && depthAttachment;
                typename TRenderTarget::_Texture& texture = isDepth ? *(*depthTextures) : *colorTextures[i];
                const AttachmentDescriptor& descriptor = isDepth ? *depthAttachment : colorAttachments[i];
                texture.setState(isSettingRenderState ? descriptor.renderState : descriptor.finalState);
            }
        }
    };

} // namespace TiRHI

#endif // TITANIUM_COMMAND_LIST_H
