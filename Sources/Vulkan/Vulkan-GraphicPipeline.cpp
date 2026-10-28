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
        m_device = std::exchange(other.m_device, vk::Device{});
        m_pipelineLayout = std::exchange(other.m_pipelineLayout, vk::PipelineLayout{});
        m_pipeline = std::exchange(other.m_pipeline, vk::Pipeline{});
    }

    GraphicPipeline& GraphicPipeline::operator=(GraphicPipeline&& other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();
        BaseGraphicPipiline::operator=(std::move(other));
        m_device = std::exchange(other.m_device, vk::Device{});
        m_pipelineLayout = std::exchange(other.m_pipelineLayout, vk::PipelineLayout{});
        m_pipeline = std::exchange(other.m_pipeline, vk::Pipeline{});

        return *this;
    }

    bool GraphicPipeline::build(Device& device)
    {
        if (!getRenderPassDescriptor())
            return false;

        m_device = device.getNativeDevice();
        if (m_device == VK_NULL_HANDLE)
            return false;

        const BinaryModule* vertexBinary = nullptr;

        const BinaryModule* fragmentBinary = nullptr;

        if (!getModule<ShaderStage::Vertex>(&vertexBinary) || !getModule<ShaderStage::Pixel>(&fragmentBinary))
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

            return device.getNativeDevice().createShaderModuleUnique(info);
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

        // 3. No vertex buffers
        vk::PipelineVertexInputStateCreateInfo vertexInput{};

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

        // 9. Pipeline layout (no descriptors)
        vk::PipelineLayoutCreateInfo layoutInfo{};

        m_pipelineLayout = device.getNativeDevice().createPipelineLayout(layoutInfo);

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

        m_pipeline = device.getNativeDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;

        return m_pipeline != VK_NULL_HANDLE;
    }

    void GraphicPipeline::destroy()
    {
        if (m_device)
        {
            m_device.destroyPipelineLayout(m_pipelineLayout);
            m_pipelineLayout = VK_NULL_HANDLE;

            m_device.destroyPipeline(m_pipeline);
            m_pipeline = VK_NULL_HANDLE;
        }
    }

} // namespace TiRHI::Vulkan
