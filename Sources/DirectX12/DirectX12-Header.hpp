#ifndef TITANIUM_DIRECTX12_HEADER_H
#define TITANIUM_DIRECTX12_HEADER_H

#include <wrl.h>

namespace TiRHI::DirectX12
{
    template<typename T>
    using MComPtr = Microsoft::WRL::ComPtr<T>;
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_HEADER_H
