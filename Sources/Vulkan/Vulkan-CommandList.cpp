#include <Vulkan/Vulkan-CommandList.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-GraphicPipeline.hpp>
#include <Vulkan/Vulkan-DescriptorSet.hpp>
#include <Vulkan/Private/Transition.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    CommandList::CommandList(RHI& rhi)
        : BaseCommandList(rhi)
    {
    }

    CommandList& CommandList::operator=(CommandList&& other) noexcept
    {
        if (this == &other)
            return *this;

        BaseCommandList::operator=(std::move(other));
        // Free buffers while their original pool still exists.
        m_commandBuffer.clear();
        m_commandPool = std::move(other.m_commandPool);
        m_commandBuffer = std::move(other.m_commandBuffer);
        m_volkTable = std::exchange(other.m_volkTable, std::nullopt);
        m_vulkanStorage = std::move(other.m_vulkanStorage);
        other.m_recordState.reset();
        return *this;
    }

    bool CommandList::build(Device& device)
    {
        vk::CommandPoolCreateInfo vkCommandPoolCreateInfo{};
        vkCommandPoolCreateInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        vkCommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        vkCommandPoolCreateInfo.queueFamilyIndex = device.getNativeGraphicQueueIndex();

        vk::Device vkDevice = device.getNativeDevice();
        if (vkDevice == VK_NULL_HANDLE)
        {
            RHI_LOG_ERROR(L"vkDevice was null handle", RhiApi::Vulkan);
            return false;
        }
        m_commandPool = vkDevice.createCommandPoolUnique(vkCommandPoolCreateInfo);

        if (!m_commandPool)
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

        m_volkTable = device.getVolkTable();
        return true;
    }

    bool CommandList::beginRecord()
    {
        if (!onBeginRecord())
            return false;

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
        if (!getRHI().getUseDebugLabel() || !m_volkTable)
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
        if (!getRHI().getUseDebugLabel() || !m_volkTable)
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
        if (currentState == state || Texture::Private::getImage(texture) == VK_NULL_HANDLE)
            return;

        if (state == ResourceState::CopyDestination)
            assert((texture.getFlags() & TextureFlags::CopyDestination) != TextureFlags::None);

        using namespace Private;

        const TransitionData& transitionData = transitionMap[getResourceCombinaisonIndex(currentState, state)];

        const vk::ImageMemoryBarrier barrier = makeImageBarrier(texture, currentState, state);

        getcurrentFrameCmb().pipelineBarrier(transitionData.srcPipelineStageFlag, transitionData.dstPipelineStageFlag,
                                             transitionData.dependencyFlags, {}, {}, barrier);
        texture.setState(state);
    }

    void CommandList::bindPipeline(const GraphicPipeline& graphicPipeline)
    {
        BaseCommandList::bindPipeline(graphicPipeline);
        getcurrentFrameCmb().bindPipeline(vk::PipelineBindPoint::eGraphics,
                                          GraphicPipeline::Private::getPipeline(*m_recordState.graphicPipeline));
    }

    void CommandList::bindDescriptorSet(const DescriptorSet& descriptorSet, uint32_t set)
    {
        if (!m_recordState.graphicPipeline)
            return;

        std::array<vk::DescriptorSet, 1> descriporSet = {DescriptorSet::Private::getDescriptorSetFrame(descriptorSet)};

        vk::CommandBuffer cmb = getcurrentFrameCmb();
        cmb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               GraphicPipeline::Private::getPipelineLayout(*m_recordState.graphicPipeline), set,
                               descriporSet, {});
    }

    void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        getcurrentFrameCmb().draw(vertexCount, instanceCount, firstVertex, firstInstance);
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

        if (std::optional<GraphicPipeline::PushConstant> pushConstant =
                m_recordState.graphicPipeline->getPushConstant())
        {
            getcurrentFrameCmb().pushConstants(layout, Vulkan::Private::toVulkan(pushConstant->flag), 0u,
                                               pushConstant->size, data);
        }
    }

    void CommandList::upload2D(Device& device, Texture& texture, std::span<const std::byte> data)
    {
        assert(texture.getState() == ResourceState::CopyDestination);

        const VkDeviceSize uploadSize =
            static_cast<VkDeviceSize>(Vulkan::Private::pixelSizeFromFormat(texture.getFormat())) * texture.getWidth() *
            texture.getHeight();

        if (data.size() != uploadSize || uploadSize == 0)
            return;

        VmaAllocator allocator = Device::Private::getAllocator(device);

        struct StagingBuffer
        {
            VkBuffer buffer = VK_NULL_HANDLE;
            VmaAllocation allocation = VK_NULL_HANDLE;
            void* mappedData = nullptr;
            VkDeviceSize capacity = 0;
        };

        // Temporary test implementation: assumes one device.
        static StagingBuffer staging{};

        if (staging.buffer == VK_NULL_HANDLE)
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = uploadSize;
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocationInfo{};
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationInfo.flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo resultInfo{};

            if (vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &staging.buffer, &staging.allocation,
                                &resultInfo) != VK_SUCCESS)
                return;

            staging.mappedData = resultInfo.pMappedData;
            staging.capacity = uploadSize;
        }

        if (uploadSize > staging.capacity)
            return;

        std::memcpy(staging.mappedData, data.data(), data.size());

        vmaFlushAllocation(allocator, staging.allocation, 0, uploadSize);

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

        getcurrentFrameCmb().copyBufferToImage(vk::Buffer(staging.buffer), Texture::Private::getImage(texture),
                                               vk::ImageLayout::eTransferDstOptimal, bufferImageCopy);
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

        barrier.oldLayout = Private::toVulkanImageLayout(current);
        barrier.newLayout = Private::toVulkanImageLayout(target);

        barrier.srcAccessMask = getAccessMask(current);
        barrier.dstAccessMask = getAccessMask(target);

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = Texture::Private::getImage(texture);
        barrier.subresourceRange = range;

        return barrier;
    }

} // namespace TiRHI::Vulkan
