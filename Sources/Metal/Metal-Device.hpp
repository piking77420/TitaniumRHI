#ifndef TITANIUM_METAL_DEVICE_H
#define TITANIUM_METAL_DEVICE_H

#include <vector>
#include <Titanium/RHI-BaseDevice.hpp>
#include <Metal/Metal-Header.hpp>

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
        Device() = delete;
        ~Device();
        RHI_MOVE_ONLY(Device)
        Device(RHI& rhi);
        
        struct Private
        {
            static MTL::Device* getDevice(Device& device)
            {
                return device.m_device;
            }
        };

        void beginFrame();

        bool build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                   std::optional<size_t> index = {});

        void wait();

        void submit(std::span<const AcquiredFrame> acquiredFrames, std::span<CommandList*> cmdLists);

        bool isValid();

    private:
        friend Private;

        MTL::Device* m_device = nullptr;

        std::vector<CommandList*> m_pendingCommandList;
    };

}

#endif // TITANIUM_METAL_DEVICE_H
