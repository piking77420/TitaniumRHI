#ifndef TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
#define TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H

#include <bitset>
#include <memory>
#include <array>
#include <optional>
#include <tuple>
#include <vector>

#include <Titanium/Private/StrongTypeId.hpp>
#include <Titanium/RHI-Object.hpp>
#include <DirectX12/DirectX12-Header.hpp>

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
        PoolIndex poolIndex{};
        DesciptorIndex descriptorIndex{};
        size_t generation = 0;
    };

    class Device;

} // TiRHI::DirectX12

namespace TiRHI::DirectX12::Private
{
    class DescriptorAllocator
    {
    public:
        DescriptorAllocator() = default;
        RHI_MOVE_CONSTRUCT_ONLY(DescriptorAllocator);
        ~DescriptorAllocator() = default;

        bool build(Device& device);

        std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> getOrCreate(Device& device,
                                                               CpuDescriptorHandle& cpuDescriptorHandle);

        std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> get(const CpuDescriptorHandle& cpuDescriptorHandle);

        void release(CpuDescriptorHandle& cpuDescriptorHandle);

    private:
        struct Pool
        {
            Pool() = default;
            RHI_MOVE_CONSTRUCT_ONLY(Pool);
            ~Pool() = default;

            static constexpr uint64_t BaseCapacity = 64;

            MComPtr<ID3D12DescriptorHeap> rtvHeap;

            UINT rtvDescriptorSize = 0;

            D3D12_CPU_DESCRIPTOR_HANDLE baseHandlePtr;

            uint64_t m_availableMask; // 1 mean free 0 occupied

            std::array<size_t, BaseCapacity> generations;

            bool build(ID3D12Device& device);
        };

        std::vector<std::unique_ptr<Pool>> m_pools;

        std::optional<std::tuple<Pool*, PoolIndex, DesciptorIndex>> allocate(ID3D12Device& device);
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_ALLOCATOR_H
