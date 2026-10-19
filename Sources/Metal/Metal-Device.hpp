#ifndef TITANIUM_METAL_DEVICE_H
#define TITANIUM_METAL_DEVICE_H

#include <Titanium/RHI-BaseDevice.hpp>

namespace TiRHI::Metal
{
    class RHI;
    class Surface;
    class Adapters;
    class AcquiredFrame;
    class CommandList;

    class Device : public BaseDevice<Device, RHI>
    {
    public:
        Device();
        ~Device();
        RHI_MOVE_ONLY(Device)
        Device(RHI& rhi)
            : BaseDevice(rhi)
        {
        }

        void beginFrame();

        bool build(RHI& rhi, const Surface& surface, const Adapter& adapter, std::optional<size_t> index);

        void wait();

        void submit(std::span<const AcquiredFrame> acquiredFrames,
                    std::span<CommandList*> cmdLists);

        bool isValid();

    private:
    };

}

#endif // TITANIUM_METAL_DEVICE_H
