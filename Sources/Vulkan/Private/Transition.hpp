#ifndef TITANIUM_VULKAN_PRIVATE_TRANSITION_MAKER_H
#define TITANIUM_VULKAN_PRIVATE_TRANSITION_MAKER_H

#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan::Private
{
    [[nodiscard]] constexpr bool requiresBarrier(ResourceState from, ResourceState to)
    {
        return from != to || isWriteState(to);
    }

   struct TransitionData
    {
        vk::PipelineStageFlags srcPipelineStageFlag;
        vk::PipelineStageFlags dstPipelineStageFlag;

        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;

        vk::DependencyFlags dependencyFlags;
    };

   static constexpr vk::AccessFlags getAccessFlags(ResourceState state)
    {
        using Access = vk::AccessFlagBits;

        switch (state)
        {
        case ResourceState::Undefined:
        case ResourceState::Present:
            return {};

        case ResourceState::Common:
            return Access::eMemoryRead | Access::eMemoryWrite;

        case ResourceState::VertexBuffer:
            return Access::eVertexAttributeRead;

        case ResourceState::IndexBuffer:
            return Access::eIndexRead;

        case ResourceState::ConstantBuffer:
            return Access::eUniformRead;

        case ResourceState::ShaderResource:
            return Access::eShaderRead;

        case ResourceState::UnorderedAccess:
            return Access::eShaderRead | Access::eShaderWrite;

        case ResourceState::RenderTarget:
            return Access::eColorAttachmentRead | Access::eColorAttachmentWrite;

        case ResourceState::DepthWrite:
            return Access::eDepthStencilAttachmentRead | Access::eDepthStencilAttachmentWrite;

        case ResourceState::DepthRead:
            return Access::eDepthStencilAttachmentRead;

        case ResourceState::CopySource:
            return Access::eTransferRead;

        case ResourceState::CopyDestination:
            return Access::eTransferWrite;

        case ResourceState::IndirectArgument:
            return Access::eIndirectCommandRead;

        default:
            assert(false);
            return {};
        }
    }

    static constexpr vk::DependencyFlags dependencyFlagFromTo(ResourceState from, ResourceState to)
    {
        return {};
    }

    static constexpr TransitionData getTransitionData(ResourceState from, ResourceState to)
    {
        TransitionData transitionData;

        transitionData.srcPipelineStageFlag =
            from == ResourceState::Undefined ? vk::PipelineStageFlagBits::eTopOfPipe : Private::getPipelineStage(from);
        transitionData.dstPipelineStageFlag = Private::getPipelineStage(to);
        transitionData.dependencyFlags = dependencyFlagFromTo(from, to);
        transitionData.srcAccessMask = Private::getAccessFlags(from);
        transitionData.dstAccessMask = Private::getAccessFlags(to);

        return transitionData;
    }

    static consteval std::array<TransitionData, ResourceStateCount * ResourceStateCount> generateStateMap()
    {
        std::array<TransitionData, ResourceStateCount * ResourceStateCount> result{};

        for (size_t src = 0; src < ResourceStateCount; src++)
        {
            for (size_t dst = 0; dst < ResourceStateCount; dst++)
            {
                const ResourceState current = static_cast<ResourceState>(src);
                const ResourceState target = static_cast<ResourceState>(dst);

                result[getResourceCombinaisonIndex(current, target)] = getTransitionData(current, target);
            }
        }

        return result;
    }

    static constexpr std::array<TransitionData, ResourceStateCount * ResourceStateCount> transitionMap =
        generateStateMap();

} // namespace TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_TRANSITION_MAKER_H
