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
#include <DirectX12/DirectX12-SwapChain.hpp>
#include <DirectX12/DirectX12-CommandList.hpp>

namespace TiRHI
{
    using RHI = DirectX12::RHI;
    using Device = DirectX12::Device;
    using SwapChain = DirectX12::SwapChain;
    using CommandList = DirectX12::CommandList;
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
    template<typename T>
    concept RHIContract = std::constructible_from<T, const RhiCreate&>;

    static_assert(RHIContract<RHI>);

    // Device
    template<typename T>
    concept DeviceContract = requires(T device, RHI& rhi, const std::span<const Adapter>& adapters,
                                      std::optional<size_t> index, CommandList& cmdList) {
        { device.build(rhi, adapters, index) } -> std::same_as<bool>;
        { device.wait() } -> std::same_as<void>;
        { device.submit(cmdList) } -> std::same_as<void>;
    };

    static_assert(DeviceContract<Device>);

    // SwapChain
    template<typename T>
    concept SwapChainContract = requires(T& swapChain, Device& device, WindowHandle windowHandle) {
        { swapChain.build(device, windowHandle) } -> std::same_as<bool>;
        { swapChain.beginFrame() } -> std::same_as<bool>;
        { swapChain.present(device) } -> std::same_as<bool>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(SwapChainContract<SwapChain>);

    // CommandList

    template<typename T>
    concept CommandListContract = requires(T& commandList, Device& device) {
        { commandList.build(device) } -> std::same_as<bool>;
        { commandList.beginRecord() } -> std::same_as<bool>;
        { commandList.endRecord() } -> std::same_as<bool>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(CommandListContract<CommandList>);

} // TiRHI::Contracts

#endif // TITANIUM_TITANIUM_HEADER_H
