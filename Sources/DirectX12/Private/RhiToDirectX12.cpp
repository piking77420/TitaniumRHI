#include <Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12::Private
{
    DXGI_FORMAT toDirectX12(Format format)
    {
        switch (format)
        {
        case Format::R8_UNorm:
            return DXGI_FORMAT_R8_UNORM;

        case Format::R8G8_UNorm:
            return DXGI_FORMAT_R8G8_UNORM;

        case Format::R8G8B8A8_UNorm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case Format::R8G8B8A8_UNorm_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case Format::B8G8R8A8_UNorm:
            return DXGI_FORMAT_B8G8R8A8_UNORM;

        case Format::B8G8R8A8_UNorm_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        case Format::R16_Float:
            return DXGI_FORMAT_R16_FLOAT;

        case Format::R16G16_Float:
            return DXGI_FORMAT_R16G16_FLOAT;

        case Format::R16G16B16A16_Float:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case Format::R32_Float:
            return DXGI_FORMAT_R32_FLOAT;

        case Format::R32G32_Float:
            return DXGI_FORMAT_R32G32_FLOAT;

        case Format::R32G32B32A32_Float:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case Format::R32_UInt:
            return DXGI_FORMAT_R32_UINT;

        case Format::R32G32_UInt:
            return DXGI_FORMAT_R32G32_UINT;

        case Format::R32G32B32A32_UInt:
            return DXGI_FORMAT_R32G32B32A32_UINT;

        case Format::D16_UNorm:
            return DXGI_FORMAT_D16_UNORM;

        case Format::D24_UNorm_S8_UInt:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;

        case Format::D32_Float:
            return DXGI_FORMAT_D32_FLOAT;

        case Format::D32_Float_S8_UInt:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }
} // namespace TiRHI::DirectX12::Private
