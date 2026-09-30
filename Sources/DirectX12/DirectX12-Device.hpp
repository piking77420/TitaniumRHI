#ifndef TITANIUM_DIRECTX12_DEVICE_H
#define TITANIUM_DIRECTX12_DEVICE_H

#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <dxgi1_4.h>

#include <Titanium/RHI-Adapter.hpp>

#include <DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class Device
    {
    public:
        Device(MComPtr<IDXGIFactory6>& factory, std::vector<Adapter>& adatpers);
        ~Device();

        MComPtr<ID3D12Device>& getDevice()
        {
            return m_device;
        }

        size_t getSelectPhyscialDeviceIndex() const
        {
            return m_selectedDeviceIndex;
        }

        void createDevice(const std::vector<Adapter>& adapter, MComPtr<IDXGIFactory6>& factory);

        void createDevice(size_t adapterIndex, MComPtr<IDXGIFactory6>& factory);

    private:
        MComPtr<ID3D12Device> m_device;

        DWORD VLayerCallbackCookie = 0;

        size_t m_selectedDeviceIndex = 0;

        std::vector<Adapter> enumerateAvailableAdapter(MComPtr<IDXGIFactory6>& factory);

        void createFromAdaptater(MComPtr<IDXGIFactory6>& factory, IDXGIAdapter3* adapter);
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DEVICE_H
