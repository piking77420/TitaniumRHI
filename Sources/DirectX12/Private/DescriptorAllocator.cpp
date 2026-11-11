#include <DirectX12/Private/DescriptorAllocator.hpp>

#include <bit>
#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-Device.hpp>

namespace TiRHI::DirectX12::Private
{
    [[nodiscard]]
    constexpr uint32_t getGuaranteedMaxDescriptorCount(D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible = true)
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return shaderVisible ? D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1 : UINT32_MAX;

        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return shaderVisible ? D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE : UINT32_MAX;

        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return shaderVisible ? 0 : UINT32_MAX;

        default:
            return 0;
        }
    }

    bool DescriptorAllocator::build(Device& device)
    {
        if (!device.getNativeDevice())
            return false;

        Microsoft::WRL::ComPtr<ID3D12Device> nativeDevice = device.getNativeDevice();
        m_descriptorSize = nativeDevice->GetDescriptorHandleIncrementSize(m_descriptorHeapType);

        m_capacity = getGuaranteedMaxDescriptorCount(m_descriptorHeapType);
        if (m_capacity == 0)
            return false;

        m_shaderVisible = m_descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
                          m_descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = m_descriptorHeapType;
        heapDesc.NumDescriptors = m_capacity;
        heapDesc.Flags = m_shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ;
        const HRESULT result = nativeDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));

        if (FAILED(result))
        {
            RHI_LOG_ERROR(L"Failed to create DescriptorAllocator's descriptor heap", RhiApi::DirectX12);
            return {};
        }

        m_baseCpuPtr = m_heap->GetCPUDescriptorHandleForHeapStart();
        m_baseGpuPtr = m_shaderVisible ? m_heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};

        m_freeBlocks.emplace_back(FreeBlock{.start = 0, .count = heapDesc.NumDescriptors});

        return true;
    }

    std::optional<DescriptorHandle> DescriptorAllocator::create(Device& device, uint32_t count)
    {
        if (count == 0)
            return std::nullopt;

        for (size_t i = 0; i < m_freeBlocks.size(); ++i)
        {
            FreeBlock& block = m_freeBlocks[i];

            if (block.count < count)
                continue;

            const uint32_t startIndex = block.start;

            block.start += count;

            assert((static_cast<int64_t>(block.count) - static_cast<int64_t>(count)) >= 0);
            block.count -= count;

            if (block.count == 0)
                m_freeBlocks.erase(m_freeBlocks.begin() + i);

            return getHandle(startIndex, count);
        }

        return std::nullopt;
    }

    void DescriptorAllocator::release(DescriptorHandle& handle)
    {
        if (!handle)
        {
            return;
        }
        const SIZE_T offset = handle.cpuHandle.ptr - m_baseCpuPtr.ptr;
        assert(m_descriptorSize != 0);
        assert(offset % m_descriptorSize == 0);
        const uint32_t start = static_cast<uint32_t>(offset / m_descriptorSize);

        assert(start < m_capacity);
        assert(handle.count <= m_capacity - start);

        auto it = std::lower_bound(m_freeBlocks.begin(), m_freeBlocks.end(), start,
                                   [](const FreeBlock& block, uint32_t index) { return block.start < index; });

        assert(it == m_freeBlocks.end() || start + handle.count <= it->start);

        assert(it == m_freeBlocks.begin() || (it - 1)->start + (it - 1)->count <= start);

        it = m_freeBlocks.insert(it, FreeBlock{start, handle.count});

        // merge with previous block.
        if (it != m_freeBlocks.begin())
        {
            auto prev = it - 1;

            if (prev->start + prev->count == it->start)
            {
                prev->count += it->count;
                it = m_freeBlocks.erase(it);
                it = prev;
            }
        }

        auto next = it + 1;
        // merge with next block.
        if (next != m_freeBlocks.end() && it->start + it->count == next->start)
        {
            it->count += next->count;
            m_freeBlocks.erase(next);
        }

        // Invalidate this instance.
        handle = {};
    }

    DescriptorHandle DescriptorAllocator::getHandle(uint32_t index, uint32_t count)
    {
        return DescriptorHandle{
            .count = count,
            .cpuHandle = m_baseCpuPtr.ptr + m_descriptorSize * index,
            .gpuHandle = m_shaderVisible ? (m_baseGpuPtr.ptr + m_descriptorSize * index) : UINT64{0},
        };
    }

} // namespace TiRHI::DirectX12::Private
