#ifndef TITANIUM_VULKAN_PRIVATE_FRAME_TRACKED_H
#define TITANIUM_VULKAN_PRIVATE_FRAME_TRACKED_H

#include <cstdint>

namespace TiRHI::Vulkan::Private
{
    template<typename T>
    class FrameTracked
    {
    public:
        std::uint64_t getLastFrameUsed() const noexcept
        {
            return m_lastFrameUsed;
        }

        T& setLastFrameUsed(std::uint64_t value) noexcept
        {
            m_lastFrameUsed = value;
            return static_cast<T&>(*this);
        }

    private:
        std::uint64_t m_lastFrameUsed = 0;
    };
}

#endif // TITANIUM_VULKAN_PRIVATE_FRAME_TRACKED_H
