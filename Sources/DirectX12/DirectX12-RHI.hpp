#ifndef TITANIUM_DIRECTX12_RHI_H
#define TITANIUM_DIRECTX12_RHI_H

// thanks Maxime "mrouffet" ROUFFET - main developer (maximerouffet@gmail.com)
// for FromVulkanToDirectX12
// https://github.com/mrouffet/FromVulkanToDirectX12/blob/main/Sources/mainDX12.cpp

#include <stdint.h>
#include <d3d12.h>
#include <dxgidebug.h>

#include <dxgi1_6.h>
#include <DirectX12-Header.hpp>
#include <DirectX12-Instance.hpp>
#include <DirectX12-Device.hpp>

#include <Titanium/RHI.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    class DirectX12RHI : public RHI<DirectX12RHI>
    {
    public:
        DirectX12RHI(const RhiCreate& rhiCreate);
        ~DirectX12RHI() = default;

        void waitImpl();

    private:
        DirectX12::Instance m_instance;
        DirectX12::Device m_device;
        MComPtr<ID3D12CommandQueue> m_graphicsQueue;

        struct Synchronisation
        {
            HANDLE deviceFenceEvent;
            MComPtr<ID3D12Fence> deviceFence;
            uint32_t deviceFenceValue = 1u;
        } m_synchronisation;
    };
}

#endif // TITANIUM_DIRECTX12_RHI_H
