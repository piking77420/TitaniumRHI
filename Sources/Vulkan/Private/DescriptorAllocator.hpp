#ifndef TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_ALLOCATOR_H
#define TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_ALLOCATOR_H

#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <Titanium/Private/StrongTypeId.hpp>
#include <Vulkan/Vulkan-Header.hpp>

namespace TiRHI::Vulkan
{
    class Device;
} // TiRHI::Vulkan

namespace TiRHI::Vulkan::Private
{
    struct DescriptorIndexTag
    {
    };
    struct PoolIndexTag
    {
    };

    using DesciptorIndex = TiRHI::Private::StrongTypeID<size_t, DescriptorIndexTag>;
    using PoolIndex = TiRHI::Private::StrongTypeID<size_t, PoolIndexTag>;

    class DescriporAllocator
    {
    public:
        DescritporAllocator() = default;
        ~DescritporAllocator() = default;

        bool build(Device& device);

    private:
        struct Pool
        {
            static constexpr uint64_t BaseCapacity = 64;

            vk::UniqueDescriptorPool descriptorPool;

            uint64_t m_availableMask; // 1 mean free 0 occupied

            std::array<size_t, BaseCapacity> generations;
        };

        std::vector<std::unique_ptr<Pool>> m_pools;
    };

} // TiRHI::Vulkan::Private

#endif // TITANIUM_VULKAN_PRIVATE_DESCRIPTOR_ALLOCATOR_H
