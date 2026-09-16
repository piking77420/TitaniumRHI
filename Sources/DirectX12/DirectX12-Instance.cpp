#include <DirectX12-Instance.hpp>

#include <string>
#include <format>
#include <print>

namespace TiRHI::DirectX12
{
#if defined(TITANIUM_VALIDATION_LAYER)
    void validationLayersDebugCallback(D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
                                       D3D12_MESSAGE_ID iD, LPCSTR description, [[mayeb_unused]] void* context)
    {
        std::string categoryStr;

        switch (category)
        {
        case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:
            categoryStr = "Application Defined";
            break;
        case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:
            categoryStr = "Miscellaneous";
            break;
        case D3D12_MESSAGE_CATEGORY_INITIALIZATION:
            categoryStr = "Initialization";
            break;
        case D3D12_MESSAGE_CATEGORY_CLEANUP:
            categoryStr = "Cleanup";
            break;
        case D3D12_MESSAGE_CATEGORY_COMPILATION:
            categoryStr = "Compilation";
            break;
        case D3D12_MESSAGE_CATEGORY_STATE_CREATION:
            categoryStr = "State Creation";
            break;
        case D3D12_MESSAGE_CATEGORY_STATE_SETTING:
            categoryStr = "State Setting";
            break;
        case D3D12_MESSAGE_CATEGORY_STATE_GETTING:
            categoryStr = "State Getting";
            break;
        case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
            categoryStr = "Resource Manipulation";
            break;
        case D3D12_MESSAGE_CATEGORY_EXECUTION:
            categoryStr = "Execution";
            break;
        case D3D12_MESSAGE_CATEGORY_SHADER:
            categoryStr = "Shader";
            break;
        default:
            categoryStr = "Unknown";
            break;
        }

        std::string dets = std::format("ID[{}]\tCategory[{}]", static_cast<int>(iD), categoryStr);

        switch (severity)
        {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            std::println("corruption : {}", dets);
            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            std::println("Error : {}", dets);
            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            std::println("warning : {}", dets);
            break;
        case D3D12_MESSAGE_SEVERITY_INFO:
            return;
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
        default:
            std::println("{}", dets);
            break;
        }
    }
#endif

    Instance::Instance()
    {
        std::println("DirectX12 backend \n");

        setupValidationLayer();
        createFactory();
    }

    Instance::~Instance()
    {
        std::println("Destroying Factory... {}", static_cast<void*>(m_factory.Get()));
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
            std::println("Validation layer uninitialized failed.");
        }
#endif // defined(TITANIUM_VALIDATION_LAYER)
    }

    D3D12MessageFunc Instance::getMessageCallBack()
    {
        return &validationLayersDebugCallback;
    }

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
                std::println("Validation layer DebugController initialization failed \n Error Code: {} \n",
                             hrDebugInterface);
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
                    std::println("Validation layer DebugInfoQueue uninitialized failed. \n");
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
            std::println("Create Factory failed {} \n", hrFactoryCreated);
        }
        else
        {
            std::println("Create Factory success \n");
        }
    }
} // namespace TiRHI::DirectX12
