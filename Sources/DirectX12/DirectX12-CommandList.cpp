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
#include <DirectX12/DirectX12-GraphicPipeline.hpp>
#include <DirectX12/DirectX12-DescriptorSet.hpp>
#include <DirectX12/DirectX12-Sampler.hpp>
#include <DirectX12/DirectX12-Buffer.hpp>
#include <DirectX12/DirectX12-Texture.hpp>
#include <DirectX12/Private/StaginBuffer.hpp>

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
        m_device = &device;
        m_allocators.resize(getRHI().getFrameInFlight());

        for (uint32_t i = 0; i < m_allocators.size(); ++i)
        {
            auto& cmdAlloc = m_allocators[i];

            const HRESULT hrCmdAllocCreated = device.getNativeDevice()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
            if (FAILED(hrCmdAllocCreated))
            {
                const std::wstring name =
                    std::format(L"CommandAlloc {} [{}]", (getName().empty() ? L"" : getNameW()), i);
                cmdAlloc->SetName(name.c_str());

                RHI_LOG_ERROR(
                    std::format(L"Create Command Allocator {} failed!\n Error Code: {}", name, hrCmdAllocCreated),
                    RhiApi::DirectX12);
                return false;
            }
            else
            {
                const std::wstring name =
                    std::format(L"CommandAlloc {} [{}]", (getName().empty() ? L"" : getNameW()), i);
                cmdAlloc->SetName(name.c_str());

                RHI_LOG_INFO(std::format(L"Create Command Allocator {} success! name : {}", i, name),
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

        if (!m_device)
            return false;

        const size_t currentFrame = getRHI().getCurrentFrame();

        const HRESULT hrAllocator = m_allocators[currentFrame]->Reset();

        if (FAILED(hrAllocator))
            return false;

        const HRESULT hrCommandList = m_commandList->Reset(m_allocators[currentFrame].Get(), nullptr);

        if (FAILED(hrCommandList))
            return false;

        resetOnRecord();
        bindDescriptorHeaps();
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

    bool CommandList::beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets)
    {
        if (!m_device)
            return false;

        if (!onBeginRenderPass(beginRenderPass, renderTargets))
            return false;

        const auto colorAtt = m_recordState.renderPassDescritptor->getColorAttachements();
        const auto depthAtt = m_recordState.renderPassDescritptor->getDepthAttachement();
        const auto renderTargetAttachement = renderTargets.getColorAttachements();
        const auto depthTargetAttachement = renderTargets.getDepthAttachement();

        const size_t colorCount = colorAtt.size();

        // Validate before modifying command-list state.
        if (colorCount > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
            return false;

        if (!renderTargetTransition(renderTargets, false))
            return false;

        setRenderPassRenderTargetState(true);

        std::array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> desc{};

        auto fillRenderTargetDesc = [device = m_device](D3D12_RENDER_PASS_RENDER_TARGET_DESC& desc,
                                                        const Texture& texture,
                                                        const AttachmentDescriptor& attachmentDescriptor)
        {
            const std::optional<TiRHI::DirectX12::Private::DescriptorHandle>& handle =
                Texture::Private::getRenderTargetViewHandle(texture);
            if (!handle)
            {
                RHI_LOG_ERROR(
                    std::format(L"Failed to get D3D12_CPU_DESCRIPTOR_HANDLE fo the texture {}", texture.getNameW()),
                    RhiApi::DirectX12);
                return;
            }

            desc.cpuDescriptor = handle->cpuHandle;
            desc.BeginningAccess.Type = Private::toDirectX12(attachmentDescriptor.loadOp);

            desc.BeginningAccess.Clear.ClearValue.Format = Private::toDirectX12(attachmentDescriptor.format);

            desc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
        };

        for (size_t i = 0; i < colorCount; ++i)
        {
            if (!renderTargetAttachement[i])
                continue;

            fillRenderTargetDesc(desc[i], *renderTargetAttachement[i], colorAtt[i]);
            std::copy_n(beginRenderPass.clearColors[i].color.begin(), beginRenderPass.clearColors[i].color.size(),
                        desc[i].BeginningAccess.Clear.ClearValue.Color);
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthDesc{};
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* depthDescPtr = nullptr;

        if (depthAtt)
        {
            const Texture& depthTexture = *depthTargetAttachement.value();
            const std::optional<TiRHI::DirectX12::Private::DescriptorHandle>& handle =
                Texture::Private::getRenderTargetViewHandle(depthTexture);
            if (!handle)
            {
                RHI_LOG_ERROR(std::format(L"Failed to get D3D12_CPU_DESCRIPTOR_HANDLE fo the texture {}",
                                          depthTexture.getNameW()),
                              RhiApi::DirectX12);
                return false;
            }
            depthDesc.cpuDescriptor = handle->cpuHandle;
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

        m_commandList->BeginRenderPass(static_cast<UINT>(colorCount), colorCount > 0 ? desc.data() : nullptr,
                                       depthDescPtr, D3D12_RENDER_PASS_FLAG_NONE);

        // set submitted value
        for (auto& texture : renderTargets.getColorAttachements())
        {
            if (texture)
                texture->setSubmissionValue(m_lastSubmissionValue);
        }

        if (const std::optional<Texture*>& texture = renderTargets.getDepthAttachement())
        {
            if (texture)
                (*texture)->setSubmissionValue(m_lastSubmissionValue);
        }

        return true;
    }

    void CommandList::endRenderPass()
    {
        m_commandList->EndRenderPass();

        if (m_recordState.renderPassDescritptor)
        {
            renderTargetTransition(*m_recordState.renderTargets, true);
            setRenderPassRenderTargetState(false);
        }
        onEndRenderPass();
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

    void CommandList::transitionResource(Texture& texture, ResourceState state)
    {
        const ResourceState current = texture.getState();
        if (state == current)
            return;

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Transition.pResource = Texture::Private::getImage(texture);
        barrier.Transition.StateBefore = Private::toDirectX12(current);
        barrier.Transition.StateAfter = Private::toDirectX12(state);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        if (barrier.Transition.StateBefore == barrier.Transition.StateAfter)
        {
            texture.setState(state);
            return;
        }

        m_commandList->ResourceBarrier(1, &barrier);
        texture.setState(state);
    }

    void CommandList::transitionResource(Buffer& buffer, ResourceState state)
    {
        const ResourceState current = buffer.getState();
        if (state == current)
            return;

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Transition.pResource = buffer.getNativeBufferFrame();
        barrier.Transition.StateBefore = Private::toDirectX12(current);
        barrier.Transition.StateAfter = Private::toDirectX12(state);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        if (barrier.Transition.StateBefore == barrier.Transition.StateAfter)
        {
            buffer.setState(state);
            return;
        }

        m_commandList->ResourceBarrier(1, &barrier);
        buffer.setState(state);
    }

    void CommandList::bindPipeline(const _GraphicPipeline& graphicPipeline)
    {
        BaseCommandList::bindPipeline(graphicPipeline);

        if (!m_recordState.graphicPipeline)
            return;

        const DirectX12::Private::D3D12DescriptorRootSignatureLayout* root =
            GraphicPipeline::Private::getD3D12DescriptorRootSignatureLayout(*m_recordState.graphicPipeline);

        bindRootSignature(root ? root->rootSignature.Get() : nullptr);
        ID3D12PipelineState* state = GraphicPipeline::Private::getPipelineState(*m_recordState.graphicPipeline);
        m_commandList->SetPipelineState(state);

        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void CommandList::bindDescriptorSet(const DescriptorSet& descriptorSet, uint32_t set)
    {
        if (!BaseCommandList::onBindDescriptorSet(descriptorSet))
            return;

        const DirectX12::Private::D3D12DescriptorRootSignatureLayout* root =
            GraphicPipeline::Private::getD3D12DescriptorRootSignatureLayout(*m_recordState.graphicPipeline);

        if (!root)
            return;

        auto& bindingsLayout = descriptorSet.getBindingsLayout();

        if (bindingsLayout.bindings.empty())
            return;

        auto& spaceDescriptor = root->spaceDescriptor;

        for (const auto& binding : bindingsLayout.bindings)
        {
            std::visit(
                overloads{[&](const DescriptorSet::SampledTexture& sampledTexture)
                          {
                              forEachShaderStage(
                                  sampledTexture.shaderStageFlag,
                                  [&]([[maybe_unused]] ShaderStageFlags stage, uint32_t index)
                                  {
                                      if (!sampledTexture.texture || !sampledTexture.sampler)
                                          return;

                                      std::optional<TiRHI::DirectX12::Private::DescriptorHandle> handleTexture =
                                          Texture::Private::getShaderRenderViewHandle(*sampledTexture.texture);
                                      std::optional<TiRHI::DirectX12::Private::DescriptorHandle> handleSampler =
                                          sampledTexture.sampler->getShaderViewHandle();

                                      if (!handleTexture || !handleSampler || handleTexture->gpuHandle.ptr == 0 ||
                                          handleSampler->gpuHandle.ptr == 0)
                                          return;

                                      auto& offsetAndCount =
                                          spaceDescriptor.bindingsSetTableOffset[index][sampledTexture.bindingIndex];
                                      assert(offsetAndCount.count == 2);
                                      auto& rootParameterBindingTexture =
                                          spaceDescriptor.rootParameterTable[index][offsetAndCount.offset];
                                      auto& rootParameterBindingSampler =
                                          spaceDescriptor.rootParameterTable[index][offsetAndCount.offset + 1];

                                      m_commandList->SetGraphicsRootDescriptorTable(
                                          rootParameterBindingTexture.rootParameterIndex, handleTexture->gpuHandle);

                                      m_commandList->SetGraphicsRootDescriptorTable(
                                          rootParameterBindingSampler.rootParameterIndex, handleSampler->gpuHandle);

                                      sampledTexture.texture->setSubmissionValue(m_lastSubmissionValue);
                                      sampledTexture.sampler->setSubmissionValue(m_lastSubmissionValue);
                                  });
                          },
                          [&](auto&& value)
                          { static_assert(AlwaysFalse<decltype(value)>, "Unsupported binding type"); }},
                binding);
        }
    }

    void CommandList::bindVertexBuffer(uint32_t bindingIndex, uint32_t offset, Buffer& buffer)
    {
        if (!onBindBuffer())
        {
            return;
        }

        std::span<const uint32_t> stridesPerBinding = m_recordState.graphicPipeline->getStridesPerBinding();

        if (stridesPerBinding.size() <= bindingIndex)
        {
            RHI_LOG_ERROR(std::format(L"Cannot bind vertex buffer at binding {}: graphics pipeline '{}' "
                                      L"has no valid vertex stride for this binding.",
                                      bindingIndex, m_recordState.graphicPipeline->getNameW()),
                          _Derived::Api);
        }

        D3D12_VERTEX_BUFFER_VIEW view{};
        view.BufferLocation = buffer.getNativeBufferFrame()->GetGPUVirtualAddress() + offset;
        view.SizeInBytes = buffer.getSize();
        view.StrideInBytes = stridesPerBinding[bindingIndex];

        m_commandList->IASetVertexBuffers(bindingIndex, 1, &view);
    }

    void CommandList::bindVertexBuffer(uint32_t bindingIndex, uint32_t offset, Buffer& buffer)
    {
        if (!onBindBuffer())
        {
            return;
        }

        std::span<const uint32_t> stridesPerBinding = m_recordState.graphicPipeline->getStridesPerBinding();

        if (stridesPerBinding.size() <= bindingIndex)
        {
            RHI_LOG_ERROR(std::format(L"Cannot bind vertex buffer at binding {}: graphics pipeline '{}' "
                                      L"has no valid vertex stride for this binding.",
                                      bindingIndex, m_recordState.graphicPipeline->getNameW()),
                          _Derived::Api);
        }

        D3D12_VERTEX_BUFFER_VIEW view{};
        view.BufferLocation = buffer.getNativeBufferFrame()->GetGPUVirtualAddress() + offset;
        view.SizeInBytes = buffer.getSize();
        view.StrideInBytes = stridesPerBinding[bindingIndex];

        m_commandList->IASetVertexBuffers(bindingIndex, 1, &view);
    }

    void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        m_commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandList::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                  int32_t vertexOffset, uint32_t firstInstance)
    {
        m_commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void CommandList::pushConstant(void* data, size_t size)
    {
        if (data == nullptr || size == 0)
            return;

        if (!m_recordState.graphicPipeline)
            return;

        if (!m_recordStateD3D12.currentRootSignature)
            return;

        if (std::optional<PushConstant> pushConstant = m_recordState.graphicPipeline->getPushConstant())
        {
            assert(size % 4 == 0);
            const DirectX12::Private::D3D12DescriptorRootSignatureLayout* root =
                GraphicPipeline::Private::getD3D12DescriptorRootSignatureLayout(*m_recordState.graphicPipeline);

            const UINT rootParameterIndex = root->spaceDescriptor.pushConstantParameterIndex;

            m_commandList->SetGraphicsRoot32BitConstants(rootParameterIndex, static_cast<UINT>(size / 4), data, 0);
        }
    }

    void CommandList::upload2d(Texture& texture, std::span<const std::byte> data)
    {
        if (!m_device || !m_commandList || !texture || data.empty())
            return;

        assert(texture.getState() == ResourceState::CopyDestination);

        ID3D12Resource* dstResource = Texture::Private::getImage(texture);
        ID3D12Device* device = m_device->getNativeDevice().Get();

        if (!dstResource)
            return;

        const D3D12_RESOURCE_DESC desc = dstResource->GetDesc();

        if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || desc.SampleDesc.Count != 1)
            return;

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        UINT numRows = 0;
        UINT64 rowSize = 0;
        UINT64 uploadSize = 0;

        device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &numRows, &rowSize, &uploadSize);

        if (numRows == 0 || rowSize == 0 || data.size_bytes() != rowSize * numRows)
            return;

        DirectX12::Private::StagingBuffer staging(*m_device, uploadSize, m_lastSubmissionValue);

        if (!staging)
            return;

        std::byte* mapped = staging.mapData();

        if (!mapped)
            return;

        const auto* src = reinterpret_cast<const std::byte*>(data.data());

        // Copy CPU data directly into the aligned staging buffer.
        for (UINT row = 0; row < numRows; row++)
        {
            std::memcpy(mapped + footprint.Offset + static_cast<size_t>(row) * footprint.Footprint.RowPitch,
                        src + static_cast<size_t>(row) * rowSize, static_cast<size_t>(rowSize));
        }

        staging.unmapData();

        D3D12_TEXTURE_COPY_LOCATION source{};
        source.pResource = staging.getBuffer();
        source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        source.PlacedFootprint = footprint;

        D3D12_TEXTURE_COPY_LOCATION destination{};
        destination.pResource = dstResource;
        destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        destination.SubresourceIndex = 0;

        m_commandList->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
    }

    void CommandList::uploadBuffer(Buffer& buffer, uint64_t offset, std::span<const std::byte> data)
    {
        if (!m_device || !m_commandList || !buffer || data.empty())
            return;

        ID3D12Device* nativeDevice = m_device->getNativeDevice().Get();

        D3D12MA::Allocator& allocator = Device::Private::getAllocator(*m_device);

        ID3D12Resource* dst = buffer.getNativeBufferFrame();

        const uint64_t size = data.size_bytes();

        assert(size > 0);
        assert(offset <= dst->GetDesc().Width);
        assert(size <= dst->GetDesc().Width - offset);

        DirectX12::Private::StagingBuffer stagingBuffer(*m_device, size, m_lastSubmissionValue);

        assert(SUCCEEDED(stagingBuffer.operator bool()));

        D3D12_RANGE readRange{0, 0};
        if (std::byte* ptr = stagingBuffer.mapData())
        {
            std::memcpy(ptr, data.data(), size);
            stagingBuffer.unmapData();
        }
        m_commandList->CopyBufferRegion(dst, offset, stagingBuffer.getBuffer(), 0, size);
        buffer.setLastTimeLineUsedFrame(m_lastSubmissionValue);
    }

    void CommandList::resetOnRecord()
    {
        m_recordStateD3D12.reset();
    }

    void CommandList::bindDescriptorHeaps()
    {
        assert(m_device);

        ID3D12DescriptorHeap* heaps[] = {Device::Private::getDescriptorAllocatorCbvSrvUav(*m_device).getHeap().Get(),

                                         Device::Private::getDescriptorAllocatorSampler(*m_device).getHeap().Get()};

        m_commandList->SetDescriptorHeaps(2, heaps);
    }

    void CommandList::bindRootSignature(ID3D12RootSignature* rootSignature)
    {
        if (m_recordStateD3D12.currentRootSignature == rootSignature)
            return;

        m_commandList->SetGraphicsRootSignature(rootSignature);
        m_recordStateD3D12.currentRootSignature = rootSignature;
    }

    bool CommandList::renderTargetTransition(RenderTargets& renderTargets, bool IsOutTransition)
    {
        // Vulkan does make initial state to render state and final state by him self
        // here we need to do it by ourself

        const RenderPassDescriptor* renderPassDescriptor = renderTargets.getRenderPassDescriptor();
        if (!renderPassDescriptor)
            return false;

        std::span attachements = renderTargets.getColorAttachements();
        std::optional depthAttachement = renderTargets.getDepthAttachement();

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
            const bool isDepthAtt = (i == attachements.size() - 1) && renderPassDescriptor->getDepthAttachement();

            Texture& texture = isDepthAtt ? *(*depthAttachement) : *attachements[i];
            const AttachmentDescriptor& desc = isDepthAtt ? *renderPassDescriptor->getDepthAttachement()
                                                          : renderPassDescriptor->getColorAttachements()[i];

            const ResourceState current = texture.getState();

            if (!IsOutTransition && current != desc.initialState)
            {
                RHI_LOG_WARNING(std::format(L"CommandList {}: Texture {} is in state {} but render pass expected {}",
                                            getNameW(), texture.getNameW(), toWString(current),
                                            toWString(desc.initialState)),
                                RhiApi::DirectX12);
            }

            D3D12_RESOURCE_BARRIER& barrier = m_barriers.emplace_back();
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = Texture::Private::getImage(texture);
            barrier.Transition.StateBefore =
                IsOutTransition ? Private::toDirectX12(desc.renderState) : Private::toDirectX12(desc.initialState);
            barrier.Transition.StateAfter =
                IsOutTransition ? Private::toDirectX12(desc.finalState) : Private::toDirectX12(desc.renderState);
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            if (barrier.Transition.StateBefore && barrier.Transition.StateAfter)
            {
                RHI_LOG_WARNING(
                    std::format(L" Texture {} Transition state before is the same after", texture.getNameW()),
                    RhiApi::DirectX12);
            }

            m_commandList->ResourceBarrier(m_barriers.size(), m_barriers.data());

            return true;
        }
    }

    void CommandList::setLastSubmissionValue(uint64_t lastSubmissionValue)
    {
        m_lastSubmissionValue = lastSubmissionValue;
    }

} // namespace TiRHI::DirectX12
