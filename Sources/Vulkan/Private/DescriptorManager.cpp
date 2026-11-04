#include <Vulkan/Private/DescriptorManager.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Vulkan::Private
{
    std::optional<DescriporManager::Pool> createPool(const DescriporManager::VulkanLayoutPools& userData)
    {
        if (userData.device == VK_NULL_HANDLE)
            return {};
        DescriporManager::Pool pool;
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.setMaxSets(DescriporManager::CommandPoolCapacity)
            .setPoolSizes(userData.descriptorPoolSizes)
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

        pool.pool = userData.device.createDescriptorPoolUnique(descriptorPoolCreateInfo);

        if (!pool.pool)
        {
            return {};
        }

        return pool;
    }

    std::optional<vk::DescriptorSet> allocDescriptorSet(DescriporManager::Pool& pool, std::size_t pooInIndex,
                                                        const DescriporManager::VulkanLayoutPools& userData)
    {
        if (userData.device == VK_NULL_HANDLE || !pool.pool || !userData.layout)
            return {};

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.setDescriptorPool(pool.pool.get())
            .setDescriptorSetCount(1)
            .setSetLayouts(userData.layout.get());

        std::vector<vk::DescriptorSet> set = userData.device.allocateDescriptorSets(descriptorSetAllocateInfo);
        if (!set.empty())
            return {};

        assert(set.size() == 1);
        return set[0];
    }

    void releaseDescriptorSet(DescriporManager::Pool& pool, vk::DescriptorSet& descriptorSet,
                              const DescriporManager::VulkanLayoutPools& userData)
    { // TODO undle deffer destroy
        if (userData.device == VK_NULL_HANDLE || !pool.pool || !userData.layout)
            return;
        userData.device.freeDescriptorSets(pool.pool.get(), descriptorSet);
        descriptorSet = VK_NULL_HANDLE;
    }

    std::optional<DescriporManager::DescriptorAllocator::Handle>
    DescriporManager::createNewDescriptorSetHandle(vk::Device device,
                                                   const Vulkan::DescriptorSet::BindingsLayout& layout)
    {
        VulkanLayoutPools* vulkanLayoutPools = tryGetOrCreateAllocator(device, layout);

        if (!vulkanLayoutPools)
            return {};

        return vulkanLayoutPools->descriptorSetAllocator.create();
    }

    DescriporManager::VulkanLayoutPools*
    DescriporManager::tryGetOrCreateAllocator(vk::Device device, const Vulkan::DescriptorSet::BindingsLayout& layout)
    {
        auto it = m_map.find(layout);

        if (it != m_map.end())
            return it->second.get();

        // new
        std::unique_ptr<DescriporManager::VulkanLayoutPools> pool = vkLayoutFromRhiLayout(device, layout);

        if (!pool)
            return {};

        auto& ref = m_map[layout];
        ref = std::move(pool);
        return ref.get();
    }

    std::unique_ptr<DescriporManager::VulkanLayoutPools>
    DescriporManager::vkLayoutFromRhiLayout(vk::Device device, const Vulkan::DescriptorSet::BindingsLayout& layout)
    {
        std::unique_ptr<DescriporManager::VulkanLayoutPools> pool =
            std::make_unique<DescriporManager::VulkanLayoutPools>();

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        bindings.reserve(layout.bindings.size());

        std::vector<vk::DescriptorPoolSize> poolSize;
        poolSize.reserve(layout.bindings.size());

        for (const auto& binding : layout.bindings)
        {
            vk::DescriptorSetLayoutBinding vkBinding{};
            // clang-format off
            std::visit(
                [&](const auto& value)
                {
                    using T = std::remove_cvref_t<decltype(value)>;

                    vkBinding.binding = value.bindingIndex;
                    vkBinding.descriptorCount = value.count;
                    vkBinding.stageFlags = Private::toVulkan(value.shaderStageFlag);
                    if constexpr (std::is_same_v<T, Vulkan::DescriptorSet::SampledTexture>)
                    {
                        vkBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                    }
                    else
                    {
                        static_assert(false);
                    }

                    // TODO avoid to empalce back each time may combine DescriptorPoolSize
                    poolSize.emplace_back(
                        vkBinding.descriptorType,
                        vkBinding.descriptorCount * CommandPoolCapacity
                    );
                },
                binding);

            // clang-format on
            bindings.emplace_back(vkBinding);
        }

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        // clang-format off
        descriptorSetLayoutCreateInfo
            .setBindings(bindings)
            .setBindingCount(static_cast<uint32_t>(bindings.size()))
            .setFlags({});
        // clang-format on
        vk::UniqueDescriptorSetLayout vkLayout = device.createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);

        if (!vkLayout)
        {
            RHI_LOG_ERROR(L"Failed to create UniqueDescriptorSetLayout", RhiApi::Vulkan);

            return {};
        }

        if (!pool->descriptorSetAllocator.build(&createPool, &allocDescriptorSet, &releaseDescriptorSet, pool.get()))
        {
            return {};
        }

        return pool;
    }

} // TiRHI::Vulkan::Private
