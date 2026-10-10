#ifndef TITANIUM_TITANIUM_HEADER_H
#define TITANIUM_TITANIUM_HEADER_H

#if defined(TITANIUM_VULKAN)

#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Vulkan-Surface.hpp>
#include <Vulkan/Vulkan-SwapChain.hpp>
#include <Vulkan/Vulkan-CommandList.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Vulkan-RenderTargets.hpp>

namespace TiRHI
{
    using RHI = Vulkan::RHI;
    using Surface = Vulkan::Surface;
    using Device = Vulkan::Device;
    using SwapChain = Vulkan::SwapChain;
    using AcquiredFrame = Vulkan::AcquiredFrame;
    using CommandList = Vulkan::CommandList;
    using RenderPassDescriptor = Vulkan::RenderPassDescriptor;
    using RenderTargets = Vulkan::RenderTargets;
}

#elif defined(TITANIUM_DIRECT_X12)

#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Surface.hpp>
#include <DirectX12/DirectX12-SwapChain.hpp>
#include <DirectX12/DirectX12-CommandList.hpp>
#include <DirectX12/DirectX12-AcquireFrame.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>
#include <DirectX12/DirectX12-RenderTargets.hpp>

namespace TiRHI
{
    using RHI = DirectX12::RHI;
    using Surface = DirectX12::Surface;
    using Device = DirectX12::Device;
    using SwapChain = DirectX12::SwapChain;
    using AcquiredFrame = DirectX12::AcquiredFrame;
    using CommandList = DirectX12::CommandList;
    using RenderPassDescriptor = DirectX12::RenderPassDescriptor;
    using RenderTargets = DirectX12::RenderTargets;
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
    concept RHIContract = requires(T& rhi) {
        { rhi.newDevice() } -> std::same_as<Device>;
    } && std::constructible_from<T, const RhiCreate&>;

    static_assert(RHIContract<RHI>);

    // Surface
    template<typename T>
    concept SurfaceContract = requires(T surface, WindowHandle windowHandle) {
        { surface.build(windowHandle) } -> std::same_as<bool>;
    };
    static_assert(SurfaceContract<Surface>);
    // Device
    template<typename T>
    concept DeviceContract = requires(T device, RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                                      std::optional<size_t> index, std::span<const AcquiredFrame> acquiredFrames,
                                      std::span<CommandList*> cmdLists) {
        { device.beginFrame() } -> std::same_as<void>;
        { device.build(rhi, surface, adapters, index) } -> std::same_as<bool>;
        { device.wait() } -> std::same_as<void>;
        { device.submit(acquiredFrames, cmdLists) } -> std::same_as<void>;

        { device.isValid() } -> std::same_as<bool>;
        { device() } -> std::same_as<bool>;
    };

    static_assert(DeviceContract<Device>);

    // SwapChain
    template<typename T>
    concept SwapChainContract = requires(T& swapChain, Device& device, Surface& surface, WindowHandle windowHandle,
                                         const RenderPassDescriptor&) {
        { swapChain.build(device, surface) } -> std::same_as<bool>;
        { swapChain.acquireNextImage() } -> std::same_as<AcquiredFrame>;
        { swapChain.present() } -> std::same_as<bool>;
        { std::as_const(swapChain).getRenderPassDescriptor() } -> std::same_as<const RenderPassDescriptor&>;
        { std::as_const(swapChain).getCurrentRenderTargets() } -> std::same_as<const RenderTargets&>;
        { swapChain.getCurrentRenderTargets() } -> std::same_as<RenderTargets&>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(SwapChainContract<SwapChain>);

    // CommandList

    template<typename T>
    concept CommandListContract =
        requires(T& commandList, Device& device, const Viewport& viewport, const Rect2D& rect2D) {
            { commandList.build(device) } -> std::same_as<bool>;
            { commandList.beginRecord() } -> std::same_as<bool>;
            { commandList.endRecord() } -> std::same_as<bool>;
            { commandList.setViewPort(viewport) } -> std::same_as<void>;
            { commandList.setScissors(rect2D) } -> std::same_as<void>;
        } && std::derived_from<T, Object<T, RHI>>;

    static_assert(CommandListContract<CommandList>);

} // TiRHI::Contracts

#endif // TITANIUM_TITANIUM_HEADER_H
