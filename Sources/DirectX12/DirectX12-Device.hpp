#ifndef TITANIUM_DIRECTX12_DEVICE_H
#define TITANIUM_DIRECTX12_DEVICE_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseDevice.hpp>

#include <DirectX12-Header.hpp>
#include <DirectX12/Private/DescriptorAllocator.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Factory;
    class CommandList;
    class Surface;
    class AcquiredFrame;

    class Device : public BaseDevice<Device, RHI>
    {
    public:
        Device() = delete;
        ~Device();
        RHI_MOVE_ONLY(Device);
        Device(RHI& rhi);

        struct Private
        {
            static DirectX12::Private::DescriptorAllocator& getDescriptorAllocatorRTV(Device& device)
            {
                return device.m_descriptorAllocatorRtv;
            }

            static DirectX12::Private::DescriptorAllocator& getDescriptorAllocatorSampler(Device& device)
            {
                return device.m_descriptorAllocatorSampler;
            }
            // TODO
        };

        bool build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                   std::optional<size_t> index = {});

        void wait();

        void submit(std::span<const AcquiredFrame> acquiredFrame, std::span<CommandList*> commandList);

        void beginFrame();

        ID3D12Device* getNativeDevice()
        {
            return m_device.Get();
        }

        MComPtr<ID3D12CommandQueue>& getNativeGraphicQueue()
        {
            return m_graphicsQueue;
        }

        bool isValid() const
        {
            return m_device;
        }

        bool operator()() const
        {
            return isValid();
        }

    private:
        friend Private;

        MComPtr<ID3D12Device> m_device;

        DWORD VLayerCallbackCookie = 0;

        MComPtr<ID3D12CommandQueue> m_graphicsQueue;

        struct Synchronization
        {
            HANDLE waitFenceEvent;
            MComPtr<ID3D12Fence> waitFence;
            UINT64 waitFenceValue = 1u;

            HANDLE frameFenceEvent = nullptr;
            MComPtr<ID3D12Fence> frameFence;
            std::vector<UINT64> frameFenceValue{0u};
            UINT64 nextFrameFenceValue = 0;

        } m_synchronization;

        DirectX12::Private::DescriptorAllocator m_descriptorAllocatorRtv;
        DirectX12::Private::DescriptorAllocator m_descriptorAllocatorSampler;

        bool createDevice(const MComPtr<IDXGIAdapter1>& adapter1);

        // handle one queu for now
        bool createUniqueQueue();

        bool createSynchronisation();

        bool queryOptions();

        bool initDescriptorAllocator();
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DEVICE_H
