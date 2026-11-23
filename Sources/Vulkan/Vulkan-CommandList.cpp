#include <variant>

#include <Vulkan/Vulkan-CommandList.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-GraphicPipeline.hpp>
#include <Vulkan/Vulkan-Sampler.hpp>
#include <Vulkan/Vulkan-DescriptorSet.hpp>
#include <Vulkan/Private/Transition.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Vulkan/Private/StaginBuffer.hpp>

namespace TiRHI::Vulkan
{
    [[nodiscard]]
    constexpr bool isValidBufferState(ResourceState state) noexcept
    {
        switch (state)
        {
        case ResourceState::RenderTarget:
        case ResourceState::DepthWrite:
        case ResourceState::DepthRead:
        case ResourceState::Present:
        case ResourceState::Count:
            return false;

        default:
            return true;
        }
    }

    CommandList::CommandList(Device& device)
        : BaseCommandList(device)
    {
    }

    CommandList::~CommandList() = default;

    bool CommandList::build()
    {
        vk::CommandPoolCreateInfo vkCommandPoolCreateInfo{};
        vkCommandPoolCreateInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        vkCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        vkCommandPoolCreateInfo.queueFamilyIndex = getDevice().getNativeGraphicQueueIndex();

        vk::Device vkDevice = getDevice().getNativeDevice();
        if (vkDevice == VK_NULL_HANDLE)
        {
            RHI_LOG_ERROR(L"vkDevice was null handle", RhiApi::Vulkan);
            return false;
        }
        m_commandPool = vkDevice.createCommandPoolUnique(vkCommandPoolCreateInfo);

        if (!m_commandPool) [[unlikely]]
        {
            RHI_LOG_ERROR(L"Failed to create command pool", RhiApi::Vulkan);
            return false;
        }

        vk::CommandBufferAllocateInfo vkCommandBufferAllocateInfo{};
        vkCommandBufferAllocateInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        vkCommandBufferAllocateInfo.commandPool = m_commandPool.get();
        vkCommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        vkCommandBufferAllocateInfo.commandBufferCount = getRHI().getFrameInFlight();

        m_commandBuffer = vkDevice.allocateCommandBuffersUnique(vkCommandBufferAllocateInfo);
        if (m_commandBuffer.empty())
        {
            RHI_LOG_ERROR(L"Failed to allocate command buffers", RhiApi::Vulkan);
            return false;
        }

        m_volkTable = &getDevice().getVolkTable();
        m_device = &getDevice();
        return m_device && m_volkTable;
    }

    bool CommandList::beginRecord()
    {
        if (!onBeginRecord())
            return false;

        if (!m_device)
            return false;

        m_submissionValue = Device::Private::getNextSubmissionValue(*m_device);
        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = {};
        getcurrentFrameCmb().reset();
        getcurrentFrameCmb().begin(beginInfo);

        return true;
    }

    bool CommandList::endRecord()
    {
        getcurrentFrameCmb().end();
        onEndRecord();
        return true;
    }

    void CommandList::beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color)
    {
        if (!getRHI().getUseDebugLabel() || !m_volkTable) [[unlikely]]
            return;

        vk::DebugUtilsLabelEXT label{};
        label.pLabelName = name.data();

        if (color)
        {
            std::copy(color->begin(), color->end(), label.color.begin());
        }

        getcurrentFrameCmb().beginDebugUtilsLabelEXT(label);
    }

    void CommandList::endDebugLabel()
    {
        if (!getRHI().getUseDebugLabel() || !m_volkTable) [[unlikely]]
            return;

        getcurrentFrameCmb().endDebugUtilsLabelEXT();
    }

    bool CommandList::beginRenderPass(const BeginRenderPass& beginRenderPass, RenderTargets& renderTargets)
    {
        if (!onBeginRenderPass(beginRenderPass, renderTargets))
            return false;

        const auto* pass = renderTargets.getRenderPassDescriptor();
        if (!pass)
            return false;

        const auto colorAttachments = pass->getColorAttachements();
        const auto depthAttachment = pass->getDepthAttachement();

        const size_t attachmentCount = colorAttachments.size() + (depthAttachment.has_value() ? 1 : 0);

        // Validate before beginning the render pass.
        if (beginRenderPass.clearColors.size() > colorAttachments.size())
            return false;

        // Assuming all color attachments use LoadOp::Clear.
        if (beginRenderPass.clearColors.size() < colorAttachments.size())
            return false;

        if (depthAttachment && depthAttachment->loadOp == LoadOp::Clear && !beginRenderPass.clearDepthStencil)
            return false;

        m_vulkanStorage.clearValues.reserve(attachmentCount);

        // Color attachments
        for (size_t i = 0; i < beginRenderPass.clearColors.size(); ++i)
        {
            m_vulkanStorage.clearValues.emplace_back().setColor(
                vk::ClearColorValue{beginRenderPass.clearColors[i].color});
        }

        // Depth/stencil attachment is the last attachment.
        if (depthAttachment && beginRenderPass.clearDepthStencil)
        {
            const auto& ds = *beginRenderPass.clearDepthStencil;

            m_vulkanStorage.clearValues.emplace_back().setDepthStencil(
                vk::ClearDepthStencilValue{ds.depth, ds.stencil});
        }

        vk::Rect2D renderArea{};
        renderArea.offset.x = beginRenderPass.renderArea.offset.x;
        renderArea.offset.y = beginRenderPass.renderArea.offset.y;
        renderArea.extent.width = beginRenderPass.renderArea.extend.width;
        renderArea.extent.height = beginRenderPass.renderArea.extend.height;

        vk::RenderPassBeginInfo beginInfo{};
        beginInfo.setRenderPass(pass->getNativeRenderPass())
            .setFramebuffer(RenderTargets::Private::getFrameBuffer(renderTargets))
            .setRenderArea(renderArea)
            .setClearValues(m_vulkanStorage.clearValues);

        setRenderPassRenderTargetState(true);

        getcurrentFrameCmb().beginRenderPass(beginInfo, vk::SubpassContents::eInline);

        // set submitted value
        for (auto& texture : renderTargets.getColorAttachements())
        {
            if (texture)
                texture->setSubmissionValue(m_submissionValue);
        }

        if (const std::optional<Texture*>& texture = renderTargets.getDepthAttachement())
        {
            if (texture)
                (*texture)->setSubmissionValue(m_submissionValue);
        }

        renderTargets.setSubmissionValue(m_submissionValue);
        return true;
    }

    void CommandList::endRenderPass()
    {
        getcurrentFrameCmb().endRenderPass();
        setRenderPassRenderTargetState(false);
        onEndRenderPass();
    }

    void CommandList::setViewPort(const Viewport& viewPort)
    {
        const vk::Viewport viewport{viewPort.position.x,    viewPort.position.y, viewPort.extend.width,
                                    viewPort.extend.height, viewPort.minDepth,   viewPort.maxDepth};
        getcurrentFrameCmb().setViewport(0, viewport);
    }

    void CommandList::setScissors(const Rect2D& rect2d)
    {
        const vk::Rect2D scissor{{rect2d.offset.x, rect2d.offset.y}, {rect2d.extend.width, rect2d.extend.height}};

        getcurrentFrameCmb().setScissor(0, scissor);
    }

    void CommandList::transitionResource(Texture& texture, const ResourceState state)
    {
        const ResourceState currentState = texture.getState();
        if (currentState == state || Texture::Private::getImage(texture) == VK_NULL_HANDLE) [[unlikely]]
            return;

        if (state == ResourceState::CopyDestination) [[unlikely]]
            assert((texture.getFlags() & TextureFlags::CopyDestination) != TextureFlags::None);

        using namespace Private;

        const TransitionData& transitionData = transitionMap[getResourceCombinaisonIndex(currentState, state)];

        const vk::ImageMemoryBarrier barrier = makeImageBarrier(texture, currentState, state);

        getcurrentFrameCmb().pipelineBarrier(transitionData.srcPipelineStageFlag, transitionData.dstPipelineStageFlag,
                                             transitionData.dependencyFlags, {}, {}, barrier);
        texture.setState(state);
    }

    void CommandList::transitionResource(Buffer& buffer, ResourceState state)
    {
        const ResourceState currentState = buffer.getState();

        if (currentState == state)
            return;

        if (buffer.getVkBufferFrame() == VK_NULL_HANDLE) [[unlikely]]
        {
            RHI_LOG_ERROR(L"Cannot transition buffer: the Vulkan buffer handle is null.", Api);
            return;
        }

        if (!isValidBufferState(state))
        {
            RHI_LOG_ERROR(std::format(L"Cannot transition buffer from '{}' to '{}': "
                                      L"the requested resource state is invalid for buffers.",
                                      toWString(currentState), toWString(state)),
                          Api);
        }
        using namespace Private;
        const vk::BufferMemoryBarrier barrier = makeBufferBarrier(buffer, currentState, state);
        const TransitionData& transitionData = transitionMap[getResourceCombinaisonIndex(currentState, state)];

        getcurrentFrameCmb().pipelineBarrier(transitionData.srcPipelineStageFlag, transitionData.dstPipelineStageFlag,
                                             {}, {}, barrier, {});
        buffer.setState(state);
    }

    void CommandList::bindPipeline(const GraphicPipeline& graphicPipeline)
    {
        BaseCommandList::bindPipeline(graphicPipeline);
        getcurrentFrameCmb().bindPipeline(vk::PipelineBindPoint::eGraphics,
                                          GraphicPipeline::Private::getPipeline(*m_recordState.graphicPipeline));
    }

    void CommandList::bindDescriptorSet(const DescriptorSet& descriptorSet, uint32_t set)
    {
        if (!m_recordState.graphicPipeline) [[unlikely]]
            return;

        std::array<vk::DescriptorSet, 1> descriporSet = {DescriptorSet::Private::getDescriptorSetFrame(descriptorSet)};
        getcurrentFrameCmb().bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            GraphicPipeline::Private::getPipelineLayout(*m_recordState.graphicPipeline), set, descriporSet, {});
        for (auto& binding : descriptorSet.getBindingsLayout().bindings)
        {
            std::visit(
                [&](auto&& value)
                {
                    using T = std::remove_cvref_t<decltype(value)>;

                    if constexpr (std::is_same_v<T, DescriptorSet::SampledTexture>)
                    {
                        if (value.texture)
                            value.texture->setSubmissionValue(m_submissionValue);

                        if (value.sampler)
                            value.sampler->setSubmissionValue(m_submissionValue);
                    }
                    else if constexpr (std::is_same_v<T, DescriptorSet::ConstantBuffer>)
                    {
                        value.buffer->setSubmissionValue(m_submissionValue);
                    }
                    else
                    {
                        static_assert(AlwaysFalse<T>, "Unsupported descriptor binding type");
                    }
                },
                binding);
        }
    }

    void CommandList::bindVertexBuffer(uint32_t bindingIndex, uint64_t offset, Buffer& buffer)
    {
        if (!buffer) [[unlikely]]
            return;

        if (!onBindBuffer()) [[unlikely]]
            return;

        getcurrentFrameCmb().bindVertexBuffers(bindingIndex, buffer.getVkBufferFrame(), offset);
    }

    void CommandList::bindIndexBuffer(Buffer& buffer, uint64_t offset, IndexType format)
    {
        if (!buffer) [[unlikely]]
            return;

        if (!onBindBuffer()) [[unlikely]]
        {
            return;
        }

        getcurrentFrameCmb().bindIndexBuffer(buffer.getVkBufferFrame(), static_cast<vk::DeviceSize>(offset),
                                             Vulkan::Private::toVulkan(format));
    }

    void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        if (m_volkTable) [[likely]]
            m_volkTable->vkCmdDraw(getcurrentFrameCmb(), vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandList::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                  int32_t vertexOffset, uint32_t firstInstance)
    {
        getcurrentFrameCmb().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void CommandList::pushConstant(void* data, uint32_t size)
    {
        if (data == nullptr || size == 0)
            return;

        if (!m_recordState.graphicPipeline)
            return;
        vk::PipelineLayout layout = GraphicPipeline::Private::getPipelineLayout(*m_recordState.graphicPipeline);
        if (layout == VK_NULL_HANDLE)
        {
            RHI_LOG_ERROR(L"vk::PipelineLayout was null", Api);
            return;
        }

        if (std::optional<PushConstant> pushConstant = m_recordState.graphicPipeline->getPushConstant())
        {
            getcurrentFrameCmb().pushConstants(layout, Vulkan::Private::toVulkan(pushConstant->flag), 0u,
                                               pushConstant->size, data);
        }
    }

    void CommandList::upload2d(Texture& texture, std::span<const std::byte> data)
    {
        if (!m_device || !texture) [[unlikely]]
            return;
        assert(texture.getState() == ResourceState::CopyDestination);
        std::array<vk::BufferImageCopy, 1> bufferImageCopy{};
        bufferImageCopy[0]
            .setBufferOffset(0)
            .setBufferRowLength(0)
            .setBufferImageHeight(0)
            .setImageSubresource(vk::ImageSubresourceLayers{}
                                     .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                     .setMipLevel(0)
                                     .setBaseArrayLayer(0)
                                     .setLayerCount(1))
            .setImageOffset({0, 0, 0})
            .setImageExtent({texture.getWidth(), texture.getHeight(), 1});

        const vk::Image vktexture = Texture::Private::getImage(texture);

        if (vktexture == VK_NULL_HANDLE) [[unlikely]]
            return;

        // TODO set m_submissionValue for texture
        Vulkan::Private::StagingBuffer stagingBuffer(*m_device, data.size_bytes(), m_submissionValue);

        if (stagingBuffer.getBuffer() == VK_NULL_HANDLE) [[unlikely]]
            return;

        if (std::byte* ptr = stagingBuffer.mapData()) [[likely]]
        {
            std::memcpy(ptr, data.data(), data.size_bytes());
            stagingBuffer.unmapData();
        }

        getcurrentFrameCmb().copyBufferToImage(stagingBuffer.getBuffer(), vktexture,
                                               vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
        texture.setSubmissionValue(m_submissionValue);
    }

    void CommandList::uploadBuffer(Buffer& buffer, uint64_t offset, std::span<const std::byte> data)
    {
        if (!m_device || !buffer) [[unlikely]]
            return;

        if (data.empty())
            return;

        assert(buffer.getState() == ResourceState::CopyDestination);

        assert(offset <= buffer.getSize());
        assert(data.size_bytes() <= buffer.getSize() - offset);

        Vulkan::Private::StagingBuffer stagingBuffer(*m_device, data.size_bytes(), m_submissionValue);

        if (stagingBuffer.getBuffer() == VK_NULL_HANDLE) [[unlikely]]
            return;

        std::byte* ptr = stagingBuffer.mapData();

        if (!ptr) [[unlikely]]
            return;

        std::memcpy(ptr, data.data(), data.size_bytes());
        stagingBuffer.unmapData();

        vk::BufferCopy region{};
        region.srcOffset = static_cast<vk::DeviceSize>(0);
        region.dstOffset = static_cast<vk::DeviceSize>(offset);
        region.size = data.size_bytes();

        getcurrentFrameCmb().copyBuffer(stagingBuffer.getBuffer(), buffer.getVkBufferFrame(), region);

        buffer.setSubmissionValue(m_submissionValue);
    }

    vk::CommandBuffer CommandList::getcurrentFrameCmb() const
    {
        return m_commandBuffer[getRHI().getCurrentFrame()].get();
    }

    vk::ImageMemoryBarrier CommandList::makeImageBarrier(Texture& texture, ResourceState current, ResourceState target)
    {
        using namespace Private;

        // TODO be able to choose level an layer
        vk::ImageSubresourceRange range{};
        range.aspectMask = vk::ImageAspectFlagBits::eColor;
        range.baseMipLevel = 0;
        range.levelCount = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer = 0;
        range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        vk::ImageMemoryBarrier barrier{};

        barrier.oldLayout = Vulkan::Private::toVulkanImageLayout(current);
        barrier.newLayout = Vulkan::Private::toVulkanImageLayout(target);

        barrier.srcAccessMask = getAccessMask(current);
        barrier.dstAccessMask = getAccessMask(target);

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = Texture::Private::getImage(texture);
        barrier.subresourceRange = range;

        return barrier;
    }

    vk::BufferMemoryBarrier CommandList::makeBufferBarrier(Buffer& buffer, ResourceState current, ResourceState target)
    {
        using namespace Private;

        vk::BufferMemoryBarrier barrier{};

        barrier.srcAccessMask = getAccessMask(current);
        barrier.dstAccessMask = getAccessMask(target);

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.buffer = buffer.getVkBufferFrame();
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;

        return barrier;
    }

} // namespace TiRHI::Vulkan
