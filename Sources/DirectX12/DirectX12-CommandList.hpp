#ifndef TITANIUM_DIRECTX12_COMMAND_LIST_H
#define TITANIUM_DIRECTX12_COMMAND_LIST_H

#include <vector>
#include <optional>
#include <variant>
#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseCommandList.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <d3d12.h>
#include <Private/StaginBuffer.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class RenderTargets;
    class Texture;
    class Buffer;
    class Sampler;
    class RenderPassDescriptor;
    class GraphicPipeline;
    class DescriptorSet;

    using TrackedResource = std::variant<Texture*, Sampler*, std::pair<Buffer*, uint32_t>,
                                         GraphicPipeline*, Private::StagingBuffer>;

    class CommandList : public BaseCommandList<CommandList, Device, RenderTargets, RenderPassDescriptor,
                                               GraphicPipeline, DescriptorSet>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        CommandList() = delete;
        ~CommandList() = default;
        RHI_MOVE_ONLY(CommandList);
        CommandList(Device& device);

        bool build();

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        bool beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets);

        void endRenderPass();

        void setViewPort(const Viewport& viewPort);

        void setScissors(const Rect2D& rect2d);

        void transitionResource(Texture& texture, ResourceState state);

        void transitionResource(Buffer& buffer, ResourceState state);

        void bindPipeline(_GraphicPipeline& graphicPipeline);

        void bindDescriptorSet(const DescriptorSet& descriptorSet, uint32_t set);

        void bindVertexBuffer(uint32_t bindingIndex, uint64_t offset, Buffer& buffer);

        void bindIndexBuffer(Buffer& buffer, uint64_t offset, IndexType format);

        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                         uint32_t firstInstance);

        void pushConstant(void* data, size_t size);

        void upload2d(Texture& texture, std::span<const std::byte> data);

        void uploadBuffer(Buffer& buffer, uint64_t offset, std::span<const std::byte> data);

        ID3D12GraphicsCommandList1* getCommandListNative()
        {
            return m_commandList.Get();
        }

        void markResourceFrame(uint64_t submisionValue);

    private:
        struct SetGraphicsRootDescriptorTable
        {
            uint32_t rootParamIndex;
            D3D12_GPU_DESCRIPTOR_HANDLE handle;
        };

        struct RecordState
        {
            ID3D12RootSignature* currentRootSignature = nullptr;
            void reset()
            {
                currentRootSignature = nullptr;
            }
        } m_recordStateD3D12;

        std::vector<MComPtr<ID3D12CommandAllocator>> m_allocators;
        MComPtr<ID3D12GraphicsCommandList4> m_commandList;
        std::vector<D3D12_RESOURCE_BARRIER> m_barriers;
        uint64_t m_lastSubmissionValue = 0;
        std::vector<TrackedResource> m_trackedResource;

        void resetOnRecord();

        void bindDescriptorHeaps();

        void bindRootSignature(ID3D12RootSignature* rootSignature);

        bool renderTargetTransition(RenderTargets& renderTargets, bool IsOutTransition);

        void trackResource(Buffer& buffer);

        void trackResource(Sampler& sampler);

        void trackResource(Texture& texture);

        void trackResource(GraphicPipeline& graphicPipeline);

        void trackResource(Private::StagingBuffer&& stagingBuffer);

        void trackResource(Private::StagingBuffer& stagingBuffer) = delete;

        void trackResource(const Private::StagingBuffer& stagingBuffer) = delete;
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_COMMAND_LIST_H
