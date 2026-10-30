#ifndef TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
#define TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <bitset>
#include <memory>
#include <array>
#include <optional>
#include <tuple>
#include <vector>
#include <utility>

#include <Titanium/Private/StrongTypeId.hpp>
#include <Titanium/RHI-Object.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12::Private
{
    class DescriptorAllocator;
}

namespace TiRHI::DirectX12
{
    struct DescriptorIndexTag
    {
    };
    struct PoolIndexTag
    {
    };

    using DesciptorIndex = TiRHI::Private::StrongTypeID<size_t, DescriptorIndexTag>;
    using PoolIndex = TiRHI::Private::StrongTypeID<size_t, PoolIndexTag>;

    struct CpuDescriptorHandle
    {
        explicit CpuDescriptorHandle(Private::DescriptorAllocator& allocator)
            : descriptorAllocator(&allocator)
        {
        }

        ~CpuDescriptorHandle()
        {
            release();
        }

        CpuDescriptorHandle(const CpuDescriptorHandle&) = delete;
        CpuDescriptorHandle& operator=(const CpuDescriptorHandle&) = delete;

        CpuDescriptorHandle(CpuDescriptorHandle&& other) noexcept
            : descriptorAllocator(std::exchange(other.descriptorAllocator, nullptr))
            , poolIndex(std::exchange(other.poolIndex, PoolIndex{}))
            , descriptorIndex(std::exchange(other.descriptorIndex, DesciptorIndex{}))
            , generation(std::exchange(other.generation, 0))
            , nativeHandle(std::exchange(other.nativeHandle, D3D12_CPU_DESCRIPTOR_HANDLE{0}))
        {
        }

        CpuDescriptorHandle& operator=(CpuDescriptorHandle&& other) noexcept
        {
            if (this != &other)
            {
                release();

                descriptorAllocator = std::exchange(other.descriptorAllocator, nullptr);
                poolIndex = std::exchange(other.poolIndex, PoolIndex{});
                descriptorIndex = std::exchange(other.descriptorIndex, DesciptorIndex{});
                generation = std::exchange(other.generation, 0);
                nativeHandle = std::exchange(other.nativeHandle, D3D12_CPU_DESCRIPTOR_HANDLE{0});
            }

            return *this;
        }

        void release() noexcept;

        Private::DescriptorAllocator* descriptorAllocator = nullptr;
        PoolIndex poolIndex{};
        DesciptorIndex descriptorIndex{};
        size_t generation = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE nativeHandle{0};
    };

    class Device;

} // TiRHI::DirectX12

namespace TiRHI::DirectX12::Private
{
    class DescriptorAllocator
    {
    public:
        DescriptorAllocator() = default;
        RHI_MOVE_ONLY(DescriptorAllocator);
        ~DescriptorAllocator() = default;

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

        std::optional<CpuDescriptorHandle> create(Device& device);

        void release(CpuDescriptorHandle& cpuDescriptorHandle);

    private:
        struct Pool
        {
            Pool() = default;
            RHI_MOVE_ONLY(Pool);
            ~Pool() = default;

            static constexpr uint64_t BaseCapacity = 64;

            MComPtr<ID3D12DescriptorHeap> rtvHeap;

            UINT rtvDescriptorSize = 0;

            D3D12_CPU_DESCRIPTOR_HANDLE baseHandlePtr;

            uint64_t m_availableMask; // 1 mean free 0 occupied

            std::array<size_t, BaseCapacity> generations;

            bool build(ID3D12Device& device, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType);
        };

        std::vector<std::unique_ptr<Pool>> m_pools;

        D3D12_DESCRIPTOR_HEAP_TYPE m_descriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        std::optional<std::tuple<Pool*, PoolIndex, DesciptorIndex>> allocate(ID3D12Device& device);
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
