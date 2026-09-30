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

    std::vector<Adapter::Features> getAdapterFeatures(ID3D12Device* device)
    {
        std::vector<Adapter::Features> features;

        // Ray tracing / DXR
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5{};

        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
        {
            if (options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
            {
                features.push_back(Adapter::Features::RayTracing);
            }
        }

        // Mesh shaders
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7{};

        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
        {
            if (options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
            {
                features.push_back(Adapter::Features::MeshShader);
            }
        }

        return features;
    }

    Adapter::Properties getAdapterProperties(const DXGI_ADAPTER_DESC1& desc, ID3D12Device* device)
    {
        Adapter::Properties rhiProperty{};
        rhiProperty.limits.minUniformBufferOffset = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

        rhiProperty.memoryLimits.vramMemoryBytes = static_cast<uint64_t>(desc.DedicatedVideoMemory);

        D3D12_FEATURE_DATA_ARCHITECTURE1 architecture{};
        architecture.NodeIndex = 0;

        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &architecture, sizeof(architecture))))
        {
            rhiProperty.deviceType =
                architecture.UMA ? Adapter::Properties::Type::IntegratedGpu : Adapter::Properties::Type::DiscreteGpu;
        }
        else
        {
            rhiProperty.deviceType = Adapter::Properties::Type::Unknow;
        }

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            rhiProperty.deviceType = Adapter::Properties::Type::Cpu;
        }

        return rhiProperty;
    }

    std::vector<Adapter> Device::enumerateAvailableAdapter(MComPtr<IDXGIFactory6>& factory)
    {
        std::vector<Adapter> adapter;

        for (UINT i = 0;; ++i)
        {
            MComPtr<IDXGIAdapter1> adapter1;

            if (factory->EnumAdapters1(i, &adapter1) == DXGI_ERROR_NOT_FOUND)
                break;

            DXGI_ADAPTER_DESC1 desc{};
            adapter1->GetDesc1(&desc);

            std::wstring_view wideName = desc.Description;
            std::string name(wideName.begin(), wideName.end());

            MComPtr<ID3D12Device> device;

            if (FAILED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
            {
                RHI_LOG_ERROR(std::format(L"Failed to Create fake device to check featurs of the adapter {}", wideName),
                              RhiApi::DirectX12);
                continue;
            }

            adapter.emplace_back(name, getAdapterFeatures(device.Get()), getAdapterProperties(desc, device.Get()),
                                 desc.VendorId);
        }

        return adapter;
    }

    Device::Device(MComPtr<IDXGIFactory6>& factory, std::vector<Adapter>& adatpers)
    {
        adatpers = enumerateAvailableAdapter(factory);
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

        m_device = nullptr;
    }

    void Device::createDevice(const std::vector<Adapter>& adapter, MComPtr<IDXGIFactory6>& factory)
    {
        MComPtr<IDXGIAdapter3> adapter3;

        const HRESULT hrQueryGPU =
            factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter3));
        if (FAILED(hrQueryGPU))
        {
            RHI_LOG_ERROR(std::format(L"Adapter not found! \n Error Code: {}", hrQueryGPU), RhiApi::DirectX12);
            return;
        }

        {
            DXGI_ADAPTER_DESC desc{};
            adapter3->GetDesc(&desc);

            size_t i = 0;
            for (; i < adapter.size(); i++)
            {
                std::wstring_view wideName = desc.Description;
                std::string name(wideName.begin(), wideName.end());
                if (name == adapter[i].getName())
                    break;
            }
            m_selectedDeviceIndex = i;
        }

        createFromAdaptater(factory, adapter3.Get());
    }

    void Device::createDevice(size_t adapterIndex, MComPtr<IDXGIFactory6>& factory)
    {
        MComPtr<IDXGIAdapter3> adapter3;

        for (UINT i = 0;; ++i)
        {
            MComPtr<IDXGIAdapter1> adapter1;

            const HRESULT hr = factory->EnumAdapters1(i, &adapter1);

            if (hr == DXGI_ERROR_NOT_FOUND)
                break;

            if (FAILED(hr))
                break;

            if (i != adapterIndex)
                continue;

            if (FAILED(adapter1.As(&adapter3)))
                break;

            break;
        }

        if (!adapter3)
        {
            RHI_LOG_ERROR(L"Failed to find adapter requires by the user", RhiApi::DirectX12);
            return;
        }

        m_selectedDeviceIndex = adapterIndex;
        createFromAdaptater(factory, adapter3.Get());
    }

    void Device::createFromAdaptater(MComPtr<IDXGIFactory6>& factory, IDXGIAdapter3* adapter)
    {
        const HRESULT hrDeviceCreated = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
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
}
