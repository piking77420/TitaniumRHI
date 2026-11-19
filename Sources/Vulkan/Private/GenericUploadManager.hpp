#ifndef TITANIUM_VULKAN_PRIVATE_GENERIC_UPLOAD_MANAGER_H
#define TITANIUM_VULKAN_PRIVATE_GENERIC_UPLOAD_MANAGER_H

#include <Vulkan/Vulkan-Header.hpp>
#include <vma/vk_mem_alloc.h>
#include <Titanium/Private/UploadManager.hpp>

namespace TiRHI::Vulkan
{
    class Device;
} // TiRHI::Vulkan

namespace TiRHI::Vulkan::Private
{
    class StagingBuffer
    {
    public:
        StagingBuffer() = default;

        StagingBuffer(VmaAllocator allocator, vk::Buffer buffer, VmaAllocation allocation,
                      vk::DeviceSize capacity) noexcept
            : m_buffer(buffer)
            , m_alloc(allocation)
            , m_capacity(capacity)
            , m_allocator(allocator)
        {
        }

        ~StagingBuffer()
        {
            reset();
        }

        StagingBuffer(const StagingBuffer&) = delete;
        StagingBuffer& operator=(const StagingBuffer&) = delete;

        StagingBuffer(StagingBuffer&& other) noexcept
            : m_buffer(std::exchange(other.m_buffer, vk::Buffer{}))
            , m_alloc(std::exchange(other.m_alloc, nullptr))
            , m_capacity(std::exchange(other.m_capacity, 0))
            , m_allocator(std::exchange(other.m_allocator, nullptr))
        {
        }

        StagingBuffer& operator=(StagingBuffer&& other) noexcept
        {
            if (this == &other)
                return *this;

            reset();

            m_buffer = std::exchange(other.m_buffer, vk::Buffer{});
            m_alloc = std::exchange(other.m_alloc, nullptr);
            m_capacity = std::exchange(other.m_capacity, 0);
            m_allocator = std::exchange(other.m_allocator, nullptr);

            return *this;
        }

        void reset() noexcept
        {
            if (m_buffer)
            {
                vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(m_buffer), m_alloc);
            }

            m_buffer = vk::Buffer{};
            m_alloc = nullptr;
            m_capacity = 0;
            m_allocator = nullptr;
        }

        vk::Buffer getBuffer() const noexcept
        {
            return m_buffer;
        }

        VmaAllocation getAllocation() const noexcept
        {
            return m_alloc;
        }

        vk::DeviceSize getCapacity() const noexcept
        {
            return m_capacity;
        }

        VmaAllocator getAllocator() const
        {
            return m_allocator;
        }

        operator bool() const
        {
            return m_buffer && m_alloc && (m_capacity != 0) && m_allocator;
        }

    private:
        vk::Buffer m_buffer{};
        VmaAllocation m_alloc = nullptr;
        vk::DeviceSize m_capacity = 0;
        VmaAllocator m_allocator = nullptr;
    };

    struct VulkanStagingFactory
    {
        VmaAllocator allocator;

        StagingBuffer operator()(uint64_t size) const
        {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = static_cast<VkDeviceSize>(size);
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocationInfo{};
            allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocationInfo.flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo resultInfo{};
            VkBuffer buffer;
            VmaAllocation VmaAllocation;

            if (vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &buffer, &VmaAllocation, &resultInfo) !=
                VK_SUCCESS)
            {
                RHI_LOG_FATAL(L"Failed to allocate Staging Buffer", TiRHI::RhiApi::Vulkan);
                return {};
            }

            return StagingBuffer{allocator, vk::Buffer(buffer), VmaAllocation, size};
        }
    };

    using VulkanUploadManager = ::TiRHI::Private::UploadManager<StagingBuffer, VulkanStagingFactory>;
} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_PRIVATE_GENERIC_UPLOAD_MANAGER_H
