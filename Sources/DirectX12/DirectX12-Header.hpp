#ifndef TITANIUM_DIRECTX12_HEADER_H
#define TITANIUM_DIRECTX12_HEADER_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <Windows.h>

#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <dxgi1_4.h>

namespace TiRHI::DirectX12
{
    template<typename T>
    using MComPtr = Microsoft::WRL::ComPtr<T>;
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_HEADER_H
