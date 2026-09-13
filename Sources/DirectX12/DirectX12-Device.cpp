#ifndef TITANIUM_DIRECTX12_DEVICE_H
#define TITANIUM_DIRECTX12_DEVICE_H

// thanks Maxime "mrouffet" ROUFFET - main developer (maximerouffet@gmail.com)
// for FromVulkanToDirectX12
// https://github.com/mrouffet/FromVulkanToDirectX12/blob/main/Sources/mainDX12.cpp

#include <Titanium/TitaniumHeader.hpp>
#include <Titanium/Device.hpp>

#include <d3d12.h>

#include <wrl.h>
template<typename T>
using MComPtr = Microsoft::WRL::ComPtr<T>;

#include <dxgi1_6.h>
MComPtr<IDXGIFactory6> factory;

namespace TiRHI
{
    struct Device::Backend
    {
        MComPtr<ID3D12Device> device;
        MComPtr<ID3D12CommandQueue> graphicsQueue;

        struct WaitForFence
        {
            HANDLE deviceFenceEvent;
            MComPtr<ID3D12Fence> deviceFence;
            uint32_t deviceFenceValue = 1u;
        } waitForFence;
    };

    Device::Device()
    {
        UINT dxgiFactoryFlags = 0;
    }

    Device::~Device()
    {
    }

    void Device::waitForDeviceIdle()
    {
        MComPtr<ID3D12CommandQueue>& graphicsQueue = m_impl->graphicsQueue;
        Device::Backend::WaitForFence& waitForFence = m_impl->waitForFence;

        // Schedule a Signal command in the queue.
        graphicsQueue->Signal(waitForFence.deviceFence.Get(), waitForFence.deviceFenceValue);

        // Wait until the fence has been processed.
        waitForFence.deviceFence->SetEventOnCompletion(waitForFence.deviceFenceValue, waitForFence.deviceFenceEvent);
        WaitForSingleObjectEx(waitForFence.deviceFenceEvent, INFINITE, false);

        // Increment for next use.
        ++waitForFence.deviceFenceValue;
    }
}

#endif // TITANIUM_DIRECTX12_DEVICE_H
