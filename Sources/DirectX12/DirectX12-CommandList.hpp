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
    class Texture;
    class RenderPassDescriptor;
    class GraphicPipeline;

    class CommandList : public BaseCommandList<CommandList, RHI, RenderTargets, RenderPassDescriptor, GraphicPipeline>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        RHI_MOVE_ONLY(CommandList)
        CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        bool beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets);

        void endRenderPass();

        void setViewPort(const Viewport& viewPort);

        void setScissors(const Rect2D& rect2d);

        void transitionResource(Texture& texture, ResourceState state);

        void bindPipeline(const _GraphicPipeline& graphicPipeline);

        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                         uint32_t firstInstance);

        ID3D12GraphicsCommandList1* getCommandListNative()
        {
            return m_commandList.Get();
        }

    private:
        Device* m_device{nullptr};
        std::vector<MComPtr<ID3D12CommandAllocator>> m_allocators;
        MComPtr<ID3D12GraphicsCommandList4> m_commandList;
        std::vector<D3D12_RESOURCE_BARRIER> m_barriers;

        bool renderTargetTransition(RenderTargets& renderTargets, bool IsOutTransition);
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_COMMAND_LIST_H
