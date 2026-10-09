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
        if (!onBeginRenderPass(beginRenderPass, renderTargets))
            return false;

        m_vulkanStorage.clearValues.clear();
        m_vulkanStorage.clearValues.reserve(beginRenderPass.clearValues.size());

        for (const auto& clearValues : beginRenderPass.clearValues)
        {
            vk::ClearValue clearValue;
            std::visit(overloaded{[&clearValue](const ClearValueDepthStencil& clearValueDepthStencil)
                                  {
                                      vk::ClearDepthStencilValue vkClearDepthStencilValue;
                                      vkClearDepthStencilValue.setDepth(clearValueDepthStencil.depth);
                                      vkClearDepthStencilValue.setStencil(clearValueDepthStencil.stencil);
                                      clearValue.setDepthStencil(vkClearDepthStencilValue);
                                  },
                                  [&clearValue](const ClearValueColor& clearValueColor)
                                  {
                                      vk::ClearColorValue vkClearColorValue;
                                      vkClearColorValue.setFloat32(clearValueColor.color);
                                      clearValue.setColor(vkClearColorValue);
                                  }},
                       clearValues);
            m_vulkanStorage.clearValues.emplace_back(clearValue);
        }

        vk::CommandBuffer cmd = getcurrentFrameCmb();

        vk::Rect2D renderArea;
        renderArea.offset.x = beginRenderPass.renderArea.offset.x;
        renderArea.offset.y = beginRenderPass.renderArea.offset.x;

        renderArea.extent.width = beginRenderPass.renderArea.extend.width;
        renderArea.extent.height = beginRenderPass.renderArea.extend.height;

        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.setRenderPass(renderTargets.getRenderPassDescriptor()->getNativeRenderPass())
            .setFramebuffer(RenderTargets::Private::getFrameBuffer(renderTargets))
            .setRenderArea(renderArea)
            .setClearValues(m_vulkanStorage.clearValues);

        getcurrentFrameCmb().beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

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
