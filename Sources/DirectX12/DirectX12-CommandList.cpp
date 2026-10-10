#include <DirectX12-CommandList.hpp>

#include <format>

#if defined(TITANIUM_USE_PIX)
#include <pix3.h>
#endif // defined(TITANIUM_USE_PIX)

#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RenderTargets.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

static uint8_t toPixColor(float value)
{
    return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

namespace TiRHI::DirectX12
{
    CommandList::CommandList(RHI& rhi)
        : BaseCommandList(rhi)
    {
    }

    bool CommandList::build(Device& device)
    {
        m_allocators.resize(getRHI().getFrameInFlight());

        for (uint32_t i = 0; i < m_allocators.size(); ++i)
        {
            auto& cmdAlloc = m_allocators[i];

            const HRESULT hrCmdAllocCreated = device.getNativeDevice()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
            if (FAILED(hrCmdAllocCreated))
            {
                RHI_LOG_ERROR(
                    std::format(L"Create Command Allocator {} failed!\n Error Code: {}", i, hrCmdAllocCreated),
                    RhiApi::DirectX12);
                return false;
            }
            else
            {
                const std::wstring name =
                    std::format(L"CommandAlloc {} [{}]", (getName().empty() ? L"" : getNameW()), i);
                cmdAlloc->SetName(name.c_str());

                RHI_LOG_INFO(std::format(L"Create Command Allocator {} success! name : {}", hrCmdAllocCreated, name),
                             RhiApi::DirectX12);
            }
        }

        const HRESULT hrCmdListCreated = device.getNativeDevice()->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));
        if (FAILED(hrCmdListCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Command List failed! {} failed! ", hrCmdListCreated), RhiApi::DirectX12);
            return false;
        }
        else
        {
            const LPCWSTR name = getName().empty() ? L"" : getNameW().c_str();
            m_commandList->SetName(name);
            RHI_LOG_INFO(std::format(L"Create Command List success.\nName : {}, Handle : {}", name,
                                     reinterpret_cast<void*>(m_commandList.Get())),
                         RhiApi::DirectX12);
        }

        m_commandList->Close();

        return true;
    }

    bool CommandList::beginRecord()
    {
        if (!onBeginRecord())
            return false;

        const size_t currentFrame = getRHI().getCurrentFrame();

        const HRESULT hrAllocator = m_allocators[currentFrame]->Reset();

        if (FAILED(hrAllocator))
            return false;

        const HRESULT hrCommandList = m_commandList->Reset(m_allocators[currentFrame].Get(), nullptr);

        if (FAILED(hrCommandList))
            return false;

        return true;
    }

    bool CommandList::endRecord()
    {
        const HRESULT hr = m_commandList->Close();

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"Close Command List failed: {}", hr), RhiApi::DirectX12);

            return false;
        }

        onEndRecord();

        return true;
    }

    void CommandList::beginDebugLabel(std::string_view name,
                                      [[maybe_unused]] std::optional<std::span<const float, 4>> color)
    {
        if (!getRHI().getUseDebugLabel())
            return;

#if defined(TITANIUM_USE_PIX)
        UINT64 colorPix = PIX_COLOR_DEFAULT;
        if (color)
        {
            const std::span<const float, 4> colors = *color;
            const auto toByte = [](float v) -> uint8_t
            { return static_cast<uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f); };

            colorPix = PIX_COLOR(toByte(colors[0]), toByte(colors[1]), toByte(colors[2]));
        }
        PIXBeginEvent(m_commandList.Get(), colorPix, name.data());
#endif
    }

    void CommandList::endDebugLabel()
    {
        if (!getRHI().getUseDebugLabel())
            return;

#if defined(TITANIUM_USE_PIX)
        PIXEndEvent(m_commandList.Get());
#endif // defined(TITANIUM_USE_PIX)
    }

    bool CommandList::beginRenderPass(const BeginRenderPass& beginRenderPass, const RenderTargets& renderTargets)
    {
        const auto* pass = renderTargets.getRenderPassDescriptor();
        const auto colorAtt = pass->getColorAttachements();
        const auto depthAtt = pass->getDepthAttachement();
        const auto d3d12Att = renderTargets.getAttachements();

        const size_t colorCount = colorAtt.size();

        // Validate before modifying command-list state.
        if (colorCount > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
            return false;

        if (d3d12Att.size() < colorCount + (depthAtt.has_value() ? 1 : 0))
            return false;

        // Assumes every color attachment uses LoadOp::Clear.
        if (beginRenderPass.clearColors.size() < colorCount)
            return false;

        if (!onBeginRenderPass(beginRenderPass, renderTargets))
            return false;

        if (!renderTargetTransitionIn(renderTargets))
            return false;

        std::array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> attachments{};

        for (size_t i = 0; i < colorCount; ++i)
        {
            auto& desc = attachments[i];
            const auto& colorAttachment = colorAtt[i];

            desc.cpuDescriptor = d3d12Att[i].handle;

            desc.BeginningAccess.Type = Private::toDirectX12(colorAtt[i].loadOp);

            desc.BeginningAccess.Clear.ClearValue.Format = Private::toDirectX12(colorAttachment.format);

            std::copy_n(beginRenderPass.clearColors[i].color.begin(), beginRenderPass.clearColors[i].color.size(),
                        desc.BeginningAccess.Clear.ClearValue.Color);

            desc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthDesc{};
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depthDescPtr = nullptr;

        if (depthAtt)
        {
            depthDesc.cpuDescriptor = d3d12Att[colorCount].handle;
            depthDesc.DepthBeginningAccess.Type = Private::toDirectX12(depthAtt->loadOp);
            depthDesc.DepthEndingAccess.Type = Private::toDirectX12(depthAtt->storeOp);
            depthDesc.StencilBeginningAccess.Type = Private::toDirectX12(depthAtt->stencilLoadOp);
            depthDesc.StencilEndingAccess.Type = Private::toDirectX12(depthAtt->stencilStoreOp);

            if (beginRenderPass.clearDepthStencil)
            {
                depthDesc.DepthBeginningAccess.Clear.ClearValue.Format = Private::toDirectX12(depthAtt->format);

                depthDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth =
                    beginRenderPass.clearDepthStencil->depth;
            }
            else
            {
                // A CLEAR operation requires a clear value.
                return false;
            }

            depthDescPtr = &depthDesc;
        }

        m_commandList->BeginRenderPass(static_cast<UINT>(colorCount), colorCount > 0 ? attachments.data() : nullptr,
                                       depthDescPtr, D3D12_RENDER_PASS_FLAG_NONE);

        m_currentRenderTargets = &renderTargets;

        return true;
    }

    void CommandList::endRenderPass()
    {
        onEndRenderPass();
        m_commandList->EndRenderPass();

        if (m_currentRenderTargets)
        {
            renderTargetTransitionOut(*m_currentRenderTargets);
            m_currentRenderTargets = nullptr;
        }
    }

    void CommandList::setViewPort(const Viewport& viewPort)
    {
        D3D12_VIEWPORT d3d12Viewport;
        d3d12Viewport.TopLeftX = viewPort.position.x;
        d3d12Viewport.TopLeftY = viewPort.position.y;
        d3d12Viewport.Width = viewPort.extend.width;
        d3d12Viewport.Height = viewPort.extend.height;
        d3d12Viewport.MinDepth = viewPort.minDepth;
        d3d12Viewport.MaxDepth = viewPort.maxDepth;
        m_commandList->RSSetViewports(1, &d3d12Viewport);
    }

    void CommandList::setScissors(const Rect2D& rect2d)
    {
        D3D12_RECT d3d12Scissors{};
        d3d12Scissors.left = rect2d.offset.x;
        d3d12Scissors.top = rect2d.offset.y;
        d3d12Scissors.right = rect2d.offset.x + static_cast<LONG>(rect2d.extend.width);
        d3d12Scissors.bottom = rect2d.offset.y + static_cast<LONG>(rect2d.extend.height);

        m_commandList->RSSetScissorRects(1, &d3d12Scissors);
    }

    bool CommandList::renderTargetTransitionIn(const RenderTargets& renderTargets)
    {
        // TODO refactor this

        const RenderPassDescriptor* renderPassDescriptor = renderTargets.getRenderPassDescriptor();
        if (!renderPassDescriptor)
            return false;

        std::span attachements = renderTargets.getAttachements();

        if (attachements.size() != renderPassDescriptor->getAttachementCount())
        {
            RHI_LOG_ERROR(
                std::format(L"CommandList {}: renderTargetTransition but d3d12 renderTargets size is not equal to ",
                            this->getNameW()),
                RhiApi::DirectX12);

            return false;
        }

        m_barriers.clear();
        m_barriers.reserve(attachements.size());
        for (size_t i = 0; i < attachements.size(); i++)
        {
            const auto& att = attachements[i];
            if (i == attachements.size() - 1 && renderPassDescriptor->getDepthAttachement())
            {
                const AttachmentDescriptor& desc = *renderPassDescriptor->getDepthAttachement();

                D3D12_RESOURCE_BARRIER& barrier = m_barriers.emplace_back();
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = att.image;
                barrier.Transition.StateBefore = Private::toDirectX12(desc.initialState);
                barrier.Transition.StateAfter = Private::toDirectX12(desc.renderState);
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            }
            else
            {
                const AttachmentDescriptor& desc = renderPassDescriptor->getColorAttachements()[i];

                D3D12_RESOURCE_BARRIER& barrier = m_barriers.emplace_back();
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = att.image;
                barrier.Transition.StateBefore = Private::toDirectX12(desc.initialState);
                barrier.Transition.StateAfter = Private::toDirectX12(desc.renderState);
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            }
        }

        m_commandList->ResourceBarrier(m_barriers.size(), m_barriers.data());

        return true;
    }

    bool CommandList::renderTargetTransitionOut(const RenderTargets& renderTargets)
    {
        const RenderPassDescriptor* renderPassDescriptor = renderTargets.getRenderPassDescriptor();
        if (!renderPassDescriptor)
            return false;

        std::span attachements = renderTargets.getAttachements();

        if (attachements.size() != renderPassDescriptor->getAttachementCount())
        {
            RHI_LOG_ERROR(
                std::format(L"CommandList {}: renderTargetTransition but d3d12 renderTargets size is not equal to ",
                            this->getNameW()),
                RhiApi::DirectX12);

            return false;
        }

        m_barriers.clear();
        m_barriers.reserve(attachements.size());
        for (size_t i = 0; i < attachements.size(); i++)
        {
            const auto& att = attachements[i];
            if (i == attachements.size() - 1 && renderPassDescriptor->getDepthAttachement())
            {
                const AttachmentDescriptor& desc = *renderPassDescriptor->getDepthAttachement();

                D3D12_RESOURCE_BARRIER& barrier = m_barriers.emplace_back();
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = att.image;
                barrier.Transition.StateBefore = Private::toDirectX12(desc.renderState);
                barrier.Transition.StateAfter = Private::toDirectX12(desc.finalState);
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            }
            else
            {
                const AttachmentDescriptor& desc = renderPassDescriptor->getColorAttachements()[i];

                D3D12_RESOURCE_BARRIER& barrier = m_barriers.emplace_back();
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = att.image;
                barrier.Transition.StateBefore = Private::toDirectX12(desc.renderState);
                barrier.Transition.StateAfter = Private::toDirectX12(desc.finalState);
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            }
        }

        m_commandList->ResourceBarrier(m_barriers.size(), m_barriers.data());

        return true;
    }

}
// namespace TiRHI::DirectX12
