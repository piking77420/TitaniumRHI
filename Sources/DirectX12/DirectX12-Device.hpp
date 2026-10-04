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

        bool build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                   std::optional<size_t> index = {});

        void wait();

        ID3D12Device* getNativeDevice()
        {
            return m_device.Get();
        }

        MComPtr<ID3D12CommandQueue>& getNativeGraphicQueue()
        {
            return m_graphicsQueue;
        }

        void submit(const AcquiredFrame& AcquiredFrame, CommandList& commandList);

    private:
        MComPtr<ID3D12Device> m_device;

        DWORD VLayerCallbackCookie = 0;

        MComPtr<ID3D12CommandQueue> m_graphicsQueue;

        struct Synchronization
        {
            HANDLE deviceFenceEvent;
            MComPtr<ID3D12Fence> deviceFence;
            uint64_t deviceFenceValue = 1u;
        } m_synchronization;

        bool createDevice(const MComPtr<IDXGIAdapter1>& adapter1);

        // handle one queu for now
        bool createUniqueQueue();

        bool createSynchronisation();
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DEVICE_H
