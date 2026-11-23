#include <Vulkan/Private/StaginBuffer.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Titanium/Private/DeferredDeletionQueue.hpp>

namespace TiRHI::Vulkan::Private
{
    StagingBuffer::~StagingBuffer()
    {
        reset();
    }

    StagingBuffer::StagingBuffer(StagingBuffer&& other) noexcept
        : m_deferredDeletionQueue(other.m_deferredDeletionQueue)
        , m_allocatedBuffer(std::exchange(other.m_allocatedBuffer, {}))
        , m_mappedData(std::exchange(other.m_mappedData, nullptr))
        , m_size(std::exchange(other.m_size, 0))
    {
    }

    StagingBuffer::StagingBuffer(Device& device, uint64_t size, uint64_t timeLineFrame)
        : m_deferredDeletionQueue(Device::Private::getDeferredDeletionQueue(device))
        , m_allocator(Device::Private::getAllocator(device))
        , m_size(size)
    {
        if (size == 0)
            return;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBuffer nativeBuffer = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;

        VmaAllocationInfo allocationInfo{};
        const VkResult result =
            vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &nativeBuffer, &allocation, &allocationInfo);

        if (result != VK_SUCCESS)
        {
            m_size = 0;
            return;
        }

        m_allocatedBuffer.buffer = vk::Buffer(nativeBuffer);
        m_allocatedBuffer.allocation = allocation;
        m_allocatedBuffer.allocationInfo = allocationInfo;
        m_allocatedBuffer.submissionValue = timeLineFrame;
    }

    std::byte* StagingBuffer::mapData() noexcept
    {
        if (!m_allocatedBuffer)
            return nullptr;

        if (m_mappedData)
            return m_mappedData;

        void* mapped = nullptr;

        const VkResult result = vmaMapMemory(m_allocator, m_allocatedBuffer.allocation, &mapped);

        if (result != VK_SUCCESS)
        {
            RHI_LOG_FATAL(std::format(L"Failed to create staging buffer"), RhiApi::Vulkan);
            return nullptr;
        }

        m_mappedData = static_cast<std::byte*>(mapped);
        return m_mappedData;
    }

    void StagingBuffer::unmapData()
    {
        if (!m_mappedData)
            return;

        const VkResult result = vmaFlushAllocation(m_allocator, m_allocatedBuffer.allocation, 0, VK_WHOLE_SIZE);
        VulkanCheckErrorStatus(result);

        vmaUnmapMemory(m_allocator, m_allocatedBuffer.allocation);
        m_mappedData = nullptr;
    }

    void StagingBuffer::reset()
    {
        if (m_mappedData)
            unmapData();

        if (m_allocatedBuffer)
        {
            const uint64_t retireValue = m_allocatedBuffer.submissionValue;
            m_deferredDeletionQueue.retire(
                retireValue, [allocator = m_allocator, allocatedBuffer = std::move(m_allocatedBuffer)]()
                { vmaDestroyBuffer(allocator, allocatedBuffer.buffer, allocatedBuffer.allocation); });
        }

        m_allocator = nullptr;
        m_size = 0;
    }
}
