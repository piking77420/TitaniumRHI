#ifndef TITANIUM_PRIVATE_FRAME_TRACKED_H
#define TITANIUM_PRIVATE_FRAME_TRACKED_H

#include <cstdint>

namespace TiRHI::Private
{
    template<typename T>
    class FrameTracked
    {
    public:
        uint64_t getLastFrameUsed() const noexcept
        {
            return m_lastFrameUsed;
        }

        T& setLastFrameUsed(uint64_t value) noexcept
        {
            m_lastFrameUsed = value;
            return static_cast<T&>(*this);
        }

    private:
        uint64_t m_lastFrameUsed = 0;
    };
}

#endif // TITANIUM_PRIVATE_FRAME_TRACKED_H
