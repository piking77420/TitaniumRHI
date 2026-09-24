#include <DirectX12-Instance.hpp>

#include <string>
#include <format>
#include <Titanium/Log.hpp>
#include <Titanium/RHITypes.hpp>

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

    Instance::Instance(const RhiCreate& rhiCreate)
    {
        RHI_LOG_INFO(L"DirectX12 backend", RhiApi::DirectX12);
        Private::logCallBack = rhiCreate.logCallback; // set up global call back

        setupValidationLayer();
        createFactory();
    }

    Instance::~Instance()
    {
        RHI_LOG_INFO(L"Destroying Factory...", RhiApi::DirectX12);
        m_factory = nullptr;

#if defined(TITANIUM_VALIDATION_LAYER)

        // Report live objects
        MComPtr<IDXGIDebug1> dxgiDebug = nullptr;

        const HRESULT hrDebugInterface = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
        if (SUCCEEDED(hrDebugInterface))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        }
        else
        {
            RHI_LOG_ERROR(L"Validation layer uninitialized failed.", RhiApi::DirectX12);
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)
    }

#if defined(TITANIUM_VALIDATION_LAYER)
    D3D12MessageFunc Instance::getMessageCallBack()
    {
        return &validationLayersDebugCallback;
    }
#endif // defined(TITANIUM_VALIDATION_LAYER)

    void Instance::setupValidationLayer()
    {
#if defined(TITANIUM_VALIDATION_LAYER)
        {
            // init debug controller
            MComPtr<ID3D12Debug1> debugController = nullptr;

            const HRESULT hrDebugInterface = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
            if (SUCCEEDED(hrDebugInterface))
            {
                debugController->EnableDebugLayer();
                debugController->SetEnableGPUBasedValidation(true);
            }
            else
            {
                RHI_LOG_ERROR(std::format(L"Validation layer DebugController initialization failed \n Error Code: {}",
                                          hrDebugInterface),
                              RhiApi::DirectX12);
            }
        }

        {
            // Report live objects
            {
                MComPtr<IDXGIInfoQueue> dxgiInfoQueue = nullptr;

                const HRESULT hrDebugInterface = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue));
                if (SUCCEEDED(hrDebugInterface))
                {
                    /**
                     * Enable this to trigger breakpoints on ReportLiveObjects() and have errors in VisualStudio's
                     * output window. WARNING: ReportLiveObjects() will ONLY output in VisualStudio's output window
                     * and not exe CONSOLE.
                     */
                    dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION,
                                                      true);
                    dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                    dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);
                }
                else
                {
                    RHI_LOG_ERROR(
                        std::format(L"Validation layer DebugController initialization failed \n Error Code: {} \n",
                                    hrDebugInterface),
                        RhiApi::DirectX12);
                }
            }

            // Enable additional debug layers.
            m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)
    }

    void Instance::createFactory()
    {
        const HRESULT hrFactoryCreated = CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));
        if (FAILED(hrFactoryCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Factory failed {}", hrFactoryCreated), RhiApi::DirectX12);
        }
        else
        {
            RHI_LOG_INFO(L"Create Factory success", RhiApi::DirectX12);
        }
    }
} // namespace TiRHI::DirectX12
