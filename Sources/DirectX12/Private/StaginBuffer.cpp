#include "StaginBuffer.hpp"
#include <DirectX12/Private/StaginBuffer.hpp>
#include <DirectX12/DirectX12-Device.hpp>
#include <Titanium/Private/DeferredDeletionQueue.hpp>

namespace TiRHI::DirectX12::Private
{
    StagingBuffer::~StagingBuffer()
    {
        reset();
    }

    StagingBuffer::StagingBuffer(StagingBuffer&& other) noexcept
        : m_deferredDeletionQueue(other.m_deferredDeletionQueue)
        , m_allocation(std::move(other.m_allocation))
        , m_mappedData(std::exchange(other.m_mappedData, nullptr))
        , m_size(std::exchange(other.m_size, 0))
    {
    }

    StagingBuffer::StagingBuffer(Device& device, uint64_t size, uint64_t timeLineFrame)
        : m_deferredDeletionQueue(Device::Private::getDeferredDeletionQueue(device))
        , m_timeLineFrame(timeLineFrame)
        , m_size(size)
    {
        D3D12MA::Allocator& allocator = Device::Private::getAllocator(device);

        D3D12MA::ALLOCATION_DESC allocDesc{};
        allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = m_size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        const HRESULT hr = allocator.CreateResource(&allocDesc, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                    m_allocation.GetAddressOf(), IID_NULL, nullptr);

        if (FAILED(hr))
        {
            m_allocation.Reset();
            reset();
        }
    }

    std::byte* StagingBuffer::mapData() noexcept
    {
        // CPU will write, not read.
        D3D12_RANGE readRange{0, 0};

        void* mapped = nullptr;

        const HRESULT mapHr = getBuffer()->Map(0, &readRange, &mapped);

        if (FAILED(mapHr))
        {
            reset();
            return nullptr;
        }

        m_mappedData = static_cast<std::byte*>(mapped);

        return m_mappedData;
    }

    void StagingBuffer::unmapData()
    {
        if (m_mappedData && m_allocation)
        {
            getBuffer()->Unmap(0, nullptr);
            m_mappedData = nullptr;
        }
    }

    void StagingBuffer::reset()
    {
        if (m_allocation)
        {
            m_deferredDeletionQueue.retire(m_timeLineFrame,
                                           [allocation = std::move(m_allocation)]() mutable { allocation.Reset(); });
            m_mappedData = nullptr;
            m_size = 0;
            m_timeLineFrame = 0;
        }
    }
}
