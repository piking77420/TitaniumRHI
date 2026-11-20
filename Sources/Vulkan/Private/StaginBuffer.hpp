#ifndef TITANIUM_DIRECTX12_PRIVATE_VULKAN_STAGING_BUFFER_H
#define TITANIUM_DIRECTX12_PRIVATE_VULKAN_STAGING_BUFFER_H

#include <vector>
#include <Vulkan/Vulkan-Header.hpp>
#include <vma/vk_mem_alloc.h>

namespace TiRHI::Private
{
    class DeferredDeletionQueue;
}

namespace TiRHI::Vulkan
{
    class Device;
}

namespace TiRHI::Vulkan::Private
{
    struct AllocatedBuffer
    {
        vk::Buffer buffer{};
        VmaAllocation allocation = VK_NULL_HANDLE;
        VmaAllocationInfo allocationInfo{};

        AllocatedBuffer() = default;
        ~AllocatedBuffer() = default;

        AllocatedBuffer(const AllocatedBuffer&) = delete;
        AllocatedBuffer& operator=(const AllocatedBuffer&) = delete;

        AllocatedBuffer(AllocatedBuffer&& other) noexcept
            : buffer(std::exchange(other.buffer, vk::Buffer{}))
            , allocation(std::exchange(other.allocation, VK_NULL_HANDLE))
            , allocationInfo(std::exchange(other.allocationInfo, VmaAllocationInfo{}))
        {
        }

        AllocatedBuffer& operator=(AllocatedBuffer&& other) noexcept
        {
            if (this == &other)
                return *this;

            buffer = std::exchange(other.buffer, vk::Buffer{});
            allocation = std::exchange(other.allocation, VK_NULL_HANDLE);
            allocationInfo = std::exchange(other.allocationInfo, VmaAllocationInfo{});

            return *this;
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(buffer) && allocation != VK_NULL_HANDLE;
        }
    };

    class StagingBuffer
    {
    public:
        StagingBuffer() = delete;
        ~StagingBuffer();

        StagingBuffer(const StagingBuffer&) = delete;
        StagingBuffer& operator=(const StagingBuffer&) = delete;

        StagingBuffer(StagingBuffer&& other) noexcept;
        StagingBuffer& operator=(StagingBuffer&& other) noexcept = delete;

        StagingBuffer(Device& device, uint64_t size, uint64_t timeLineFrame);

        vk::Buffer getBuffer() const noexcept
        {
            return m_allocatedBuffer ? m_allocatedBuffer.buffer : VK_NULL_HANDLE;
        }

        std::byte* mapData() noexcept;

        void unmapData();

        uint64_t getSize() const noexcept
        {
            return m_size;
        }

        explicit operator bool() const noexcept
        {
            return m_allocatedBuffer.operator bool();
        }

    private:
        TiRHI::Private::DeferredDeletionQueue& m_deferredDeletionQueue;
        VmaAllocator m_allocator;
        AllocatedBuffer m_allocatedBuffer;
        std::byte* m_mappedData = nullptr;
        uint64_t m_size = 0;
        uint64_t m_submissionValue = 0;

        void reset();
    };
}

#endif // TITANIUM_DIRECTX12_PRIVATE_VULKAN_STAGING_BUFFER_H
