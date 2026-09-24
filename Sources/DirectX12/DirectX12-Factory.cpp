#include <DirectX12-Factory.hpp>

#include <string>
#include <format>
#include <Titanium/Log.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::DirectX12
{
    Factory::Factory()
    {
        setupValidationLayer();
        createFactory();
    }

    Factory::~Factory()
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

    void Factory::setupValidationLayer()
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

    void Factory::createFactory()
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
