#include <DirectX12/DirectX12-Buffer.hpp>
#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RHI.hpp>

namespace TiRHI::DirectX12
{
    Buffer ::~Buffer()
    {
        destroy();
    };

    Buffer::Buffer(Buffer&& other) noexcept
        : BaseBuffer(std::move(other))
        , m_allocatedBuffers(std::exchange(other.m_allocatedBuffers, {}))
    {
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();
        BaseBuffer::operator=(std::move(other));
        m_allocatedBuffers = std::exchange(other.m_allocatedBuffers, {});

        return *this;
    }

    bool Buffer::build()
    {
        if (getSize() == 0 || getUsage() == BufferUsage::None)
            return false;

        D3D12MA::Allocator& allocator = Device::Private::getAllocator(getDevice());

        D3D12MA::ALLOCATION_DESC allocDesc{};

        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

        switch (getMemoryType())
        {
        case BufferMemoryType::Default:
            allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            initialState = D3D12_RESOURCE_STATE_COMMON;
            break;

        case BufferMemoryType::Upload:
            allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
            initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
            break;

        case BufferMemoryType::Readback:
            allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
            initialState = D3D12_RESOURCE_STATE_COPY_DEST;
            break;

        default:
            RHI_LOG_ERROR(L"Unknown buffer memory type", Api);
            return false;
        }

        D3D12_RESOURCE_DESC bufferDesc{};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = static_cast<UINT64>(getSize());
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        const uint32_t bufferCount = getUpdateMode() == BufferUpdateMode::Static ? 1 : getRHI().getFrameInFlight();

        if (bufferCount == 0)
            return false;

        m_allocatedBuffers.resize(bufferCount);

        for (AllocatedBuffer& allocatedBuffer : m_allocatedBuffers)
        {
            const HRESULT hr = allocator.CreateResource(&allocDesc, &bufferDesc, initialState, nullptr,
                                                        allocatedBuffer.allocation.GetAddressOf(), IID_NULL, nullptr);

            if (FAILED(hr))
            {
                RHI_LOG_ERROR(std::format(L"Buffer {}, failed to allocate: HRESULT {}", getNameW(), hr), Api);

                m_allocatedBuffers.clear();
                return false;
            }
        }

        return createHandle();
    }

    std::byte* Buffer::beginDynamicUpdate()
    {
        if (getSize() == 0)
            return nullptr;

        if (getMemoryType() != BufferMemoryType::Upload)
        {
            RHI_LOG_ERROR(
                std::format(L"Buffer {}, to dynamic update an buffer the buffer should use BufferMemoryType::Upload",
                            getNameW()),
                Api);

            return nullptr;
        }

        AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();
        if (!allocatedBuffer)
            return nullptr;

        ID3D12Device* d3d12Device = getDevice().getNativeDevice().Get();

        D3D12_RANGE readRange = {0, 0};
        void* mapped = nullptr;
        ID3D12Resource* uploadBuffer = allocatedBuffer->allocation->GetResource();
        const HRESULT hr = uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mapped));

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"Buffer {}, faile to map buffer", getNameW()), Api);
            return nullptr;
        }

        return static_cast<std::byte*>(mapped);
    }

    void Buffer::endDynamicUpdate()
    {
        if (getSize() == 0)
            return;

        AllocatedBuffer* allocatedBuffer = getAllocatedBufferFrame();
        if (!allocatedBuffer || !allocatedBuffer->allocation)
            return;

        allocatedBuffer->allocation->GetResource()->Unmap(0, nullptr);
    }

    ID3D12Resource* Buffer::getNativeBufferFrame()
    {
        const size_t index = getCurrentAllocatedFrameIndex();

        return m_allocatedBuffers.size() >= (index + 1) ? m_allocatedBuffers[index].allocation->GetResource() : nullptr;
    }

    const TiRHI::DirectX12::Private::DescriptorHandle* Buffer::getDescriptorHandleFrame() const
    {
        const size_t index = getCurrentAllocatedFrameIndex();

        if (index >= m_constantBufferViews.size())
            return nullptr;

        const auto& descriptor = m_constantBufferViews[index];

        return descriptor ? &descriptor.value() : nullptr;
    }

    uint32_t Buffer::getCurrentAllocatedFrameIndex() const
    {
        return getUpdateMode() == BufferUpdateMode::Static ? 0 : getRHI().getCurrentFrame();
    }

    Buffer& Buffer::setLastFrameUsed(uint64_t value, uint32_t frameIndex)
    {
        assert(frameIndex < m_allocatedBuffers.size());
        if (frameIndex >= m_allocatedBuffers.size())
        {
            RHI_LOG_FATAL(std::format(L"Buffer {}, frame index outside of bound", getNameW()), Api);
        }
        m_allocatedBuffers[frameIndex].setLastFrameUsed(value);
        return *this;
    }

    void Buffer::destroy()
    {
        TiRHI::Private::DeferredDeletionQueue& m_deferredDeletionQueue =
            Device::Private::getDeferredDeletionQueue(getDevice());

        for (AllocatedBuffer& allocatedBuffer : m_allocatedBuffers)
        {
            m_deferredDeletionQueue.retire(allocatedBuffer.getLastFrameUsed(),
                                         [allocation = std::move(allocatedBuffer.allocation)]() mutable
                                         { allocation.Reset(); });
        }
    }

    Buffer::AllocatedBuffer* Buffer::getAllocatedBufferFrame()
    {
        const size_t index = getUpdateMode() == BufferUpdateMode::Static ? 0 : getRHI().getCurrentFrame();
        return m_allocatedBuffers.size() >= (index + 1) ? &m_allocatedBuffers[index] : nullptr;
    }

    bool Buffer::createHandle()
    {
        m_constantBufferViews.reserve(m_allocatedBuffers.size());

        for (size_t i = 0; i < m_allocatedBuffers.size(); i++)
        {
            auto& alloc = m_allocatedBuffers[i];
            if (!alloc.allocation)
                return false;

            if ((getUsage() & BufferUsage::Constant) != BufferUsage::None)
            {
                auto& handle = m_constantBufferViews.emplace_back(
                    Device::Private::getDescriptorAllocatorCbvSrvUav(getDevice()).create(getDevice()));
                if (!handle)
                    return false;

                D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
                desc.BufferLocation = alloc.allocation->GetResource()->GetGPUVirtualAddress();
                desc.SizeInBytes = getSize();
                getDevice().getNativeDevice()->CreateConstantBufferView(&desc, handle->cpuHandle);
            }
        }
        return true;
    }

} // namespace TiRHI::DirectX12
