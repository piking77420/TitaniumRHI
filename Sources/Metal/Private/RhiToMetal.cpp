#include <Private/RhiToMetal.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Metal::Private
{
    MTL::PixelFormat toMetal(Format format)
    {
        switch (format)
        {
        case Format::R8_UNorm:
            return MTL::PixelFormatR8Unorm;

        case Format::R8G8_UNorm:
            return MTL::PixelFormatRG8Unorm;

        case Format::R8G8B8A8_UNorm:
            return MTL::PixelFormatRGBA8Unorm;

        case Format::R8G8B8A8_UNorm_SRGB:
            return MTL::PixelFormatRGBA8Unorm_sRGB;

        case Format::B8G8R8A8_UNorm:
            return MTL::PixelFormatBGRA8Unorm;

        case Format::B8G8R8A8_UNorm_SRGB:
            return MTL::PixelFormatBGRA8Unorm_sRGB;

        case Format::R16_Float:
            return MTL::PixelFormatR16Float;

        case Format::R16G16_Float:
            return MTL::PixelFormatRG16Float;

        case Format::R16G16B16A16_Float:
            return MTL::PixelFormatRGBA16Float;

        case Format::R32_Float:
            return MTL::PixelFormatR32Float;

        case Format::R32G32_Float:
            return MTL::PixelFormatRG32Float;

        case Format::R32G32B32A32_Float:
            return MTL::PixelFormatRGBA32Float;

        case Format::R32_UInt:
            return MTL::PixelFormatR32Uint;

        case Format::R32G32_UInt:
            return MTL::PixelFormatRG32Uint;

        case Format::R32G32B32A32_UInt:
            return MTL::PixelFormatRGBA32Uint;

        case Format::D16_UNorm:
            return MTL::PixelFormatDepth16Unorm;

        case Format::D24_UNorm_S8_UInt:
            return MTL::PixelFormatDepth24Unorm_Stencil8;

        case Format::D32_Float:
            return MTL::PixelFormatDepth32Float;

        case Format::D32_Float_S8_UInt:
            return MTL::PixelFormatDepth32Float_Stencil8;

        default:
            RHI_LOG_ERROR(L"Unsupported Metal format", RhiApi::Metal);
            return MTL::PixelFormatInvalid;
        }
        
        return MTL::PixelFormatInvalid;
    }

} // namespace TiRHI::Metal::Private