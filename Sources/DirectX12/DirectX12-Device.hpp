#ifndef TITANIUM_DIRECTX12_DEVICE_H
#define TITANIUM_DIRECTX12_DEVICE_H

#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <dxgi1_4.h>

#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseDevice.hpp>

#include <DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class Factory;

    class Device : public BaseDevice
    {
    public:
        Device() = default;
        ~Device();
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) noexcept = default;
        Device& operator=(Device&&) noexcept = default;
        Device(Factory& factory, const std::vector<MComPtr<IDXGIAdapter1>>& dxAdapters,
               const std::vector<Adapter>& adapters);
        Device(Factory& factory, const std::vector<MComPtr<IDXGIAdapter1>>& dxAdapters,
               const std::vector<Adapter>& adapters, size_t index);

        void wait();

        MComPtr<ID3D12Device>& getNativeHandle()
        {
            return m_device;
        }

    private:
        MComPtr<ID3D12Device> m_device;

        DWORD VLayerCallbackCookie = 0;

        void create(MComPtr<IDXGIFactory6>& factory, const MComPtr<IDXGIAdapter1>& adapter1);
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DEVICE_H
