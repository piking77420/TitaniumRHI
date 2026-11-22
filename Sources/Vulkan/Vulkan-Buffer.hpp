#ifndef TITANIUM_VULKAN_BUFFER_H
#define TITANIUM_VULKAN_BUFFER_H

#include <type_traits>
#include <Titanium/RHI-BaseBuffer.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Private/StaginBuffer.hpp>

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

        vk::Buffer getVkBufferFrame();

        void setSubmissionValue(uint64_t timeLine);

        std::span<Vulkan::Private::AllocatedBuffer> getAllocatedBuffer()
        {
            return m_allocatedBuffer;
        }

        std::span<const Vulkan::Private::AllocatedBuffer> getAllocatedBuffer() const
        {
            return m_allocatedBuffer;
        }

    private:
        Device* m_device = nullptr;

        std::vector<Vulkan::Private::AllocatedBuffer> m_allocatedBuffer;

        bool build(Device& device, vk::Image image, Format format);

        void destroy();

        Vulkan::Private::AllocatedBuffer* getAllocatedBufferFrame();
    };
}

#endif // TITANIUM_VULKAN_BUFFER_H
