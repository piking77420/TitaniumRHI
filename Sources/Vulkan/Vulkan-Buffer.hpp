#ifndef TITANIUM_VULKAN_BUFFER_H
#define TITANIUM_VULKAN_BUFFER_H

#include <type_traits>
#include <Titanium/RHI-BaseBuffer.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <vma/vk_mem_alloc.h>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Buffer : public BaseBuffer<Buffer, Device, RHI>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;
        Buffer() = delete;
        ~Buffer();
        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;
        Buffer(RHI& rhi)
            : BaseBuffer<Buffer, Device, RHI>(rhi)
        {
        }

        bool build(Device& device);

        bool isValid() const
        {
            return !m_allocatedBuffer.empty();
        }

        bool isInvalid() const
        {
            return !isValid();
        }

        operator bool() const
        {
            return isValid();
        }

        std::byte* beginDynamicUpdate();

        void endDynamicUpdate();

        vk::Buffer getFrameVkBuffer();

    private:
        struct AllocatedBuffer
        {
            vk::Buffer buffer = VK_NULL_HANDLE;
            VmaAllocation allocation = VK_NULL_HANDLE;
            VmaAllocationInfo allocationInfo{};
        };

        Device* m_device = nullptr;

        std::vector<AllocatedBuffer> m_allocatedBuffer;

        bool build(Device& device, vk::Image image, Format format);

        void destroy();

        AllocatedBuffer* getAllocatedBufferFrame();
    };
}

#endif // TITANIUM_VULKAN_BUFFER_H
