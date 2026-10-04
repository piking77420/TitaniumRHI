#include <DirectX12/DirectX12-SwapChain.hpp>

#include <format>
#include <Titanium/Log.hpp>

#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Surface.hpp>

namespace TiRHI::DirectX12
{
    SwapChain::SwapChain(RHI& rhi)
        : BaseSwapChain(rhi)
    {
    }

    SwapChain::~SwapChain()
    {
        if (m_synchronisation.swapchainFenceEvent)
        {
            CloseHandle(m_synchronisation.swapchainFenceEvent);
        }
    }

    bool SwapChain::build(Device& device, Surface& surface)
    {
        return createSwapChain(device, surface);
    }

    bool SwapChain::beginFrame([[maybe_unused]] Device& device)
    {
        const UINT32 prevFenceValue = swapchainFenceValues[m_swapchainFrameIndex];

        // Update frame index.
        m_swapchainFrameIndex = m_swapchain->GetCurrentBackBufferIndex();

        const UINT32 currFenceValue = swapchainFenceValues[m_swapchainFrameIndex];

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_synchronisation.swapchainFence->GetCompletedValue() < currFenceValue)
        {
            const HRESULT hrSetEvent = m_synchronisation.swapchainFence->SetEventOnCompletion(
                currFenceValue, m_synchronisation.swapchainFenceEvent);
            if (FAILED(hrSetEvent))
            {
                RHI_LOG_ERROR(std::format(L"Fence SetEventOnCompletion failed.\nError Code: {}", hrSetEvent),
                              RhiApi::DirectX12);
                return false;
            }

            WaitForSingleObjectEx(m_synchronisation.swapchainFenceEvent, INFINITE, FALSE);
        }

        // Set the fence value for the next frame.
        swapchainFenceValues[m_swapchainFrameIndex] = prevFenceValue + 1;

        return true;
    }

    bool SwapChain::present(Device& device)
    {
        // Automatically present using internal present queue if possible.
        const HRESULT hrPresent = m_swapchain->Present(m_vsync ? 1 : 0, 0);

        if (FAILED(hrPresent))
        {
            RHI_LOG_ERROR(std::format(L"SwapChain Present failed!\nError Code: {}", hrPresent), RhiApi::DirectX12);

            return false;
        }

        // Schedule a Signal command in the queue.
        const UINT64 currFenceValue = swapchainFenceValues[m_swapchainFrameIndex];

        const HRESULT hrFenceSignal =
            device.getGraphicQueue()->Signal(m_synchronisation.swapchainFence.Get(), currFenceValue);

        if (FAILED(hrFenceSignal))
        {
            RHI_LOG_ERROR(std::format(L"SwapChain Fence Signal failed!\nError Code: {}", hrFenceSignal),
                          RhiApi::DirectX12);

            return false;
        }

        return true;
    }

    bool SwapChain::recreateSwapChain(Device& device, Surface& surface)
    {
        m_images.clear();

        const HRESULT hr =
            m_swapchain->ResizeBuffers(getImageCount(), getWidth(), getHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, 0);

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"ResizeBuffers failed: {}", hr), RhiApi::DirectX12);

            return false;
        }

        m_swapchainFrameIndex = m_swapchain->GetCurrentBackBufferIndex();

        return queryBuffer() && createRenderTarget(device);
    }

    ID3D12Resource* SwapChain::getNativeCurrentBackBuffer() const
    {
        return m_images[m_swapchainFrameIndex].Get();
    }

    bool SwapChain::createSwapChain(Device& device, Surface& surface)
    {
        if (!getRHI().getNativeFactory())
        {
            RHI_LOG_ERROR(L"device.getFactory() was null when SwapChain::create", RhiApi::DirectX12);
            return false;
        }

        if (surface.getWindowHandle() == nullptr)
        {
            RHI_LOG_ERROR(L"windowHandle was null when SwapChain::create", RhiApi::DirectX12);
            return false;
        }

        const DXGI_SWAP_CHAIN_DESC1 desc{
            .Width = getWidth(),
            .Height = getHeight(),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM, // TODO ABSTRACT
            .Stereo = false,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = getImageCount(),
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = 0,
        };

        MComPtr<IDXGISwapChain1> swapchain1;
        const HRESULT hrSwapChainCreated = getRHI().getNativeFactory()->CreateSwapChainForHwnd(
            device.getGraphicQueue().Get(), reinterpret_cast<HWND>(surface.getWindowHandle()), &desc, nullptr, nullptr,
            &swapchain1);

        std::string_view name = getName();
        if (!name.empty())
            swapchain1->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.data());

        if (FAILED(hrSwapChainCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create SwapChain failed!\nError Code: {}", hrSwapChainCreated),
                          RhiApi::DirectX12);

            return false;
        }
        else
        {
            RHI_LOG_INFO(std::format(L"Create SwapChain success\nHandle: {}, Name: {}.", hrSwapChainCreated,
                                     std::wstring(name.begin(), name.end())),
                         RhiApi::DirectX12);
        }

        const HRESULT hrSwapChainCast = swapchain1.As(&m_swapchain);
        if (FAILED(hrSwapChainCast))
        {
            RHI_LOG_ERROR(std::format(L"SwapChain cast failed! \n Error Code: {}", hrSwapChainCast), RhiApi::DirectX12);
        }

        return queryBuffer() && initSynchronisation(device) && createRenderTarget(device);
    }

    bool SwapChain::queryBuffer()
    {
        m_images.resize(getImageCount());
        for (uint32_t i = 0; i < m_images.size(); ++i)
        {
            const HRESULT hrSwapChainGetBuffer = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_images[i]));
            if (FAILED(hrSwapChainGetBuffer))
            {
                RHI_LOG_ERROR(
                    std::format(L"Get SwapChain Buffer {} failed! \n Error Code: {}", i, hrSwapChainGetBuffer),
                    RhiApi::DirectX12);
                return false;
            }
            else
            {
                const std::wstring name = L"SwapchainBackBuffer [" + std::to_wstring(i) + L"]";
                m_images[i]->SetName(name.data());

                RHI_LOG_INFO(std::format(L"Get SwapChain Buffer [%1] success.\n handle: {}, name: {}", i, name,
                                         static_cast<void*>(m_images[i].Get())),
                             RhiApi::DirectX12);
            }
        }

        return true;
    }

    bool SwapChain::initSynchronisation(Device& device)
    {
        swapchainFenceValues.resize(m_images.size());
        m_synchronisation.swapchainFenceEvent = CreateEvent(nullptr, false, false, nullptr);

        if (!m_synchronisation.swapchainFenceEvent)
        {
            RHI_LOG_ERROR(L"Create SwapChain Fence Event failed!", RhiApi::DirectX12);

            return false;
        }

        const HRESULT hrSwapChainFenceCreated = device.getNativeDevice()->CreateFence(
            0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_synchronisation.swapchainFence));

        if (FAILED(hrSwapChainFenceCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create SwapChain Fence failed!\nError Code: {}", hrSwapChainFenceCreated),
                          RhiApi::DirectX12);

            return false;
        }

        constexpr std::wstring_view name = L"SwapchainFence";

        m_synchronisation.swapchainFence->SetName(name.data());

        RHI_LOG_INFO(std::format(L"Create SwapChain Fence success.\nHandle: {}, Name: {}",
                                 static_cast<void*>(m_synchronisation.swapchainFence.Get()), name),
                     RhiApi::DirectX12);

        return true;
    }

    bool SwapChain::createRenderTarget(Device& device)
    {
        m_rtvHeap.Reset();
        ID3D12Device* d3d12Device = device.getNativeDevice();

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.NumDescriptors = static_cast<UINT>(m_images.size());
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        const HRESULT result = d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));

        if (FAILED(result))
        {
            RHI_LOG_ERROR(L"Failed to create swapChain descriptor heap", RhiApi::DirectX12);
            return false;
        }

        m_rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < heapDesc.NumDescriptors; ++i)
        {
            if (FAILED(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_images[i]))))
            {
                RHI_LOG_ERROR(L"Failed to get buffer of swapChain image", RhiApi::DirectX12);

                return false;
            }

            d3d12Device->CreateRenderTargetView(m_images[i].Get(), nullptr, rtvHandle);

            rtvHandle.ptr += m_rtvDescriptorSize;
        }

        return true;
    }

} // namespace TiRHI::DirectX12
