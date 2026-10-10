#ifndef TITANIUM_VULKAN_PRIVATE_TRANSITION_MAKER_H
#define TITANIUM_VULKAN_PRIVATE_TRANSITION_MAKER_H

#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan::Private
{
    struct TransitionData
    {
        vk::PipelineStageFlags srcPipelineStageFlag;
        vk::PipelineStageFlags dstPipelineStageFlag;
    };

    static constexpr TransitionData getTransitionData(ResourceState from, ResourceState to)
    {
        TransitionData transitionData;

        transitionData.srcPipelineStageFlag =
            from == ResourceState::Undefined ? vk::PipelineStageFlagBits::eTopOfPipe : Private::getPipelineStage(from);
        transitionData.dstPipelineStageFlag = Private::getPipelineStage(to);

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
