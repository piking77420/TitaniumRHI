#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>

#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    RenderPassDescriptor::RenderPassDescriptor(RHI& rhi)
        : BaseRenderPassDescriptor<RenderPassDescriptor, RHI>(rhi)
    {
    }

    bool RenderPassDescriptor::build(Device& device)
    {
        return createRenderPass(device.getNativeDevice());
    }

    bool RenderPassDescriptor::createRenderPass(vk::Device device)
    {

        const std::span colorAttachements = getColorAttachements();
        const std::optional depthAttachment = getDepthAttachement();
        const PipelineType accepetdPipeline = getAcceptedPipelineType();

        if (colorAttachements.empty() && !depthAttachment)
        {
            RHI_LOG_ERROR(std::format(L"Empty RenderTargets", getNameW()), RhiApi::Vulkan);
            return false;
        }

        std::vector<vk::AttachmentDescription> vkAttachmentDescriptions;
        vkAttachmentDescriptions.resize(colorAttachements.size() + (depthAttachment ? 1 : 0));
        std::vector<vk::AttachmentReference> vkColorAttachmentReferences;
        vkColorAttachmentReferences.resize(colorAttachements.size());

        for (size_t i = 0; i < colorAttachements.size(); i++)
        {
            const AttachmentDescriptor& attachmentDescription = colorAttachements[i];
            vk::AttachmentDescription& vkAttachmentDescription = vkAttachmentDescriptions[i];
            vkAttachmentDescription.format = Private::toVulkan(attachmentDescription.format);
            vkAttachmentDescription.samples = vk::SampleCountFlagBits::e1; // TODO
            vkAttachmentDescription.loadOp = Private::toVulkanAttachementLoadOp(attachmentDescription.loadOp);
            vkAttachmentDescription.storeOp = Private::toVulkanAttachementStoreOp(attachmentDescription.storeOp);
            vkAttachmentDescription.stencilLoadOp =
                Private::toVulkanAttachementLoadOp(attachmentDescription.stencilLoadOp);
            vkAttachmentDescription.stencilStoreOp =
                Private::toVulkanAttachementStoreOp(attachmentDescription.stencilStoreOp);
            vkAttachmentDescription.initialLayout =
                attachmentDescription.loadOp == LoadOp::LoadOp
                    ? Private::toVulkanImageLayout(attachmentDescription.renderState)
                    : vk::ImageLayout::eUndefined;
            vkAttachmentDescription.finalLayout = Private::toVulkanImageLayout(attachmentDescription.finalState);

            vk::AttachmentReference& attachmentReference = vkColorAttachmentReferences[i];
            attachmentReference.attachment = static_cast<uint32_t>(i);
            attachmentReference.layout = Private::toVulkanImageLayout(attachmentDescription.renderState);
        }

        vk::AttachmentReference vkDepthAttachmentReference{};

        if (depthAttachment)
        {
            const uint32_t depthIndex = static_cast<uint32_t>(colorAttachements.size());

            const AttachmentDescriptor& attachment = *depthAttachment;
            vk::AttachmentDescription& vkAttachment = vkAttachmentDescriptions[depthIndex];

            vkAttachment.format = Private::toVulkan(attachment.format);
            vkAttachment.samples = vk::SampleCountFlagBits::e1; // TODO
            vkAttachment.loadOp = Private::toVulkanAttachementLoadOp(attachment.loadOp);
            vkAttachment.storeOp = Private::toVulkanAttachementStoreOp(attachment.storeOp);
            vkAttachment.stencilLoadOp = Private::toVulkanAttachementLoadOp(attachment.stencilLoadOp);
            vkAttachment.stencilStoreOp = Private::toVulkanAttachementStoreOp(attachment.stencilStoreOp);

            vkAttachment.initialLayout = attachment.loadOp == LoadOp::LoadOp
                                             ? Private::toVulkanImageLayout(attachment.renderState)
                                             : vk::ImageLayout::eUndefined;

            vkAttachment.finalLayout = Private::toVulkanImageLayout(attachment.finalState);

            vkDepthAttachmentReference.attachment = depthIndex;
            vkDepthAttachmentReference.layout = Private::toVulkanImageLayout(attachment.renderState);
        }

        vk::SubpassDescription subpassDesc{};
        subpassDesc.pipelineBindPoint = Private::toPipelineBindPoint(accepetdPipeline);
        subpassDesc.colorAttachmentCount = static_cast<uint32_t>(vkColorAttachmentReferences.size());
        subpassDesc.pColorAttachments =
            vkColorAttachmentReferences.empty() ? nullptr : vkColorAttachmentReferences.data();

        if (depthAttachment)
        {
            subpassDesc.pDepthStencilAttachment = &vkDepthAttachmentReference;
        }

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.attachmentCount = static_cast<uint32_t>(vkAttachmentDescriptions.size());
        renderPassInfo.pAttachments = vkAttachmentDescriptions.data();
        renderPassInfo.subpassCount = 1u;
        renderPassInfo.pSubpasses = &subpassDesc;

        m_renderPass = device.createRenderPassUnique(renderPassInfo);

        return m_renderPass.get() != VK_NULL_HANDLE;
    }

} // namespace TiRHI::Vulkan
