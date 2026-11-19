#include <Vulkan/Vulkan-Buffer.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>
#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI::Vulkan
{
    Buffer ::~Buffer() = default;

    Buffer::Buffer(Buffer&& other) noexcept
        : BaseBuffer(std::move(other))
        , m_device(std::exchange(other.m_device, nullptr))
        , m_allocatedBuffer(std::exchange(other.m_allocatedBuffer, {}))
    {
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();
        BaseBuffer::operator=(std::move(other));
        m_device = std::exchange(other.m_device, nullptr);
        m_allocatedBuffer = std::exchange(other.m_allocatedBuffer, {});

        return *this;
    }

    bool Buffer::build(Device& device)
    {
        if (getSize() == 0 || getUsage() == BufferUsage::None)
            return false;
        m_device = &device;

        VmaAllocator& allocator = Device::Private::getAllocator(*m_device);
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

        if (!m_device)
        {
            return nullptr;
        }

        AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();
        if (!allocatedBuffer)
            return nullptr;

        VmaAllocator& allocator = Device::Private::getAllocator(*m_device);

        void* mapped = nullptr;

        const vk::Result result =
            static_cast<vk::Result>(vmaMapMemory(allocator, allocatedBuffer->allocation, &mapped));

        return !VulkanCheckErrorStatus(result) ? static_cast<std::byte*>(mapped) : nullptr;
    }

    void Buffer::endDynamicUpdate()
    {
        if (getSize() == 0)
            return;

        if (!m_device)
        {
            return;
        }

        AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();
        if (!allocatedBuffer)
            return;

        VmaAllocator& allocator = Device::Private::getAllocator(*m_device);

        vmaFlushAllocation(allocator, allocatedBuffer->allocation, 0, m_size);

        vmaUnmapMemory(allocator, allocatedBuffer->allocation);
    }

    void Buffer::destroy()
    {
        for (auto& allocatedBuffer : m_allocatedBuffer)
        {
            if (allocatedBuffer.allocation && m_device)
            {
                vmaDestroyBuffer(Device::Private::getAllocator(*m_device),
                                 static_cast<VkBuffer>(allocatedBuffer.buffer), allocatedBuffer.allocation);
            }
            allocatedBuffer.buffer = vk::Buffer{};
            allocatedBuffer.allocation = nullptr;
            allocatedBuffer.allocationInfo = {};
        }
    }

    vk::Buffer Buffer::getFrameVkBuffer()
    {
        AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();

        return allocatedBuffer ? allocatedBuffer->buffer : VK_NULL_HANDLE;
    }

    Buffer::AllocatedBuffer* Buffer::getAllocatedBufferFrame()
    {
        const size_t index = getUpdateMode() == BufferUpdateMode::Static ? 0 : getRHI().getCurrentFrame();

        return m_allocatedBuffer.size() >= (index + 1) ? &m_allocatedBuffer[index] : nullptr;
    }

} // namespace TiRHI::Vulkan
