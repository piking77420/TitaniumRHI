#ifndef TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H
#define TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H

#include <Titanium/RHITypes.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12::Private
{
    DXGI_FORMAT toDirectX12(Format format);

    D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE toDirectX12(LoadOp loadOp);

    D3D12_RENDER_PASS_ENDING_ACCESS_TYPE toDirectX12(StoreOp storeOp);

    D3D12_RESOURCE_STATES toDirectX12(ResourceState state);

    D3D_PRIMITIVE_TOPOLOGY toDirectX12(PrimitiveTopology primitiveTopology);

    D3D12_PRIMITIVE_TOPOLOGY_TYPE toDirectX12TopologyType(PrimitiveTopology primitiveTopology);

    D3D12_FILL_MODE toDirectX12(PolygonMode polygonMode);

    D3D12_CULL_MODE toDirectX12(CullModeFlag cullModeFlag);

    BOOL toDirectX12(FrontFace frontFace);

} // TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H
