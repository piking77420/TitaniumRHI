#ifndef TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_HANDLE_H
#define TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_HANDLE_H

#include <array>
#include <limits>
#include <DirectX12-Header.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::DirectX12::Private
{
    struct DescriptorHandle
    {
        static constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

        // index in the pool
        size_t elementIndex = InvalidIndex;

        // which pool index
        size_t poolIndex = InvalidIndex;

        // how much consecutive descriptors:
        size_t count = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE nativeHandle{};

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return elementIndex != InvalidIndex && poolIndex != InvalidIndex && count > 0;
        }
    };

} // TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_HANDLE_H
