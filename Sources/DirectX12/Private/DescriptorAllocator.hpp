#ifndef TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
#define TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H

#include <bitset>
#include <memory>
#include <array>
#include <optional>
#include <tuple>
#include <vector>
#include <utility>

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/Private/HandleAllocator.hpp>
#include <Titanium/Private/StrongTypeId.hpp>
#include <Titanium/RHI-Object.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class Device;
} // TiRHI::DirectX12

namespace TiRHI::DirectX12::Private
{
    class DescriptorAllocator
    {
    public:
        DescriptorAllocator() = default;
        ~DescriptorAllocator() = default;
        RHI_MOVE_ONLY(DescriptorAllocator);

        static constexpr size_t BaseCapacity = 64;

        struct Pool
        {
            D3D12_CPU_DESCRIPTOR_HANDLE baseCpuPtr;

            MComPtr<ID3D12DescriptorHeap> heap;
        };

        using Allocator = TiRHI::Private::HandleAllocator<Pool, D3D12_CPU_DESCRIPTOR_HANDLE, BaseCapacity>;
        using Handle = Allocator::Handle;

        D3D12_DESCRIPTOR_HEAP_TYPE getDescriptorHeapType()
        {
            return m_descriptorHeapType;
        }

        DescriptorAllocator& setDescriptorHeapType(D3D12_DESCRIPTOR_HEAP_TYPE type)
        {
            m_descriptorHeapType = type;
            return *this;
        }

        bool build(Device& device);

        std::optional<Handle> create(Device& device);

        void release(std::optional<Handle>& cpuDescriptorHandle);

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        UINT m_descriptorSize = 0;

        TiRHI::Private::HandleAllocator<Pool, D3D12_CPU_DESCRIPTOR_HANDLE, 64> m_handleAllocator;
    };

    using DescriptorHandle = DescriptorAllocator::Handle;

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
