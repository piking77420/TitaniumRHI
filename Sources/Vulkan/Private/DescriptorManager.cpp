#include <Vulkan/Private/DescriptorManager.hpp>
#include <Titanium/Log.hpp>

#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Vulkan/Vulkan-DescriptorSet.hpp>

namespace TiRHI::Vulkan::Private
{
    std::optional<DescriporManager::DescriptorSetHandle>
    DescriporManager::allocDescriptors(vk::Device device, const BindingsLayout& layout, size_t count)
    {
        if (m_device == VK_NULL_HANDLE)
        {
            m_device = device;
        }
        else if (m_device != device)
        {
            RHI_LOG_FATAL(L"DescriptorManager: Cannot use multiple Vulkan devices.", RhiApi::Vulkan);
        }

        std::optional<std::tuple<DescriporManager::PoolCache*, DescriporManager::LayoutCache*, size_t>> cache =
            tryGetOrCreateAllocator(device, layout, count);

        if (!cache)
            return {};

        DescriporManager::PoolCache* poolCache = std::get<DescriporManager::PoolCache*>(*cache);
        DescriporManager::LayoutCache* layoutCache = std::get<DescriporManager::LayoutCache*>(*cache);

        // allocate from pool
        DescriptorSetHandle handle;
        handle.descriptorCount = count;
        handle.poolIndex = std::get<size_t>(*cache);

        std::array<vk::DescriptorSetLayout, MaxFramesInFlight> layouts{};
        layouts.fill(layoutCache->layout.get());

        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.setDescriptorPool(poolCache->pool.get())
            .setDescriptorSetCount(static_cast<uint32_t>(count))
            .setPSetLayouts(layouts.data());

        vk::Result result = device.allocateDescriptorSets(&allocInfo, handle.descriptorSets.data());
        if (result != vk::Result::eSuccess)
        {
            RHI_LOG_ERROR(L"Faile to allocated Descriptor set", RhiApi::Vulkan);
            return std::nullopt;
        }
        poolCache->remainingSize -= count;

        return handle;
    }

    void DescriporManager::release(const BindingsLayout& layout, DescriptorSetHandle& descriptorSetHandle)
    {
        // TODO CALL THIS IN DEFFERED DESTROY
        auto it = m_descriptorPoolMap.find(layout);
        if (it == m_descriptorPoolMap.end())
            return;

        auto& pools = it->second;
        if (descriptorSetHandle.poolIndex >= pools.size())
            return;

        if (m_device == VK_NULL_HANDLE)
            return;

        m_device.freeDescriptorSets(
            pools[descriptorSetHandle.poolIndex].pool.get(),
            std::span(descriptorSetHandle.descriptorSets.data(), descriptorSetHandle.descriptorCount));

        pools[descriptorSetHandle.poolIndex].remainingSize += descriptorSetHandle.descriptorCount;

        descriptorSetHandle = {}; // reset
    }

    std::optional<std::tuple<DescriporManager::PoolCache*, DescriporManager::LayoutCache*, size_t>>
    DescriporManager::tryGetOrCreateAllocator(vk::Device device, const Vulkan::DescriptorSet::BindingsLayout& layout,
                                              size_t count)
    {
        if (device == VK_NULL_HANDLE || count == 0 || count > MaxFramesInFlight || count > CommandPoolCapacity)
        {
            return std::nullopt;
        }

        // look for existing layout
        LayoutCache* layoutCache = vkLayoutFromRhiLayout(device, layout);
        if (layoutCache == nullptr)
            return {};

        auto& poolCache = m_descriptorPoolMap[layout];
        if (!poolCache.empty())
        {
            size_t findIndex = std::numeric_limits<size_t>::max();
            for (size_t i = 0; i < poolCache.size(); i++)
            {
                auto& pool = poolCache[i];
                if (pool.remainingSize >= count)
                {
                    findIndex = i;
                    break;
                }
            }

            if (findIndex != std::numeric_limits<size_t>::max())
            {
                return std::tuple<DescriporManager::PoolCache*, DescriporManager::LayoutCache*, size_t>(
                    &poolCache[findIndex], layoutCache, findIndex);
            }
        }

        // new pool
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.setMaxSets(DescriporManager::CommandPoolCapacity)
            .setPoolSizes(layoutCache->poolSize)
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

        vk::UniqueDescriptorPool pool = device.createDescriptorPoolUnique(descriptorPoolCreateInfo);
        if (!pool)
            return {};

        auto& cachedPool = poolCache.emplace_back();
        cachedPool.pool = std::move(pool);
        cachedPool.remainingSize = CommandPoolCapacity;

        return std::tuple<DescriporManager::PoolCache*, DescriporManager::LayoutCache*, size_t>(
            &cachedPool, layoutCache, (poolCache.size() - 1));
    }

    DescriporManager::LayoutCache* DescriporManager::vkLayoutFromRhiLayout(vk::Device device,
                                                                           const BindingsLayout& layout)
    {
        auto it = m_descriptorLayoutMap.find(layout);
        if (it != m_descriptorLayoutMap.cend())
        {
            return &it->second;
        }

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
                    if (vkBinding.descriptorCount > 0)
                    {
                        // TODO avoid to empalce back each time may combine DescriptorPoolSize
                        poolSize.emplace_back(
                            vkBinding.descriptorType,
                            vkBinding.descriptorCount * CommandPoolCapacity
                        );
                    }
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
            return nullptr;
        }

        vk::DescriptorSetLayout outLayout = vkLayout.get();
        auto& place = m_descriptorLayoutMap[layout];
        place = LayoutCache{.layout = std::move(vkLayout), .poolSize = std::move(poolSize)};
        return &place;
    }

} // TiRHI::Vulkan::Private
