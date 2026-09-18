#include "DirectX12-Device.hpp"

#include <print>
#include <DirectX12/DirectX12-Instance.hpp>

namespace TiRHI::DirectX12
{
    Device::Device(Instance& instance)
    {
        MComPtr<IDXGIFactory6>& factory = instance.getFactory();
        MComPtr<IDXGIAdapter3> adapter;

        const HRESULT hrQueryGPU =
            factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
        if (FAILED(hrQueryGPU))
        {
            std::println("Adapter not found! \n Error Code: {}", hrQueryGPU);
            return;
        }

        const HRESULT hrDeviceCreated =
            D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        if (FAILED(hrDeviceCreated))
        {
            std::println("Create Device failed! \n Error Code: {}", hrDeviceCreated);
            return;
        }
        else
        {
            const LPCWSTR name = L"Main Device";
            m_device->SetName(name);
            std::println("Create Device success. \n{}", std::string(name, name + lstrlenW(name)).data());
        }

#if defined(TITANIUM_VALIDATION_LAYER)
        // Validation Layers (device-level) /* 0002-1 */
        {
            MComPtr<ID3D12InfoQueue1> infoQueue = nullptr;

            const HRESULT hrQueryInfoQueue = m_device->QueryInterface(IID_PPV_ARGS(&infoQueue));
            if (SUCCEEDED(hrQueryInfoQueue))
            {
                /**
                 * Cookie must be provided to properly register message callback (and unregister later).
                 * Set nullptr as cookie will not crash (and no error) but won't work.
                 */
                infoQueue->RegisterMessageCallback(instance.getMessageCallBack(), D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS,
                                                   nullptr, &VLayerCallbackCookie);

                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            }
            else
            {
                std::println("Device query info queue to enable validation layers failed. \n Error Code: {}",
                             hrQueryInfoQueue);
            }
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)
    }

    Device::~Device()
    {
#if defined(TITANIUM_VALIDATION_LAYER)
        // Validation Layers (device-level)
        if (VLayerCallbackCookie)
        {
            MComPtr<ID3D12InfoQueue1> infoQueue = nullptr;

            const HRESULT hrQueryInfoQueue = m_device->QueryInterface(IID_PPV_ARGS(&infoQueue));
            if (SUCCEEDED(hrQueryInfoQueue))
            {
                infoQueue->UnregisterMessageCallback(VLayerCallbackCookie);
                VLayerCallbackCookie = 0;
            }
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)
        std::println("Destroy Device... {}", static_cast<void*>(m_device.Get()));
        m_device = nullptr;
    }
}
