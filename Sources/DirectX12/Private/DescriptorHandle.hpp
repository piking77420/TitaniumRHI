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
        // how much consecutive descriptors:
        uint32_t count = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};

        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return count > 0 && cpuHandle.ptr != 0 && gpuHandle.ptr != 0;
        }
    };

} // TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_HANDLE_H
