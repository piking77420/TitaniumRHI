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

} // TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H
