#ifndef TITANIUM_DIRECTX12_PRIVATE_STAGING_BUFFER_H
#define TITANIUM_DIRECTX12_PRIVATE_STAGING_BUFFER_H

#include <vector>
#include <D3D12MemAlloc.h>

#include <Titanium/Private/FrameTracked.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace D3D12MA
{
    class D3D12MA;
} // D3D12MA

namespace TiRHI::Private
{
    class DeferredDeletionQueue;
}

namespace TiRHI::DirectX12
{
    class Device;
}

namespace TiRHI::DirectX12::Private
{
    class StagingBuffer : public TiRHI::Private::FrameTracked<StagingBuffer>
    {
    public:
        StagingBuffer() = delete;
        ~StagingBuffer();

        StagingBuffer(const StagingBuffer&) = delete;
        StagingBuffer& operator=(const StagingBuffer&) = delete;

        StagingBuffer(StagingBuffer&& other) noexcept;
        StagingBuffer& operator=(StagingBuffer&& other) noexcept = delete;

        StagingBuffer(Device& device, uint64_t size, uint64_t timeLineFrame);

        ID3D12Resource* getBuffer() const noexcept
        {
            return m_allocation ? m_allocation->GetResource() : nullptr;
        }

        std::byte* mapData() noexcept;

        void unmapData();

        uint64_t getSize() const noexcept
        {
            return m_size;
        }

        explicit operator bool() const noexcept
        {
            return m_allocation;
        }

    private:
        TiRHI::Private::DeferredDeletionQueue& m_deferredDeletionQueue;
        MComPtr<D3D12MA::Allocation> m_allocation;
        std::byte* m_mappedData = nullptr;
        uint64_t m_size = 0;

        void reset();
    };
}

#endif // TITANIUM_DIRECTX12_PRIVATE_STAGING_BUFFER_H
