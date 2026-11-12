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

        std::optional<DescriptorHandle> create(Device& device, uint32_t count = 1);

        void release(DescriptorHandle& cpuDescriptorHandle);

        MComPtr<ID3D12DescriptorHeap> getHeap() const
        {
            return m_heap;
        }

    private:
        struct FreeBlock
        {
            uint32_t start;
            uint32_t count;
        };

        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        bool m_shaderVisible = false;

        UINT m_descriptorSize = 0;

        UINT m_capacity = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE m_baseCpuPtr{};

        D3D12_GPU_DESCRIPTOR_HANDLE m_baseGpuPtr{};

        MComPtr<ID3D12DescriptorHeap> m_heap;

        std::vector<FreeBlock> m_freeBlocks;

        DescriptorHandle getHandle(uint32_t index, uint32_t count);
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
