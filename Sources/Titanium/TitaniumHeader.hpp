#ifndef TITANIUM_TITANIUM_HEADER_H
#define TITANIUM_TITANIUM_HEADER_H

#if defined(TITANIUM_VULKAN)

#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI
{
    using RHI = Vulkan::RHI;
    using Device = Vulkan::Device;
}

#elif defined(TITANIUM_DIRECT_X12)

#include <DirectX12/DirectX12-RHI.hpp>

namespace TiRHI
{
    using RHI = DirectX12::RHI;
    using Device = DirectX12::Device;
}

#elif defined(TITANIUM_METAL)

#include <Metal/Metal-RHI.hpp>

namespace TiRHI
{
    using Rhi = MetalRHI;
}

#else
#error "No TitaniumRHI backend selected"
#endif

namespace TiRHI::Contract
{
    struct RHIContractAccess
    {
        template<typename T>
        static auto createDevice(T& device) -> decltype(device.createDevice())
        {
            return device.createDevice();
        }
        template<typename T>
        static auto createDevice(T& device, size_t index) -> decltype(device.createDevice(index))
        {
            return device.createDevice(index);
        }
    };

    template<typename T>
    concept RHIContract = requires(T device) {
        { RHIContractAccess::createDevice(device) } -> std::same_as<Device>;
    };

    static_assert(RHIContract<RHI>);

    struct DeviceContractAccess
    {
        template<typename T>
        static auto wait(T& device) -> decltype(device.wait());
    };

    template<typename T>
    concept DeviceContract = requires(T device) {
        { DeviceContractAccess::wait(device) } -> std::same_as<void>;
    };

    static_assert(DeviceContract<Device>);

} // TiRHI::Contracts

#endif // TITANIUM_TITANIUM_HEADER_H
