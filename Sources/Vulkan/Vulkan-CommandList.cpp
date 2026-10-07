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

    vk::CommandBuffer CommandList::getcurrentFrameCmb()
    {
        return m_commandBuffer[getRHI().getCurrentFrame()].get();
    }

} // namespace TiRHI::Vulkan
