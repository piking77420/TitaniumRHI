#ifndef TITANIUM_METAL_ACQUIRE_FRAME_H
#define TITANIUM_METAL_ACQUIRE_FRAME_H

#include <Titanium/RHI-BaseAcquiredFrame.hpp>

namespace TiRHI::Metal
{
    class RHI;
    
    class AcquiredFrame : public BaseAcquiredFrame
    {
    public:
        explicit AcquiredFrame() = delete;
        ~AcquiredFrame() = default;
        explicit AcquiredFrame(bool success);

        bool getSucces() const
        {
            return BaseAcquiredFrame::getSucces();
        }

        explicit operator bool() const
        {
            return BaseAcquiredFrame::operator bool();
        }

    private:
    };
    
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_ACQUIRE_FRAME_H