#ifndef TITANIUM_TITANIUM_HEADER_H
#define TITANIUM_TITANIUM_HEADER_H

#if defined(TITANIUM_VULKAN)

#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Vulkan-Surface.hpp>
#include <Vulkan/Vulkan-SwapChain.hpp>
#include <Vulkan/Vulkan-CommandList.hpp>
#include <Vulkan/Vulkan-RenderPassDescriptor.hpp>
#include <Vulkan/Vulkan-RenderTargets.hpp>
#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-GraphicPipeline.hpp>

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
    using Texture = Vulkan::Texture;
    using GraphicPipeline = Vulkan::GraphicPipeline;
}

#elif defined(TITANIUM_DIRECT_X12)

#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Surface.hpp>
#include <DirectX12/DirectX12-SwapChain.hpp>
#include <DirectX12/DirectX12-CommandList.hpp>
#include <DirectX12/DirectX12-AcquireFrame.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>
#include <DirectX12/DirectX12-RenderTargets.hpp>
#include <DirectX12/DirectX12-Texture.hpp>
#include <DirectX12/DirectX12-GraphicPipeline.hpp>

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
    using Texture = DirectX12::Texture;
    using GraphicPipeline = DirectX12::GraphicPipeline;
}

#elif defined(TITANIUM_METAL)

#include <Metal/Metal-RHI.hpp>
#include <Metal/Metal-Surface.hpp>
#include <Metal/Metal-SwapChain.hpp>
#include <Metal/Metal-CommandList.hpp>
#include <Metal/Metal-AcquiredFrame.hpp>
#include <Metal/Metal-RenderPassDescriptor.hpp>
#include <Metal/Metal-RenderTargets.hpp>
#include <Metal/Metal-Texture.hpp>
#include <Metal/Metal-GraphicPipeline.hpp>

namespace TiRHI
{
    using RHI = Metal::RHI;
    using Surface = Metal::Surface;
    using Device = Metal::Device;
    using SwapChain = Metal::SwapChain;
    using AcquiredFrame = Metal::AcquiredFrame;
    using CommandList = Metal::CommandList;
    using RenderPassDescriptor = Metal::RenderPassDescriptor;
    using RenderTargets = Metal::RenderTargets;
    using Texture = Metal::Texture;
    using GraphicPipeline = Metal::GraphicPipeline;
}

#else
#error "No TitaniumRHI backend selected"
#endif

namespace TiRHI::Contract
{
    /*
    template<typename T>
    concept RHIContract = requires(T& rhi) {
        { rhi.newDevice() } -> std::same_as<Device>;
        { std::as_const(rhi).getApi() } -> std::same_as<RhiApi>;
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
        { swapChain.acquireNextImage(device) } -> std::same_as<AcquiredFrame>;
        { swapChain.present() } -> std::same_as<bool>;
        { std::as_const(swapChain).getRenderPassDescriptor() } -> std::same_as<const RenderPassDescriptor&>;
        { std::as_const(swapChain).getCurrentRenderTargets() } -> std::same_as<const RenderTargets&>;
        { swapChain.getCurrentRenderTargets() } -> std::same_as<RenderTargets&>;
        { std::as_const(swapChain.getCurrentSwapChainTexture()) } -> std::same_as<const Texture&>;
        { swapChain.getCurrentSwapChainTexture() } -> std::same_as<Texture&>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(SwapChainContract<SwapChain>);

    // CommandList

    template<typename T>
    concept CommandListContract = requires(T& commandList, Device& device, const Viewport& viewport,
                                           const Rect2D& rect2D, const BeginRenderPass& beginRenderPass,
                                           RenderTargets& renderTargets, const GraphicPipeline& graphicPipeline,
                                           uint32_t uint) {
        { commandList.build(device) } -> std::same_as<bool>;
        { commandList.beginRecord() } -> std::same_as<bool>;
        { commandList.endRecord() } -> std::same_as<bool>;
        { commandList.setViewPort(viewport) } -> std::same_as<void>;
        { commandList.setScissors(rect2D) } -> std::same_as<void>;
        { commandList.beginRenderPass(beginRenderPass, renderTargets) } -> std::same_as<bool>;
        { commandList.endRenderPass() } -> std::same_as<void>;
        { commandList.bindPipeline(graphicPipeline) } -> std::same_as<void>;
        { commandList.draw(uint, uint, uint, uint) } -> std::same_as<void>;
        { commandList.drawIndexed(uint, uint, uint, uint, uint) } -> std::same_as<void>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(CommandListContract<CommandList>);

    template<typename T>
    concept RederPassDescriptorContract = requires(T& renderPassDescriptor, Device& device) {
        { renderPassDescriptor.build(device) } -> std::same_as<bool>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(RederPassDescriptorContract<RenderPassDescriptor>);

    template<typename T>
    concept RenderTargetsContract = requires(T& renderTargets, Device& device) {
        { renderTargets.build(device) } -> std::same_as<bool>;
    } && std::derived_from<T, Object<T, RHI>>;

    static_assert(RenderTargetsContract<RenderTargets>);

    template<typename T>
    concept TextureContract = requires(T& texture, Device& device) {
        { texture.build(device) } -> std::same_as<bool>;
    } && std::derived_from<T, Object<T, RHI>>;

    template<typename T>
    concept GraphicPipelineContract = requires(T& graphicPipeline, Device& device) {
        { graphicPipeline.build(device) } -> std::same_as<bool>;
    } && std::derived_from<T, Object<T, RHI>>;*/

} // TiRHI::Contracts

#endif // TITANIUM_TITANIUM_HEADER_H
