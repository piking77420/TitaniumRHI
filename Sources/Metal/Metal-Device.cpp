#ifndef TITANIUM_METAL_DEVICE_H
#define TITANIUM_METAL_DEVICE_H

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Metal/Metal.hpp>
#include <MetalFX/MetalFX.hpp>

#include <Metal/Metal.hpp>

#include <Titanium/Device.hpp>

namespace TiRHI
{
    struct Device::Backend
    {
    };

    Device::Device()
    {
    }

    Device::~Device()
    {
    }
}

#endif // TITANIUM_METAL_DEVICE_H
