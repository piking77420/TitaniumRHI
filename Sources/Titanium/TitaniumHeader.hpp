#ifndef TITANIUM_TITIANIUM_HEADER_H
#define TITANIUM_TITIANIUM_HEADER_H

#if defined(TITANIUM_VULKAN)
#include <Vulkan/Vulkan-RHI.hpp>
#elif defined(TITANIUM_DIRECT_X12)
#include <DirectX12/DirectX12-RHI.hpp>
namespace TiRHI
{
    using Rhi = TiRHI::DirectX12RHI;
} // TiRHI
#elif defined(TITANIUM_METAL)
#include <Metal/Metal-RHI.hpp>
#endif // defined(TITANIUM_VULKAN)

#endif // TITANIUM_TITIANIUM_HEADER_H
