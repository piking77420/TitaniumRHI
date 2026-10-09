#ifndef TITANIUM_COMMAND_LIST_H
#define TITANIUM_COMMAND_LIST_H

#include <format>
#include <optional>
#include <array>
#include <variant>
#include <Titanium/RHI-Object.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    // helper for std::visit
    template<class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    struct ClearValueDepthStencil
    {
        float depth;
        uint32_t stencil;
    };

    struct ClearValueColor
    {
        std::array<float, 4> color;
    };

    using ClearValue = std::variant<ClearValueColor, ClearValueDepthStencil>;

    struct BeginRenderPass
    {
        std::span<ClearValue> clearValues;
        Rect2D renderArea;

        BeginRenderPass& setClearColor(const std::span<ClearValue>& newClearValue) noexcept
        {
            clearValues = newClearValue;
            return *this;
        }

        BeginRenderPass& setRenderArea(const Rect2D newRenderArea) noexcept
        {
            renderArea = newRenderArea;
            return *this;
        }
    };

    template<typename T, typename TRHI, typename TRenderTarget>
    class BaseCommandList : public Object<T, TRHI>
    {
    public:
        BaseCommandList() = delete;
        ~BaseCommandList() = default;
        RHI_MOVE_CONSTRUCT_ONLY(BaseCommandList)
        BaseCommandList(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseCommandList<T, TRHI, TRenderTarget>>);
        }

    protected:
        struct RecordState
        {
            bool isRecording = false;
            bool isInRenderPass = false;

            void reset()
            {
                isRecording = false;
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

        bool onBeginRenderPass(const BeginRenderPass& beginRenderPass, const TRenderTarget& renderTargets)
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
        }
    };

} // namespace TiRHI

#endif // TITANIUM_COMMAND_LIST_H
