#ifndef TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_MANAGER_H
#define TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_MANAGER_H

#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <Titanium/Private/StrongTypeId.hpp>
#include <Titanium/Private/HandleAllocator.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Vulkan-DescriptorSet.hpp>

namespace TiRHI::Vulkan
{
    class Device;
} // TiRHI::Vulkan

namespace TiRHI::Vulkan::Private
{
    class DescriporManager
    {
    public:
        DescriporManager() = default;
        ~DescriporManager() = default;

        static constexpr size_t CommandPoolCapacity = 64;

        struct Pool
        {
            vk::UniqueDescriptorPool pool;
        };

        struct VulkanLayoutPools;
        // TODO may add more then one DescriptorSet per native handle
        using DescriptorAllocator =
            TiRHI::Private::HandleAllocator<Pool, vk::DescriptorSet, VulkanLayoutPools, CommandPoolCapacity>;

        struct VulkanLayoutPools
        {
            vk::UniqueDescriptorSetLayout layout;
            DescriptorAllocator descriptorSetAllocator;
            vk::Device device;
            std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
        };

        std::optional<DescriptorAllocator::Handle>
        createNewDescriptorSetHandle(vk::Device device, const Vulkan::DescriptorSet::BindingsLayout& layout);

    private:
        using DescriptorLayoutMap =
            std::unordered_map<Vulkan::DescriptorSet::BindingsLayout, std::unique_ptr<VulkanLayoutPools>,
                               Vulkan::DescriptorSet::BindingsLayoutHash, Vulkan::DescriptorSet::BindingsLayoutEqual>;

        DescriptorLayoutMap m_map;

        VulkanLayoutPools* tryGetOrCreateAllocator(vk::Device device,
                                                   const Vulkan::DescriptorSet::BindingsLayout& layout);

        std::unique_ptr<DescriporManager::VulkanLayoutPools>
        vkLayoutFromRhiLayout(vk::Device device, const Vulkan::DescriptorSet::BindingsLayout& layout);
    };

} // TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_MANAGER_H
