#ifndef TITANIUM_DIRECTX12_BUFFER_H
#define TITANIUM_DIRECTX12_BUFFER_H

#include <D3D12MemAlloc.h>

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseBuffer.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/Private/DescriptorAllocator.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class Buffer : public BaseBuffer<Buffer, Device>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        Buffer() = delete;
        ~Buffer();
        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;
        Buffer(Device& device)
            : BaseBuffer(device)
        {
        }

        bool build();

        bool isValid() const
        {
            return !m_allocatedBuffers.empty();
        }

        bool isInvalid() const
        {
            return !isValid();
        }

        operator bool() const
        {
            return isValid();
        }

        std::byte* beginDynamicUpdate();

        void endDynamicUpdate();

        ID3D12Resource* getNativeBufferFrame();

        const TiRHI::DirectX12::Private::DescriptorHandle* getDescriptorHandleFrame() const;

        void setLastTimeLineUsedFrame(uint64_t timeLine);

    private:
        struct AllocatedBuffer
        {
            MComPtr<D3D12MA::Allocation> allocation;
            uint64_t lastTimeLineUsed;
        };

        std::vector<AllocatedBuffer> m_allocatedBuffers;
        std::vector<std::optional<TiRHI::DirectX12::Private::DescriptorHandle>> m_constantBufferViews;

        void destroy();

        AllocatedBuffer* getAllocatedBufferFrame();

        bool createHandle();
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_BUFFER_H
