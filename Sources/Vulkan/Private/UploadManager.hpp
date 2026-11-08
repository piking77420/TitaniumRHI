#ifndef TITANIUM_VULKAN_PRIVATE_DEFFERED_UPLOAD_MANAGER_H
#define TITANIUM_VULKAN_PRIVATE_DEFFERED_UPLOAD_MANAGER_H

#include <functional>
#include <queue>
#include <Vulkan/Vulkan-Header.hpp>
#include <vma/vk_mem_alloc.h>

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

    struct UploadBatch
    {
        UploadBatch() = default;
        ~UploadBatch() = default;

        UploadBatch(const UploadBatch&) = delete;
        UploadBatch& operator=(const UploadBatch&) = delete;

        UploadBatch(UploadBatch&&) noexcept = default;
        UploadBatch& operator=(UploadBatch&&) noexcept = default;

        std::vector<StagingBuffer> buffers;
    };

    class UploadManager
    {
    public:
        UploadManager() = default;
        ~UploadManager() = default;

        StagingBuffer acquire(Device& device, VkDeviceSize size);

        void retire(UploadBatch batch, uint64_t submissionValue);

        void collect(Device& device, uint64_t completedValue);

    private:
        struct InFlightBatch
        {
            uint64_t retireValue;
            UploadBatch batch;
        };

        std::vector<StagingBuffer> m_available;
        std::vector<InFlightBatch> m_inFlight;
    };

} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_PRIVATE_DEFFERED_UPLOAD_MANAGER_H
