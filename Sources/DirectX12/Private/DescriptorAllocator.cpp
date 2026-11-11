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

        Microsoft::WRL::ComPtr<ID3D12Device> nativeDevice = device.getNativeDevice();
        m_descriptorSize = nativeDevice->GetDescriptorHandleIncrementSize(m_descriptorHeapType);
        return true;
    }

    std::optional<DescriptorHandle> DescriptorAllocator::create(Device& device)
    {
        for (std::size_t i = 0; i < m_pools.size(); ++i)
        {
            if (m_pools[i].availableMask.any())
                return createFromPool(i);
        }

        m_pools.emplace_back(createPool(device.getNativeDevice()));

        return createFromPool(m_pools.size() - 1);
    }

    void DescriptorAllocator::release(DescriptorHandle& handle)
    {
        if (!handle)
        {
            return;
        }

        if (handle.poolIndex >= m_pools.size() || handle.elementIndex >= Capacity)
        {
            return;
        }

        Pool& pool = m_pools[handle.poolIndex];
        const std::size_t index = handle.elementIndex;

        if (pool.availableMask.test(index))
        {
            return;
        }

        pool.availableMask.set(index);
        // Invalidate this instance.
        handle = {};
    }

    auto DescriptorAllocator::getPool(uint32_t index) const -> const Pool*
    {
        return (index < m_pools.size()) ? &m_pools[index] : nullptr;
    }

    std::optional<DescriptorHandle> DescriptorAllocator::createFromPool(std::size_t poolIndex)
    {
        Pool& pool = m_pools[poolIndex];

        for (std::size_t index = 0; index < Capacity; ++index)
        {
            if (!pool.availableMask.test(index))
                continue;

            D3D12_CPU_DESCRIPTOR_HANDLE nativeHandle = initHandle(pool, index);

            // set dirty
            pool.availableMask.reset(index);
            return DescriptorHandle{index, poolIndex, 1, nativeHandle};
        }

        return std::nullopt;
    }

    auto DescriptorAllocator::createPool(MComPtr<ID3D12Device> device) -> Pool
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = m_descriptorHeapType;
        heapDesc.NumDescriptors = static_cast<UINT>(Capacity);
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        Pool pool;
        const HRESULT result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pool.heap));

        if (FAILED(result))
        {
            RHI_LOG_ERROR(L"Failed to create DescriptorAllocator's descriptor heap", RhiApi::DirectX12);
            return {};
        }

        pool.baseCpuPtr = pool.heap->GetCPUDescriptorHandleForHeapStart();

        return pool;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::initHandle(Pool& pool, size_t indexInPool)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = pool.baseCpuPtr;
        handle.ptr += static_cast<SIZE_T>(m_descriptorSize) * indexInPool;

        return handle;
    }

} // namespace TiRHI::DirectX12::Private
