#include <DirectX12/Private/DescriptorAllocator.hpp>

#include <bit>
#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-Device.hpp>

namespace TiRHI::DirectX12::Private
{
    bool DescriptorAllocator::build(Device& device)
    {
        if (!device.getNativeDevice())
            return false;

        return m_pools.emplace_back(std::make_unique<Pool>())
            ->build(*device.getNativeDevice(), getDescriptorHeapType());
    }

    std::optional<CpuDescriptorHandle> DescriptorAllocator::create(Device& device)
    {
        CpuDescriptorHandle cpuHandle;
        auto optAlloc = allocate(*device.getNativeDevice());

        if (!optAlloc)
            return {};

        std::tuple<DescriptorAllocator::Pool*, PoolIndex, DesciptorIndex>& alloc = *optAlloc;
        cpuHandle.poolIndex = std::get<PoolIndex>(alloc);
        cpuHandle.descriptorIndex = std::get<DesciptorIndex>(alloc);

        Pool* pool = std::get<DescriptorAllocator::Pool*>(alloc);
        cpuHandle.generation = pool->generations[cpuHandle.poolIndex.toUnderlying()];

        D3D12_CPU_DESCRIPTOR_HANDLE handle = pool->baseHandlePtr;
        handle.ptr += cpuHandle.descriptorIndex.toUnderlying() * pool->rtvDescriptorSize;

        cpuHandle.nativeHandle = handle;

        return cpuHandle;
    }

    void DescriptorAllocator::release(CpuDescriptorHandle& handle)
    {
        if (!handle.poolIndex.isValid() || !handle.descriptorIndex.isValid())
            return;

        const size_t poolIndex = handle.poolIndex.toUnderlying();
        const size_t index = handle.descriptorIndex.toUnderlying();

        if (poolIndex >= m_pools.size() || index >= Pool::BaseCapacity)
            return;

        Pool& pool = *m_pools[poolIndex];

        const uint64_t bit = uint64_t{1} << index;

        // Already free
        if (pool.m_availableMask & bit)
        {
            RHI_LOG_ERROR(L"Descriptor double free", RhiApi::DirectX12);
            return;
        }

        // Stale handle
        if (pool.generations[index] != handle.generation)
        {
            RHI_LOG_ERROR(L"Invalid descriptor generation", RhiApi::DirectX12);
            return;
        }

        pool.m_availableMask |= bit;
        ++pool.generations[index];

        handle = {};
    }

    std::optional<std::tuple<DescriptorAllocator::Pool*, PoolIndex, DesciptorIndex>>
    DescriptorAllocator::allocate(ID3D12Device& device)
    {

        for (size_t i = 0; i < m_pools.size(); i++)
        {
            auto& pool = m_pools[i];
            if (!pool)
                continue;

            const uint64_t available = pool->m_availableMask;

            if (available == 0)
                continue;

            const size_t index = std::countr_zero(available);

            // Mark descriptor as allocated
            pool->m_availableMask &= ~(uint64_t{1} << index);

            return std::make_optional(
                std::make_tuple(pool.get(), PoolIndex::FromUnderlying(i), DesciptorIndex::FromUnderlying(index)));
        }

        auto& pool = m_pools.emplace_back(std::make_unique<Pool>());
        pool->build(device, getDescriptorHeapType());

        // allocate index 0
        pool->m_availableMask &= ~uint64_t{1};

        return std::make_optional(std::make_tuple(pool.get(), PoolIndex::FromUnderlying(m_pools.size() - 1),
                                                  DesciptorIndex::FromUnderlying(0u)));
    }

    bool DescriptorAllocator::Pool::build(ID3D12Device& device, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType)
    {
        RHI_LOG_INFO(L"New DescriptorAllocator pool", RhiApi::DirectX12);

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = descriptorHeapType;
        heapDesc.NumDescriptors = static_cast<UINT>(BaseCapacity);
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        const HRESULT result = device.CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap));

        if (FAILED(result))
        {
            RHI_LOG_ERROR(L"Failed to create DescriptorAllocator's descriptor heap", RhiApi::DirectX12);
            return false;
        }

        rtvDescriptorSize = device.GetDescriptorHandleIncrementSize(descriptorHeapType);
        assert(rtvDescriptorSize != 0);

        baseHandlePtr = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        m_availableMask = std::numeric_limits<uint64_t>::max();

        return true;
    }

} // namespace TiRHI::DirectX12::Private
