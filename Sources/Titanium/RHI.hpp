#ifndef TITANIUM_RHI_H
#define TITANIUM_RHI_H

#include <concepts>

#if defined(TITANIUM_VULKAN)
#include <Vulkan/Vulkan-RHI.hpp>
#elif defined(TITANIUM_DIRECT_X12)
#include <DirectX12/DirectX12-RHI.hpp>
#elif defined(TITANIUM_METAL)
#include <Metal/Metal-RHI.hpp>
#endif // defined(TITANIUM_VULKAN)

namespace TiRHI
{
    template<typename T>
    concept RHISignature = requires(T& thing) {
        { thing.waitForDeviceIdle() } -> std::same_as<void>;
    };

    static_assert(RHISignature<RHI>);
}

#endif // TITANIUM_D3D12_DEVICE_H
