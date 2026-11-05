#ifndef TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_MANAGER_H
#define TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_MANAGER_H

#include <unordered_map>

#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <Titanium/Private/StrongTypeId.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Vulkan-DescriptorSet.hpp>

namespace TiRHI::Vulkan
{
    class Device;
} // TiRHI::Vulkan

namespace TiRHI::Vulkan::Private
{
    using BindingsLayout = Vulkan::DescriptorSet::BindingsLayout;

    class DescriporManager
    {
    public:
        DescriporManager() = default;
        ~DescriporManager() = default;

        static constexpr size_t CommandPoolCapacity = 64;

        using DescriptorSetHandle = TiRHI::Vulkan::Private::DescriptorSetHandle;

        std::optional<DescriptorSetHandle> allocAndWriteDescriptors(vk::Device device, const BindingsLayout& layout,
                                                                    size_t count);

        void release(const BindingsLayout& layout, DescriptorSetHandle& descriptorSetHandle);

    private:
        struct PoolCache
        {
            vk::UniqueDescriptorPool pool;
            size_t remainingSize = CommandPoolCapacity;
        };

        struct LayoutCache
        {
            vk::UniqueDescriptorSetLayout layout;
            std::vector<vk::DescriptorPoolSize> poolSize;
        };

        using DescriptorLayoutCache =
            std::unordered_map<BindingsLayout, LayoutCache, Vulkan::DescriptorSet::BaseType::BindingsLayoutHash,
                               Vulkan::DescriptorSet::BaseType::BindingsLayoutEqual>;

        using DescriptorPoolCache = std::unordered_map<BindingsLayout, std::vector<PoolCache>,
                                                       Vulkan::DescriptorSet::BaseType::BindingsLayoutHash,
                                                       Vulkan::DescriptorSet::BaseType::BindingsLayoutEqual>;

        vk::Device m_device = {};

        DescriptorLayoutCache m_descriptorLayoutMap;

        DescriptorPoolCache m_descriptorPoolMap;

        std::optional<std::tuple<DescriporManager::PoolCache*, DescriporManager::LayoutCache*,
                                 size_t>> // pool and index in vector cache
        tryGetOrCreateAllocator(vk::Device device, const BindingsLayout& layout, size_t count);

        DescriporManager::LayoutCache* vkLayoutFromRhiLayout(vk::Device device, const BindingsLayout& layout);

        void writeDescriptors(DescriptorSetHandle& handle, const BindingsLayout& layout, LayoutCache& layoutCache);
    };

} // TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_MANAGER_H
