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
#include <Titanium/Private/StrongTypeId.hpp>
#include <Titanium/RHI-Object.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/Private/DescriptorHandle.hpp>

namespace TiRHI::DirectX12
{
    class Device;
} // TiRHI::DirectX12

namespace TiRHI::DirectX12::Private
{
    // TODO:
    // - Use one large shader-visible ID3D12DescriptorHeap per heap type.
    // - Store both CPU and GPU base descriptor handles.
    // - Allocate descriptors using indexed slots.
    // - Track occupied slots using std::bitset or a dynamic bitset.
    // - Support contiguous descriptor ranges for descriptor tables.
    // - Defer descriptor reuse until GPU execution has finished.
    // - DescriptorHandle should contain CPU/GPU handles.
    // - Keep allocation metadata inside DescriptorAllocator.

    class DescriptorAllocator
    {
    public:
        DescriptorAllocator() = default;
        ~DescriptorAllocator() = default;
        RHI_MOVE_ONLY(DescriptorAllocator);

        static constexpr size_t Capacity = 64;

        struct Pool
        {
            D3D12_CPU_DESCRIPTOR_HANDLE baseCpuPtr;

            MComPtr<ID3D12DescriptorHeap> heap;

            std::bitset<Capacity> availableMask;

            Pool()
            {
                availableMask.set();
            }
        };

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

        std::optional<DescriptorHandle> create(Device& device);

        void release(DescriptorHandle& cpuDescriptorHandle);

        auto getPool(uint32_t index) const -> const Pool*;

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        UINT m_descriptorSize = 0;

        std::vector<Pool> m_pools;

        std::optional<DescriptorHandle> createFromPool(std::size_t poolIndex);

        Pool createPool(MComPtr<ID3D12Device> device);

        D3D12_CPU_DESCRIPTOR_HANDLE initHandle(Pool& pool, size_t indexInPool);
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
