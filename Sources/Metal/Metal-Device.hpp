#ifndef TITANIUM_METAL_DEVICE_H
#define TITANIUM_METAL_DEVICE_H

#include <Titanium/RHI-BaseDevice.hpp>

namespace TiRHI::Metal
{
    class RHI;

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

    private:
    };

}

#endif // TITANIUM_METAL_DEVICE_H
