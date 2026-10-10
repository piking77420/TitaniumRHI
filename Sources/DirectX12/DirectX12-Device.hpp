#ifndef TITANIUM_DIRECTX12_DEVICE_H
#define TITANIUM_DIRECTX12_DEVICE_H

#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseDevice.hpp>

#include <DirectX12-Header.hpp>

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
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) noexcept = default;
        Device& operator=(Device&&) noexcept = default;
        Device(RHI& rhi);

        struct Private
        {
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

        bool createDevice(const MComPtr<IDXGIAdapter1>& adapter1);

        // handle one queu for now
        bool createUniqueQueue();

        bool createSynchronisation();

        bool queryOptions();
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DEVICE_H
