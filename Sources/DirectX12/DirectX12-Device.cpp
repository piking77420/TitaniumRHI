#include <DirectX12-Device.hpp>

#include <format>

#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Utils.hpp>
#include <DirectX12/DirectX12-CommandList.hpp>

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

    Device::Device(RHI& rhi)
        : BaseDevice(rhi)
    {
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

        if (m_synchronization.deviceFenceEvent)
        {
            CloseHandle(m_synchronization.deviceFenceEvent);
        }

        m_device = nullptr;
    }

    bool Device::build(RHI& rhi, [[maybe_unused]] Surface& surface, const std::span<const Adapter>& adapters,
                       std::optional<size_t> index)
    {
        if (IDXGIFactory6* factory = getRHI().getNativeFactory())
        {
            const size_t adaptaterIndex = index ? *index : BaseDevice::getBestAdapter(adapters);

            const auto& nativeAdapter = Internal::getAllNativeAdapters(factory);
#undef min;
            if (adaptaterIndex >= nativeAdapter.size())
                return false;

            return createDevice(nativeAdapter[adaptaterIndex]);
        }
        RHI_LOG_FATAL(L"Factory was null in device creating", RhiApi::DirectX12);
        return false;
    }

    bool Device::createDevice(const MComPtr<IDXGIAdapter1>& adapter1)
    {
        MComPtr<IDXGIAdapter3> adapter;
        HRESULT hr = adapter1.As(&adapter);
        if (FAILED(hr))
        {
            // IDXGIAdapter1 does not expose IDXGIAdapter3
            RHI_LOG_ERROR(L"Failed to create an IDXGIAdapter3 from IDXGIAdapter1", RhiApi::DirectX12);
            return false;
        }

        const HRESULT hrDeviceCreated =
            D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
        if (FAILED(hrDeviceCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Device failed! \n Error Code: {}", hrDeviceCreated), RhiApi::DirectX12);
            return false;
        }
        else
        {
            const std::wstring name = getNameW().empty() ? L"Main Device" : getNameW();
            m_device->SetName(name.data());
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

        return createUniqueQueue() && createSynchronisation();
    }

    bool Device::createUniqueQueue()
    {
        const D3D12_COMMAND_QUEUE_DESC desc{
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };

        const HRESULT hrGFXCmdQueueCreated = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_graphicsQueue));
        if (FAILED(hrGFXCmdQueueCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Graphics Queue failed!\nError Code: {}", hrGFXCmdQueueCreated),
                          RhiApi::DirectX12);
            return false;
        }
        else
        {
            const LPCWSTR name = L"GraphicsQueue";
            m_graphicsQueue->SetName(name);
            RHI_LOG_INFO(L"Create Graphics Queue success.", RhiApi::DirectX12);
        }
        return true;
    }

    bool Device::createSynchronisation()
    {
        m_synchronization.deviceFenceEvent = CreateEvent(nullptr, false, false, nullptr);
        if (!m_synchronization.deviceFenceEvent)
        {
            RHI_LOG_ERROR(L"Create Device Fence Event failed!", RhiApi::DirectX12);
            return false;
        }
        else
        {
            RHI_LOG_INFO(L"Create Device Fence Event success.", RhiApi::DirectX12);
        }

        const HRESULT hrDeviceFenceCreated =
            m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_synchronization.deviceFence));
        if (FAILED(hrDeviceFenceCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Device Fence failed! \nError Code: {}", hrDeviceFenceCreated),
                          RhiApi::DirectX12);
            return false;
        }
        else
        {
            const LPCWSTR name = L"DeviceFence";
            m_synchronization.deviceFence->SetName(name);

            RHI_LOG_INFO(L"Create Device Fence success.", RhiApi::DirectX12);
        }

        return true;
    }

    void Device::wait()
    {
        // Schedule a Signal command in the queue.
        m_graphicsQueue->Signal(m_synchronization.deviceFence.Get(), m_synchronization.deviceFenceValue);

        // Wait until the fence has been processed.
        m_synchronization.deviceFence->SetEventOnCompletion(m_synchronization.deviceFenceValue,
                                                            m_synchronization.deviceFenceEvent);
        WaitForSingleObjectEx(m_synchronization.deviceFenceEvent, INFINITE, false);

        // Increment for next use.
        ++m_synchronization.deviceFenceValue;
    }

    void Device::submit(CommandList& CommandList)
    {
        ID3D12CommandList* cmdListsArr[] = {CommandList.getCommandListNative()};
        m_graphicsQueue->ExecuteCommandLists(1, cmdListsArr);
        m_graphicsQueue->Signal(m_synchronization.deviceFence.Get(), m_synchronization.deviceFenceValue);
    }

}
