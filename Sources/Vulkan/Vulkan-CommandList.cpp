#include <Vulkan/Vulkan-CommandList.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>

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

    vk::CommandBuffer CommandList::getcurrentFrameCmb()
    {
        return m_commandBuffer[getRHI().getCurrentFrame()].get();
    }

} // namespace TiRHI::Vulkan
