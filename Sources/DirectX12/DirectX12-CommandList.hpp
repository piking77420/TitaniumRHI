#ifndef TITANIUM_DIRECTX12_COMMAND_LIST_H
#define TITANIUM_DIRECTX12_COMMAND_LIST_H

#include <optional>
#include <vector>
#include <Titanium/RHI-BaseCommandList.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <d3d12.h>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class RenderTargets;

    class CommandList : public BaseCommandList<CommandList, RHI, RenderTargets>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        bool beginRenderPass(const BeginRenderPass& beginRenderPass, const RenderTargets& renderTargets);

        void endRenderPass();

        void setViewPort(const Viewport& viewPort);

        void setScissors(const Rect2D& rect2d);

        ID3D12GraphicsCommandList1* getCommandListNative()
        {
            return m_commandList.Get();
        }

    private:
        std::vector<MComPtr<ID3D12CommandAllocator>> m_allocators;
        MComPtr<ID3D12GraphicsCommandList4> m_commandList;
        std::vector<D3D12_RESOURCE_BARRIER> m_barriers;
        const RenderTargets* m_currentRenderTargets;

        bool renderTargetTransitionIn(const RenderTargets& renderTargets);
        bool renderTargetTransitionOut(const RenderTargets& renderTargets);
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_COMMAND_LIST_H
