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

        return m_pools.emplace_back(std::make_unique<Pool>())->build(*device.getNativeDevice());
    }

    std::optional<D3D12_CPU_DESCRIPTOR_HANDLE>
    DescriptorAllocator::getOrCreate(Device& device, CpuDescriptorHandle& cpuDescriptorHandle)
    {
        if (!cpuDescriptorHandle.descriptorIndex.isValid() && !cpuDescriptorHandle.poolIndex.isValid())
        {
            auto optAlloc = allocate(*device.getNativeDevice());
            if (optAlloc)
            {
                std::tuple<DescriptorAllocator::Pool*, PoolIndex, DesciptorIndex>& alloc = *optAlloc;
                cpuDescriptorHandle.poolIndex = std::get<PoolIndex>(alloc);
                cpuDescriptorHandle.descriptorIndex = std::get<DesciptorIndex>(alloc);

                Pool* pool = std::get<DescriptorAllocator::Pool*>(alloc);
                cpuDescriptorHandle.generation = pool->generations[cpuDescriptorHandle.poolIndex.toUnderlying()];

                D3D12_CPU_DESCRIPTOR_HANDLE handle = pool->baseHandlePtr;
                handle.ptr += cpuDescriptorHandle.descriptorIndex.toUnderlying() * pool->rtvDescriptorSize;
                return handle;
            }
            else
            {
                RHI_LOG_FATAL(L"Failed to allocate Descriptor", RhiApi::DirectX12);
                return {};
            }
        }
        else if (cpuDescriptorHandle.descriptorIndex.isValid() && cpuDescriptorHandle.poolIndex.isValid())
        {
            if (cpuDescriptorHandle.poolIndex.toUnderlying() >= 0 &&
                cpuDescriptorHandle.poolIndex.toUnderlying() < m_pools.size())
            {
                Pool& pool = *m_pools[cpuDescriptorHandle.poolIndex.toUnderlying()].get();

                D3D12_CPU_DESCRIPTOR_HANDLE handle = pool.baseHandlePtr;
                handle.ptr += cpuDescriptorHandle.descriptorIndex.toUnderlying() * pool.rtvDescriptorSize;
                return handle;
            }
            else
            {
                RHI_LOG_FATAL(L"Un error as occur", RhiApi::DirectX12);
            }
        }
        else
        {
            RHI_LOG_FATAL(L"Un error as occur", RhiApi::DirectX12);
        }
        return {};
    }

    std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> DescriptorAllocator::get(const CpuDescriptorHandle& cpuDescriptorHandle)
    {
        if (!cpuDescriptorHandle.descriptorIndex.isValid() || !cpuDescriptorHandle.poolIndex.isValid())
        {
            return std::nullopt;
        }

        if (cpuDescriptorHandle.poolIndex.toUnderlying() >= 0 &&
            cpuDescriptorHandle.poolIndex.toUnderlying() < m_pools.size())
        {
            Pool& pool = *m_pools[cpuDescriptorHandle.poolIndex.toUnderlying()].get();

            D3D12_CPU_DESCRIPTOR_HANDLE handle = pool.baseHandlePtr;
            handle.ptr += cpuDescriptorHandle.descriptorIndex.toUnderlying() * pool.rtvDescriptorSize;
            return handle;
        }
        else
        {
            RHI_LOG_FATAL(L"Un error as occur", RhiApi::DirectX12);
        }
        return std::nullopt;
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
        pool->build(device);

        // allocate index 0
        pool->m_availableMask &= ~uint64_t{1};

        return std::make_optional(std::make_tuple(pool.get(), PoolIndex::FromUnderlying(m_pools.size() - 1),
                                                  DesciptorIndex::FromUnderlying(0u)));
    }

    bool DescriptorAllocator::Pool::build(ID3D12Device& device)
    {
        RHI_LOG_INFO(L"New DescriptorAllocator pool", RhiApi::DirectX12);

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.NumDescriptors = static_cast<UINT>(BaseCapacity);
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        const HRESULT result = device.CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap));

        if (FAILED(result))
        {
            RHI_LOG_ERROR(L"Failed to create DescriptorAllocator's descriptor heap", RhiApi::DirectX12);
            return false;
        }

        rtvDescriptorSize = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        baseHandlePtr = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        m_availableMask = std::numeric_limits<uint64_t>::max();

        return true;
    }

} // namespace TiRHI::DirectX12::Private
