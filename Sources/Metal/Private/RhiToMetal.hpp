#ifndef TITANIUM_METAL_PRIVATE_RHI_TO_METAL_H
#define TITANIUM_METAL_PRIVATE_RHI_TO_METAL_H

#include <Metal/Metal-Header.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::Metal::Private
{
    MTL::PixelFormat toMetal(Format format);

} // namespace TiRHI::Metal::Private

#endif // TITANIUM_METAL_PRIVATE_RHI_TO_METAL_H