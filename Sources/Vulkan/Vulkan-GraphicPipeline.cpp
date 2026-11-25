#include <Vulkan/Vulkan-GraphicPipeline.hpp>

#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    GraphicPipeline::~GraphicPipeline()
    {
        destroy();
    }

    GraphicPipeline::GraphicPipeline(GraphicPipeline&& other) noexcept
        : BaseGraphicPipiline(std::move(other))
    {
        m_pipelineLayout = std::exchange(other.m_pipelineLayout, vk::PipelineLayout{});
        m_pipeline = std::exchange(other.m_pipeline, vk::Pipeline{});
    }

    GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();
        BaseGraphicPipiline::operator=(std::move(other));
        m_pipelineLayout = std::exchange(other.m_pipelineLayout, vk::PipelineLayout{});
        m_pipeline = std::exchange(other.m_pipeline, vk::Pipeline{});

        return *this;
    }

    bool GraphicPipeline::build()
    {
        if (!getRenderPassDescriptor() || !sanityCheck())
            return false;

        const BinaryModule* vertexBinary = nullptr;

        const BinaryModule* fragmentBinary = nullptr;

        if (!getModule(&vertexBinary, ShaderStage::Vertex) || !getModule(&fragmentBinary, ShaderStage::Pixel))
            return false;

        auto createShaderModule = [&](const BinaryModule& binary) -> vk::UniqueShaderModule
        {
            if (binary.size == 0 || binary.size % 4 != 0)
                throw std::runtime_error("Invalid SPIR-V size");

            vk::ShaderModuleCreateInfo info{};
            info.codeSize = binary.size;

            // vkCreateShaderModule accepts SPIR-V as uint32_t words.
            // Copy into aligned storage.
            std::vector<uint32_t> code(binary.size / 4);
            std::memcpy(code.data(), binary.data.get(), binary.size);

            info.pCode = code.data();

            return getDevice().getNativeDevice().createShaderModuleUnique(info);
        };

        vk::UniqueShaderModule vertexModule = createShaderModule(*vertexBinary);

        vk::UniqueShaderModule fragmentModule = createShaderModule(*fragmentBinary);

        // 2. Shader stages
        std::array<vk::PipelineShaderStageCreateInfo, 2> stages{};

        stages[0]
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(*vertexModule)
            .setPName(vertexBinary->entryPoint.data());

        stages[1]
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(*fragmentModule)
            .setPName(fragmentBinary->entryPoint.data());

        std::vector<vk::VertexInputBindingDescription> vkVertexBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> vkVertexInputAttributeDescriptions;

        // 3. vertex buffers
        vk::PipelineVertexInputStateCreateInfo vertexInput{};
        if (!getVertexBindingDescription().empty() || !getVertexInputAttributeDescription().empty())
        {
            vkVertexBindingDescription.reserve(getVertexBindingDescription().size());
            vkVertexInputAttributeDescriptions.reserve(getVertexBindingDescription().size());

            for (const auto& vertexBindingDescription : getVertexBindingDescription())
                vkVertexBindingDescription.emplace_back(Vulkan::Private::toVulkan(vertexBindingDescription));

            for (const auto& vertexInputAttributeDescription : getVertexInputAttributeDescription())
                vkVertexInputAttributeDescriptions.emplace_back(
                    Vulkan::Private::toVulkan(vertexInputAttributeDescription));

            vertexInput.pVertexAttributeDescriptions = vkVertexInputAttributeDescriptions.data();
            vertexInput.vertexAttributeDescriptionCount =
                static_cast<uint32_t>(vkVertexInputAttributeDescriptions.size());

            vertexInput.pVertexBindingDescriptions = vkVertexBindingDescription.data();
            vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(vkVertexBindingDescription.size());
        }

        // 4. Triangle list
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.topology = Vulkan::Private::toVulkan(getPrimitiveTopology());
        inputAssembly.primitiveRestartEnable = vk::False;

        // 5. Dynamic viewport and scissor
        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        std::array dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.setDynamicStates(dynamicStates);

        // 6. Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.depthClampEnable = vk::False;
        rasterizer.rasterizerDiscardEnable = vk::False;
        rasterizer.polygonMode = Vulkan::Private::toVulkan(getPolygoneMode());
        rasterizer.cullMode = Vulkan::Private::toVulkan(getCullFlagMode());
        rasterizer.frontFace = Vulkan::Private::toVulkan(getFrontFace());
        rasterizer.depthBiasEnable = vk::False;
        rasterizer.lineWidth = 1.0f;

        // Depth
        vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};

        pipelineDepthStencilStateCreateInfo.pNext = nullptr;
        pipelineDepthStencilStateCreateInfo.flags = {};

        pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLess;

        pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        pipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;
        pipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f;

        pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        pipelineDepthStencilStateCreateInfo.front.failOp = vk::StencilOp::eKeep;
        pipelineDepthStencilStateCreateInfo.front.passOp = vk::StencilOp::eKeep;
        pipelineDepthStencilStateCreateInfo.front.depthFailOp = vk::StencilOp::eKeep;
        pipelineDepthStencilStateCreateInfo.front.compareOp = vk::CompareOp::eAlways;
        pipelineDepthStencilStateCreateInfo.front.compareMask = 0xFF;
        pipelineDepthStencilStateCreateInfo.front.writeMask = 0xFF;
        pipelineDepthStencilStateCreateInfo.front.reference = 0;

        pipelineDepthStencilStateCreateInfo.back.failOp = vk::StencilOp::eKeep;
        pipelineDepthStencilStateCreateInfo.back.passOp = vk::StencilOp::eKeep;
        pipelineDepthStencilStateCreateInfo.back.depthFailOp = vk::StencilOp::eKeep;
        pipelineDepthStencilStateCreateInfo.back.compareOp = vk::CompareOp::eAlways;
        pipelineDepthStencilStateCreateInfo.back.compareMask = 0xFF;
        pipelineDepthStencilStateCreateInfo.back.writeMask = 0xFF;
        pipelineDepthStencilStateCreateInfo.back.reference = 0;

        // 7. Multisampling
        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampling.sampleShadingEnable = vk::False;

        // 8. Color blending
        vk::PipelineColorBlendAttachmentState colorAttachment{};
        colorAttachment.blendEnable = vk::False;
        colorAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.setAttachments(colorAttachment);

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

        uint32_t bindingCount = getDescriptorSetLayoutEntries().size();

        if (bindingCount > 0)
        {
            descriptorSetLayouts.resize(getMaxSetIndex() + 1, VK_NULL_HANDLE);
            for (auto& entry : getDescriptorSetLayoutEntries())
            {
                vk::DescriptorSetLayout descriptorSetLayout =
                    Device::Private::getDescriptorManager(getDevice()).getDescriptorSetLayout(entry.layout);

                if (descriptorSetLayout == VK_NULL_HANDLE)
                {
                    RHI_LOG_ERROR(L"Failed to resolve descriptorSetLayout from pipieline", Api);
                    return false;
                }
                descriptorSetLayouts[entry.setIndex] = descriptorSetLayout;
            }
        }

        // Pipeline layout
        vk::PipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.setPSetLayouts(descriptorSetLayouts.data())
            .setSetLayoutCount(static_cast<uint32_t>(descriptorSetLayouts.size()));

        if (const std::optional<PushConstant>& pushConstant = getPushConstant())
        {
            vk::PushConstantRange pushConstantRange{};
            pushConstantRange.setStageFlags(Vulkan::Private::toVulkan(pushConstant->flag))
                .setOffset(0)
                .setSize(pushConstant->size);

            layoutInfo.setPPushConstantRanges(&pushConstantRange).setPushConstantRangeCount(1);
        }

        m_pipelineLayout = getDevice().getNativeDevice().createPipelineLayout(layoutInfo);

        // 10. Graphics pipeline
        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.setStages(stages);
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = getRenderPassDescriptor()->getNativeRenderPass();
        pipelineInfo.subpass = 0;
        if (getRenderPassDescriptor()->getDepthAttachement())
            pipelineInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;

        m_pipeline = getDevice().getNativeDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;

        vk::DebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.objectType = vk::ObjectType::ePipeline;
        nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkPipeline>(m_pipeline));
        nameInfo.pObjectName = getName().data();

        getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);

        return m_pipeline != VK_NULL_HANDLE;
    }

    void GraphicPipeline::destroy()
    {
        Device::Private::getDeferredDeletionQueue(getDevice())
            .retire(getLastFrameUsed(),
                    [device = getDevice().getNativeDevice(), pipelineLayout = std::move(m_pipelineLayout),
                     pipeline = std::move(m_pipeline)]()
                    {
                        device.destroyPipelineLayout(pipelineLayout);

                        device.destroyPipeline(pipeline);
                    });

        m_pipelineLayout = VK_NULL_HANDLE;
        m_pipeline = VK_NULL_HANDLE;
    }

} // namespace TiRHI::Vulkan
