#include <DirectX12-Device.hpp>

#include <format>

#include <Titanium/Log.hpp>

namespace TiRHI::DirectX12
{

#if defined(TITANIUM_VALIDATION_LAYER)
    void validationLayersDebugCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
                                       D3D12_MESSAGE_ID iD, LPCSTR description, [[mayeb_unused]] void* context)
    {
        using namespace std::literals;

        std::wstring_view categoryStr;

        switch (category)
        {
        case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:
            categoryStr = L"Application Defined"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:
            categoryStr = L"Miscellaneous"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_INITIALIZATION:
            categoryStr = L"Initialization"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_CLEANUP:
            categoryStr = L"Cleanup"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_COMPILATION:
            categoryStr = L"Compilation"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_STATE_CREATION:
            categoryStr = L"State Creation"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_STATE_SETTING:
            categoryStr = L"State Setting"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_STATE_GETTING:
            categoryStr = L"State Getting"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
            categoryStr = L"Resource Manipulation"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_EXECUTION:
            categoryStr = L"Execution"sv;
            break;
        case D3D12_MESSAGE_CATEGORY_SHADER:
            categoryStr = L"Shader"sv;
            break;
        default:
            categoryStr = L"Unknown"sv;
            break;
        }

        std::wstring dets = std::format(L"ID[{}]\tCategory[{}]", static_cast<int>(iD), categoryStr);

        switch (severity)
        {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            RHI_LOG_FATAL(std::format(L"Validation Layer: {}", dets), RhiApi::DirectX12);
            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            RHI_LOG_ERROR(std::format(L"Validation Layer: {}", dets), RhiApi::DirectX12);
            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            RHI_LOG_WARNING(std::format(L"Validation Layer: {}", dets), RhiApi::DirectX12);
            break;
        case D3D12_MESSAGE_SEVERITY_INFO:
            return;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
        default:
            RHI_LOG_INFO(std::format(L"Validation Layer: {}", dets), RhiApi::DirectX12);
            break;
        }
    }
#endif

    Device::Device(MComPtr<IDXGIFactory6>& factory)
    {
        MComPtr<IDXGIAdapter3> adapter;

        const HRESULT hrQueryGPU =
            factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
        if (FAILED(hrQueryGPU))
        {
            RHI_LOG_ERROR(std::format(L"Adapter not found! \n Error Code: {}", hrQueryGPU), RhiApi::DirectX12);
            return;
        }

        const HRESULT hrDeviceCreated =
            D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        if (FAILED(hrDeviceCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Device failed! \n Error Code: {}", hrDeviceCreated), RhiApi::DirectX12);
            return;
        }
        else
        {
            const LPCWSTR name = L"Main Device";
            m_device->SetName(name);
            RHI_LOG_INFO(std::format(L"Create Device Success! Name: {}", name), RhiApi::DirectX12);
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
                infoQueue->RegisterMessageCallback(&validationLayersDebugCallback,
                                                   D3D12_MESSAGE_CALLBACK_IGNORE_FILTERS, nullptr,
                                                   &VLayerCallbackCookie);

                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            }
            else
            {
                RHI_LOG_INFO(
                    std::format(L"Device query info queue to enable validation layers failed. \n Error Code: {}",
                                hrQueryInfoQueue),
                    RhiApi::DirectX12);
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

        RHI_LOG_INFO(std::format(L"Destroy Device... "), RhiApi::DirectX12);
        m_device = nullptr;
    }
}
