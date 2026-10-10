#include <Vulkan/Vulkan-CommandList.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    CommandList::CommandList(RHI& rhi)
        : BaseCommandList(rhi)
    {
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

        m_volkTable = &device.getVolkTable();
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

    bool CommandList::beginRenderPass(const BeginRenderPass& beginRenderPass, const RenderTargets& renderTargets)
    {
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

        if (!onBeginRenderPass(beginRenderPass, renderTargets))
            return false;

        getcurrentFrameCmb().beginRenderPass(beginInfo, vk::SubpassContents::eInline);

        return true;
    }

    void CommandList::endRenderPass()
    {
        onEndRenderPass();
        getcurrentFrameCmb().endRenderPass();
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

    void CommandList::transitionResource(Texture& texture, const ResourceState state) const
    {
        const ResourceState currentState = texture.getState();
        if (currentState == state || Texture::Private::getImage(texture) == VK_NULL_HANDLE)
            return;

        const vk::PipelineStageFlags srcPipelineStageFlag = getTransitionSrcMask(currentState);
        const vk::PipelineStageFlags dstPipelineStageFlag = getTransitionDstMask(state);

        const vk::ImageMemoryBarrier barrier = makeImageBarrier(texture, currentState, state);

        getcurrentFrameCmb().pipelineBarrier(srcPipelineStageFlag, dstPipelineStageFlag,
                                             getTransitionDependencyMask(currentState, state), {}, {}, barrier);
        texture.setState(state);
    }

    vk::CommandBuffer CommandList::getcurrentFrameCmb() const
    {
        return m_commandBuffer[getRHI().getCurrentFrame()].get();
    }

    vk::PipelineStageFlags CommandList::getTransitionSrcMask(ResourceState current)
    {
        if (current == ResourceState::Undefined)
            return vk::PipelineStageFlagBits::eTopOfPipe;

        return Private::getPipelineStage(current);
    }

    vk::PipelineStageFlags CommandList::getTransitionDstMask(ResourceState target)
    {
        return Private::getPipelineStage(target);
    }

    vk::DependencyFlags CommandList::getTransitionDependencyMask(ResourceState current, ResourceState target)
    {
        return {};
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
