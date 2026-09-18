#ifndef TITANIUM_DIRECTX12_DEVICE_H
#define TITANIUM_DIRECTX12_DEVICE_H

#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <dxgi1_4.h>

#include <DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class Instance;

    class Device
    {
    public:
        Device(Instance& instance);
        ~Device();

        MComPtr<ID3D12Device>& getDevice()
        {
            return m_device;
        }

    private:
        MComPtr<ID3D12Device> m_device;

        DWORD VLayerCallbackCookie = 0;
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DEVICE_H
