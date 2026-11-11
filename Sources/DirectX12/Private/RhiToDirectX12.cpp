#include <Private/RhiToDirectX12.hpp>

#include <format>
#include <Titanium/Log.hpp>

namespace TiRHI::DirectX12::Private
{
    DXGI_FORMAT toDirectX12(Format format)
    {
        switch (format)
        {
        case Format::R8UNorm:
            return DXGI_FORMAT_R8_UNORM;

        case Format::R8G8UNorm:
            return DXGI_FORMAT_R8G8_UNORM;

        case Format::R8G8B8A8UNorm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case Format::R8G8B8A8UNormSRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case Format::B8G8R8A8UNorm:
            return DXGI_FORMAT_B8G8R8A8_UNORM;

        case Format::B8G8R8A8UNormSRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        case Format::R16Float:
            return DXGI_FORMAT_R16_FLOAT;

        case Format::R16G16Float:
            return DXGI_FORMAT_R16G16_FLOAT;

        case Format::R16G16B16A16Float:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case Format::R32Float:
            return DXGI_FORMAT_R32_FLOAT;

        case Format::R32G32Float:
            return DXGI_FORMAT_R32G32_FLOAT;

        case Format::R32G32B32A32Float:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case Format::R32UInt:
            return DXGI_FORMAT_R32_UINT;

        case Format::R32G32UInt:
            return DXGI_FORMAT_R32G32_UINT;

        case Format::R32G32B32A32UInt:
            return DXGI_FORMAT_R32G32B32A32_UINT;

        case Format::D16UNorm:
            return DXGI_FORMAT_D16_UNORM;

        case Format::D24UNormS8UInt:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;

        case Format::D32Float:
            return DXGI_FORMAT_D32_FLOAT;

        case Format::D32FloatS8UInt:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        default:
        {
            RHI_LOG_ERROR(std::format(L"Unsupported format '{}' for DirectX 12 conversion.", toWString(format)),
                          RhiApi::DirectX12);
            return DXGI_FORMAT_UNKNOWN;
        }
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    D3D12_SHADER_VISIBILITY toDirectX12(ShaderStageFlags flag)
    {
        const uint32_t value = static_cast<uint32_t>(flag);

        switch (value)
        {
        case static_cast<uint32_t>(ShaderStageFlags::Vertex):
            return D3D12_SHADER_VISIBILITY_VERTEX;

        case static_cast<uint32_t>(ShaderStageFlags::Hull):
            return D3D12_SHADER_VISIBILITY_HULL;

        case static_cast<uint32_t>(ShaderStageFlags::Domain):
            return D3D12_SHADER_VISIBILITY_DOMAIN;

        case static_cast<uint32_t>(ShaderStageFlags::Geometry):
            return D3D12_SHADER_VISIBILITY_GEOMETRY;

        case static_cast<uint32_t>(ShaderStageFlags::Pixel):
            return D3D12_SHADER_VISIBILITY_PIXEL;

        case static_cast<uint32_t>(ShaderStageFlags::Amplification):
            return D3D12_SHADER_VISIBILITY_AMPLIFICATION;

        case static_cast<uint32_t>(ShaderStageFlags::Mesh):
            return D3D12_SHADER_VISIBILITY_MESH;

        default:
            // Compute, RayTracing, and multi-stage masks.
            return D3D12_SHADER_VISIBILITY_ALL;
        }
    }

    D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE toDirectX12(LoadOp loadOp)
    {
        switch (loadOp)
        {
        case LoadOp::LoadOp:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;

        case LoadOp::Clear:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;

        case LoadOp::DontCare:
            return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
        }

        RHI_LOG_ERROR(std::format(L"Unsupported LoadOp '{}' for DirectX 12 conversion.", toWString(loadOp)),
                      RhiApi::DirectX12);

        return {};
    }

    D3D12_RENDER_PASS_ENDING_ACCESS_TYPE toDirectX12(StoreOp storeOp)
    {
        switch (storeOp)
        {
        case StoreOp::Store:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

        case StoreOp::DontCare:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;

        case StoreOp::None:
            return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
        }

        RHI_LOG_ERROR(std::format(L"Unsupported StoreOp '{}' for DirectX 12 conversion.", toWString(storeOp)),
                      RhiApi::DirectX12);
        return {};
    }

    D3D12_RESOURCE_STATES toDirectX12(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::Undefined:
            return D3D12_RESOURCE_STATE_COMMON;

        case ResourceState::Common:
            return D3D12_RESOURCE_STATE_COMMON;

        case ResourceState::VertexBuffer:
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

        case ResourceState::IndexBuffer:
            return D3D12_RESOURCE_STATE_INDEX_BUFFER;

        case ResourceState::ConstantBuffer:
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

        case ResourceState::ShaderResource:
            return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

        case ResourceState::UnorderedAccess:
            return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

        case ResourceState::RenderTarget:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;

        case ResourceState::DepthWrite:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;

        case ResourceState::DepthRead:
            return D3D12_RESOURCE_STATE_DEPTH_READ;

        case ResourceState::CopySource:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;

        case ResourceState::CopyDestination:
            return D3D12_RESOURCE_STATE_COPY_DEST;

        case ResourceState::IndirectArgument:
            return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

        case ResourceState::Present:
            return D3D12_RESOURCE_STATE_PRESENT;
        }

        RHI_LOG_ERROR(L"Unsupported resource state for DirectX 12 conversion.", RhiApi::DirectX12);

        return D3D12_RESOURCE_STATE_COMMON;
    }

    D3D_PRIMITIVE_TOPOLOGY toDirectX12(PrimitiveTopology primitiveTopology)
    {
        switch (primitiveTopology)
        {
        case PrimitiveTopology::PointList:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::LineList:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::TriangleList:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveTopology::LineListAdjacency:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case PrimitiveTopology::LineStripAdjacency:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case PrimitiveTopology::TriangleListAdjacency:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case PrimitiveTopology::TriangleStripAdjacency:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        case PrimitiveTopology::PatchList:
            // Requires the patch control point count (1–32).
            return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
        default:
            RHI_LOG_ERROR(
                std::format(L"Unsupported RHI PrimitiveTopology, Topology = {}", toWString(primitiveTopology)),
                RhiApi::DirectX12);
            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE toDirectX12TopologyType(PrimitiveTopology primitiveTopology)
    {
        switch (primitiveTopology)
        {
        case PrimitiveTopology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

        case PrimitiveTopology::LineList:
        case PrimitiveTopology::LineStrip:
        case PrimitiveTopology::LineListAdjacency:
        case PrimitiveTopology::LineStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleListAdjacency:
        case PrimitiveTopology::TriangleStripAdjacency:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        case PrimitiveTopology::PatchList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

        default:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }
    }

    D3D12_FILL_MODE toDirectX12(PolygonMode polygonMode)
    {
        switch (polygonMode)
        {
        case PolygonMode::Fill:
            return D3D12_FILL_MODE_SOLID;
        case PolygonMode::Line:
            return D3D12_FILL_MODE_WIREFRAME;
        default:
            RHI_LOG_ERROR(std::format(L"Unsupported RHI PolygonMode, Mode = {}", toWString(polygonMode)),
                          RhiApi::DirectX12);
            return D3D12_FILL_MODE_SOLID;
        }
    }

    D3D12_CULL_MODE toDirectX12(CullModeFlag cullModeFlag)
    {
        switch (cullModeFlag)
        {
        case CullModeFlag::None:
            return D3D12_CULL_MODE_NONE;
        case CullModeFlag::Front:
            return D3D12_CULL_MODE_FRONT;
        case CullModeFlag::Back:
            return D3D12_CULL_MODE_BACK;
        default:
            RHI_LOG_ERROR(std::format(L"Unsupported RHI CullModeFlag, Mode = {}", toWString(cullModeFlag)),
                          RhiApi::DirectX12);
            return D3D12_CULL_MODE_NONE;
        }
    }

    BOOL toDirectX12(FrontFace frontFace)
    {
        switch (frontFace)
        {
        case FrontFace::CounterClockwise:
            return TRUE;
        case FrontFace::Clockwise:
            return FALSE;
        default:
            RHI_LOG_ERROR(std::format(L"Unsupported RHI FrontFace, Face = {}", toWString(frontFace)),
                          RhiApi::DirectX12);
            return FALSE;
        }
    }

    D3D12_RESOURCE_FLAGS textureFlagtoDirectX12(TextureFlags flag, Format format)
    {
        D3D12_RESOURCE_FLAGS dx12flag;

        switch (format)
        {
        case Format::D16UNorm:
        case Format::D32Float:
        case Format::D24UNormS8UInt:
        case Format::D32FloatS8UInt:
            dx12flag |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            break;

        case Format::R8UNorm:
        case Format::R8G8UNorm:
        case Format::R8G8B8A8UNorm:
        case Format::R8G8B8A8UNormSRGB:
        case Format::B8G8R8A8UNorm:
        case Format::B8G8R8A8UNormSRGB:
        case Format::R16Float:
        case Format::R16G16Float:
        case Format::R16G16B16A16Float:
        case Format::R32Float:
        case Format::R32G32Float:
        case Format::R32G32B32A32Float:
        case Format::R32UInt:
        case Format::R32G32UInt:
        case Format::R32G32B32A32UInt:
            break;

        default:
            RHI_LOG_ERROR(
                std::format(L"Unsupported RHI Format for textureFlagtoDirectX12 , Format = {}", toWString(format)),
                RhiApi::Vulkan);
            return {};
        }

        if ((flag & TextureFlags::RenderTarget) != TextureFlags::None)
        {
            dx12flag |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }

        if ((flag & TextureFlags::UnorderedAccess) != TextureFlags::None)
        {
            dx12flag |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
    }

} // namespace TiRHI::DirectX12::Private
