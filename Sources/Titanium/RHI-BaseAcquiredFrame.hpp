#ifndef TITANIUM_BASE_ACQUIRE_FRAME_H
#define TITANIUM_BASE_ACQUIRE_FRAME_H

namespace TiRHI
{
    class BaseAcquiredFrame
    {
    public:
        explicit BaseAcquiredFrame() = delete;
        ~BaseAcquiredFrame() = default;
        explicit BaseAcquiredFrame(bool success);

        bool getSucces() const
        {
            return m_success;
        }

        bool operator()() const
        {
            return m_success;
        }

    private:
        bool m_success{false};
    };

} // TiRHI

#endif // TITANIUM_BASE_ACQUIRE_FRAME_H
