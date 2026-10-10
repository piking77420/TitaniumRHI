#ifndef TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H
#define TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H

#include <Titanium/RHITypes.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12::Private
{

    DXGI_FORMAT toDirectX12(Format format);

} // TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_RHI_TO_DIRECTX12_H
