#include <Vulkan/Vulkan-Buffer.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Titanium/Private/DeferredDeletionQueue.hpp>

namespace TiRHI::Vulkan
{
    Buffer ::~Buffer()
    {
        destroy();
    };

    Buffer::Buffer(Buffer&& other) noexcept
        : BaseBuffer(std::move(other))
        , m_allocatedBuffer(std::exchange(other.m_allocatedBuffer, {}))
    {
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();
        BaseBuffer::operator=(std::move(other));
        m_allocatedBuffer = std::exchange(other.m_allocatedBuffer, {});

        return *this;
    }

    bool Buffer::build()
    {
        if (getSize() == 0 || getUsage() == BufferUsage::None)
            return false;

        VmaAllocator& allocator = Device::Private::getAllocator(getDevice());
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = getSize();
        bufferInfo.usage = static_cast<VkBufferUsageFlags>(Private::toVulkan(getUsage()));
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

        switch (getMemoryType())
        {
        case BufferMemoryType::Default:
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            break;

        case BufferMemoryType::Upload:
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;

        case BufferMemoryType::Readback:
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        default:
            RHI_LOG_FATAL(std::format(L"Buffer : {}, Unknow Buffer memory Type", getNameW()), Api);
            break;
        }

        m_allocatedBuffer.resize(m_updateMode == BufferUpdateMode::Static ? 1 : getRHI().getFrameInFlight());
        for (auto& allocatedBuffer : m_allocatedBuffer)
        {
            const vk::Result vkResult = static_cast<vk::Result>(vmaCreateBuffer(
                allocator, &bufferInfo, &allocInfo, reinterpret_cast<VkBuffer*>(&allocatedBuffer.buffer),
                &allocatedBuffer.allocation, &allocatedBuffer.allocationInfo));

            if (VulkanCheckErrorStatus(vkResult))
                return false;

            vk::DebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.objectType = vk::ObjectType::eBuffer;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkBuffer>(allocatedBuffer.buffer));
            nameInfo.pObjectName = getName().data();

            getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);
        }

        return true;
    }

    std::byte* Buffer::beginDynamicUpdate()
    {
        if (getSize() == 0)
            return nullptr;

        if (getMemoryType() != BufferMemoryType::Upload)
        {
            RHI_LOG_ERROR(
                std::format(L"Buffer {}, to dynamic update an buffer the buffer should use BufferMemoryType::Upload",
                            getNameW()),
                Api);

            return nullptr;
        }

        Vulkan::Private::AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();
        if (!allocatedBuffer)
            return nullptr;

        VmaAllocator& allocator = Device::Private::getAllocator(getDevice());

        void* mapped = nullptr;

        const vk::Result result =
            static_cast<vk::Result>(vmaMapMemory(allocator, allocatedBuffer->allocation, &mapped));

        return !VulkanCheckErrorStatus(result) ? static_cast<std::byte*>(mapped) : nullptr;
    }

    void Buffer::endDynamicUpdate()
    {
        if (getSize() == 0)
            return;

        Vulkan::Private::AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();
        if (!allocatedBuffer)
            return;

        VmaAllocator& allocator = Device::Private::getAllocator(getDevice());

        vmaFlushAllocation(allocator, allocatedBuffer->allocation, 0, m_size);

        vmaUnmapMemory(allocator, allocatedBuffer->allocation);
    }

    uint32_t Buffer::getFrameAllocationIndex() const
    {
        const size_t index = getUpdateMode() == BufferUpdateMode::Static ? 0 : getRHI().getCurrentFrame();

        return index;
    }

    std::uint64_t Buffer::getLastFrameUsed(uint32_t frameIndex) const noexcept
    {
        assert(frameIndex < m_allocatedBuffer.size());
        if (frameIndex >= m_allocatedBuffer.size()) [[unlikely]]
        {
            RHI_LOG_FATAL(std::format(L"Invalid access to mark the Buffer set {}", getNameW()), Api);
        }

        return m_allocatedBuffer[frameIndex].getLastFrameUsed();
    }

    Buffer& Buffer::setLastFrameUsed(std::uint64_t value, uint32_t frameIndex) noexcept
    {
        assert(frameIndex < m_allocatedBuffer.size());
        if (frameIndex >= m_allocatedBuffer.size()) [[unlikely]]
        {
            RHI_LOG_FATAL(std::format(L"Invalid access to mark the Buffer set {}", getNameW()), Api);
        }

        m_allocatedBuffer[frameIndex].setLastFrameUsed(value);
        return *this;
    }

    void Buffer::destroy()
    {
        TiRHI::Private::DeferredDeletionQueue& defferdDeferredDeletionQueue =
            Device::Private::getDeferredDeletionQueue(getDevice());

        for (size_t i = 0; i < m_allocatedBuffer.size(); i++)
        {
            auto& allocatedBuffer = m_allocatedBuffer[i];
            if (allocatedBuffer.allocation)
            {
                defferdDeferredDeletionQueue.retire(allocatedBuffer.getLastFrameUsed(),
                                                    [allocator = Device::Private::getAllocator(getDevice()),
                                                     allocatedBuffer = std::move(allocatedBuffer)]()
                                                    {
                                                        vmaDestroyBuffer(allocator,
                                                                         static_cast<VkBuffer>(allocatedBuffer.buffer),
                                                                         allocatedBuffer.allocation);
                                                    });
            }
        }

        m_allocatedBuffer.clear();
    }

    vk::Buffer Buffer::getVkBufferFrame()
    {
        Vulkan::Private::AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();

        return allocatedBuffer ? allocatedBuffer->buffer : VK_NULL_HANDLE;
    }

    Vulkan::Private::AllocatedBuffer* Buffer::getAllocatedBufferFrame()
    {
        const size_t index = getUpdateMode() == BufferUpdateMode::Static ? 0 : getRHI().getCurrentFrame();

        return m_allocatedBuffer.size() >= (index + 1) ? &m_allocatedBuffer[index] : nullptr;
    }

} // namespace TiRHI::Vulkan
