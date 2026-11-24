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

    class Buffer : public BaseBuffer<Buffer, Device>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;
        Buffer() = delete;
        ~Buffer();
        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;
        Buffer(Device& device)
            : BaseBuffer<Buffer, Device>(device)
        {
        }

        bool build();

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

        std::span<Vulkan::Private::AllocatedBuffer> getAllocatedBuffer()
        {
            return m_allocatedBuffer;
        }

        std::span<const Vulkan::Private::AllocatedBuffer> getAllocatedBuffer() const
        {
            return m_allocatedBuffer;
        }

        uint32_t getFrameAllocationIndex() const; 

        std::uint64_t getLastFrameUsed(uint32_t frameIndex) const noexcept;

        Buffer& setLastFrameUsed(std::uint64_t value, uint32_t frameIndex) noexcept;

    private:
        std::vector<Vulkan::Private::AllocatedBuffer> m_allocatedBuffer;

        bool build(Device& device, vk::Image image, Format format);

        void destroy();

        Vulkan::Private::AllocatedBuffer* getAllocatedBufferFrame();
    };
}

#endif // TITANIUM_VULKAN_BUFFER_H
