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

        auto createPool = [nativeDevice, heapType = m_descriptorHeapType]() -> std::optional<Pool>
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
            heapDesc.Type = heapType;
            heapDesc.NumDescriptors = static_cast<UINT>(BaseCapacity);
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            Pool pool;
            const HRESULT result = nativeDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pool.heap));

            if (FAILED(result))
            {
                RHI_LOG_ERROR(L"Failed to create DescriptorAllocator's descriptor heap", RhiApi::DirectX12);
                return {};
            }

            pool.baseCpuPtr = pool.heap->GetCPUDescriptorHandleForHeapStart();

            return pool;
        };

        auto initializeHandle = [size = m_descriptorSize](Pool& pool,
                                                          std::size_t indexInPool) -> D3D12_CPU_DESCRIPTOR_HANDLE
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = pool.baseCpuPtr;
            handle.ptr += static_cast<SIZE_T>(size) * indexInPool;

            return handle;
        };

        return m_handleAllocator.build(createPool, initializeHandle);
    }

    std::optional<DescriptorAllocator::Handle> DescriptorAllocator::create(Device& device)
    {
        return m_handleAllocator.create();
    }

    void DescriptorAllocator::release(std::optional<DescriptorAllocator::Handle>& handle)
    {
        if (!handle)
            return;

        m_handleAllocator.release(*handle);
    }

} // namespace TiRHI::DirectX12::Private
