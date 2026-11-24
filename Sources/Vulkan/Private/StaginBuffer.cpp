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
        : AllocatedBuffer(std::move(other))
        , m_deferredDeletionQueue(other.m_deferredDeletionQueue)
        , m_allocator(std::exchange(other.m_allocator, nullptr))
        , m_mappedData(std::exchange(other.m_mappedData, nullptr))
        , m_size(std::exchange(other.m_size, 0))
    {
    }

    StagingBuffer::StagingBuffer(Device& device, uint64_t size)
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

        VmaAllocationInfo allocationInfo{};
        const VkResult result = vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo,
                                                reinterpret_cast<VkBuffer*>(&buffer), &allocation, &allocationInfo);

        if (result != VK_SUCCESS)
        {
            m_size = 0;
            return;
        }
    }

    std::byte* StagingBuffer::mapData() noexcept
    {
        if (m_mappedData)
            return m_mappedData;

        void* mapped = nullptr;

        const VkResult result = vmaMapMemory(m_allocator, allocation, &mapped);

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

        const VkResult result = vmaFlushAllocation(m_allocator, allocation, 0, VK_WHOLE_SIZE);
        VulkanCheckErrorStatus(result);

        vmaUnmapMemory(m_allocator, allocation);
        m_mappedData = nullptr;
    }

    void StagingBuffer::reset()
    {
        if (m_mappedData)
            unmapData();

        if (buffer && allocation)
        {
            const uint64_t retireValue = getLastFrameUsed();
            m_deferredDeletionQueue.retire(
                retireValue, [allocator = m_allocator, cBuffer = std::move(buffer), cAllocation = std::move(allocation)]()
                                           { vmaDestroyBuffer(allocator, cBuffer, cAllocation); });
            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }

        m_allocator = nullptr;
        m_size = 0;
    }
}
