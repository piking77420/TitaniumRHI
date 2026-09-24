#ifndef TITANIUM_TITANIUM_HEADER_H
#define TITANIUM_TITANIUM_HEADER_H

#if defined(TITANIUM_VULKAN)

#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI
{
    using Rhi = VulkanRHI;
}

#elif defined(TITANIUM_DIRECT_X12)

#include <DirectX12/DirectX12-RHI.hpp>

namespace TiRHI
{
    using Rhi = DirectX12RHI;
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

#endif // TITANIUM_TITANIUM_HEADER_H
