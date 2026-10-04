#ifndef TITANIUM_DIRECTX12_UTILIS_H
#define TITANIUM_DIRECTX12_UTILIS_H

#include <vector>
#include <dxgi1_6.h>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12::Internal
{
    std::vector<MComPtr<IDXGIAdapter1>> getAllNativeAdapters(IDXGIFactory6* factory);

} // TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_UTILIS_H
