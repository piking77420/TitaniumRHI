#ifndef TITANIUM_DIRECTX12_RHI_H
#define TITANIUM_DIRECTX12_RHI_H

// thanks Maxime "mrouffet" ROUFFET - main developer (maximerouffet@gmail.com)
// for FromVulkanToDirectX12
// https://github.com/mrouffet/FromVulkanToDirectX12/blob/main/Sources/mainDX12.cpp

#include <Titanium/TitaniumHeader.hpp>

#include <stdint.h>
#include <d3d12.h>
#include <dxgidebug.h>

#include <dxgi1_6.h>
#include <DirectX12-Header.hpp>
#include <DirectX12-Instance.hpp>

namespace TiRHI
{
    class RHI
    {
    public:
        RHI();
        ~RHI();

        void waitForDeviceIdle();

    private:
        DirectX12::Instance instance;
        MComPtr<ID3D12Device> device;
        MComPtr<ID3D12CommandQueue> graphicsQueue;

        struct WaitForFence
        {
            HANDLE deviceFenceEvent;
            MComPtr<ID3D12Fence> deviceFence;
            uint32_t deviceFenceValue = 1u;
        } waitForFence;
    };
}

#endif // TITANIUM_DIRECTX12_RHI_H
