#include <Vulkan/Private/UploadManager.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::Vulkan::Private
{

    StagingBuffer createStagingBuffer(vk::Device device, VmaAllocator vmaAllocator, VkDeviceSize size)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo resultInfo{};
        VkBuffer buffer;
        VmaAllocation VmaAllocation;

        if (vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocationInfo, &buffer, &VmaAllocation, &resultInfo) !=
            VK_SUCCESS)
        {
            RHI_LOG_FATAL(L"Failed to allocate Staging Buffer", TiRHI::RhiApi::Vulkan);
            return {};
        }

        return StagingBuffer{vmaAllocator, vk::Buffer(buffer), VmaAllocation, size};
    }

    StagingBuffer UploadManager::acquire(Device& device, VkDeviceSize size)
    {
        // get the one closest to the user size
        decltype(m_available)::iterator minIterator = m_available.end();
        VkDeviceSize minValue = std::numeric_limits<VkDeviceSize>::max();

        for (auto it = m_available.begin(); it != m_available.end(); ++it)
        {
            const VkDeviceSize capacity = it->getCapacity();

            if (capacity >= size && capacity < minValue)
            {
                minIterator = it;
                minValue = capacity;
                if (capacity == size)
                    break;
            }
        }

        if (minIterator != m_available.end())
        {
            StagingBuffer result = std::move(*minIterator);
            m_available.erase(minIterator);
            return result;
        }

        return createStagingBuffer(device.getNativeDevice(), Device::Private::getAllocator(device), size);
    }

    void UploadManager::retire(UploadBatch batch, uint64_t submissionValue)
    {
        if (batch.buffers.empty())
            return;

        m_inFlight.push_back({submissionValue, std::move(batch)});
    }

    void UploadManager::collect(Device& device, uint64_t completedValue)
    {
        auto it = m_inFlight.begin();

        while (it != m_inFlight.end())
        {
            if (it->retireValue > completedValue)
            {
                ++it;
                continue;
            }

            for (auto& buffer : it->batch.buffers)
                m_available.push_back(std::move(buffer));

            it = m_inFlight.erase(it);
        }
    }

} // TiRHI::Vulkan::Private
