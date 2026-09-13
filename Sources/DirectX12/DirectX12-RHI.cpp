#ifndef TITANIUM_DIRECTX12_RHI_H
#define TITANIUM_DIRECTX12_RHI_H

// thanks Maxime "mrouffet" ROUFFET - main developer (maximerouffet@gmail.com)
// for FromVulkanToDirectX12
// https://github.com/mrouffet/FromVulkanToDirectX12/blob/main/Sources/mainDX12.cpp

#include <Titanium/TitaniumHeader.hpp>
#include <Titanium/RHI.hpp>

#include <d3d12.h>
#include <dxgidebug.h>

#include <dxgi1_6.h>
#include <DirectX12-Header.hpp>
#include <DirectX12-Instance.hpp>

namespace TiRHI
{
    MComPtr<IDXGIFactory6> factory;

    struct RHI::Backend
    {
        DirectX12::Instance instance;
        MComPtr<ID3D12Device> device;
        MComPtr<ID3D12CommandQueue> graphicsQueue;

        struct WaitForFence
        {
            HANDLE deviceFenceEvent;
            MComPtr<ID3D12Fence> deviceFence;
            uint32_t deviceFenceValue = 1u;
        } waitForFence;

        void createFactory();
    };

    RHI::RHI()
        : m_impl(std::make_unique<RHI::Backend>())
    {
    }

    RHI::~RHI()
    {
    }

    void RHI::waitForDeviceIdle()
    {
        MComPtr<ID3D12CommandQueue>& graphicsQueue = m_impl->graphicsQueue;
        RHI::Backend::WaitForFence& waitForFence = m_impl->waitForFence;

        // Schedule a Signal command in the queue.
        graphicsQueue->Signal(waitForFence.deviceFence.Get(), waitForFence.deviceFenceValue);

        // Wait until the fence has been processed.
        waitForFence.deviceFence->SetEventOnCompletion(waitForFence.deviceFenceValue, waitForFence.deviceFenceEvent);
        WaitForSingleObjectEx(waitForFence.deviceFenceEvent, INFINITE, false);

        // Increment for next use.
        ++waitForFence.deviceFenceValue;
    }
}

#endif // TITANIUM_DIRECTX12_RHI_H
