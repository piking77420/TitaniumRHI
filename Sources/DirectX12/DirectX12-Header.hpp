#ifndef TITANIUM_DIRECTX12_HEADER_H
#define TITANIUM_DIRECTX12_HEADER_H

#include <wrl.h>

namespace TiRHI
{
    template<typename T>
    using MComPtr = Microsoft::WRL::ComPtr<T>;
} // namespace TiRHI

#endif // TITANIUM_DIRECTX12_HEADER_H
