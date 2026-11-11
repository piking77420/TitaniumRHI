#include <Titanium/RHITypes.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    uint32_t pixelSizeFromFormat(Format format)
    {
        switch (format)
        {
        case Format::R8UNorm:
            return 1;

        case Format::R8G8UNorm:
        case Format::R16Float:
        case Format::D16UNorm:
            return 2;

        case Format::R8G8B8A8UNorm:
        case Format::R8G8B8A8UNormSRGB:
        case Format::B8G8R8A8UNorm:
        case Format::B8G8R8A8UNormSRGB:
        case Format::R16G16Float:
        case Format::R32Float:
        case Format::R32UInt:
        case Format::D24UNormS8UInt:
        case Format::D32Float:
            return 4;

        case Format::R16G16B16A16Float:
        case Format::R32G32Float:
        case Format::R32G32UInt:
        case Format::D32FloatS8UInt:
            return 8;

        case Format::R32G32B32A32Float:
        case Format::R32G32B32A32UInt:
            return 16;

        default:
            RHI_LOG_ERROR(
                std::format(L"Unsupported RHI Format for Vulkan image aspect, Format = {}", toWString(format)),
                RhiApi::Vulkan);
            return 0;
        }

        return 0;
    }
} // namespace TiRHI
