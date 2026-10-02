#include <DirectX12/DirectX12-SwapChain.hpp>

#include <format>
#include <Titanium/Log.hpp>

#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RHI.hpp>

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

    bool SwapChain::build(Device& device, WindowHandle windowHandle)
    {
        if (!createSwapChain(device, windowHandle))
            return false;

        if (!queryBuffer())
            return false;

        if (!initSynchronisation(device))
            return false;

        return true;
    }

    bool SwapChain::beginFrame()
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
        const HRESULT hrPresent = m_swapchain->Present(1, 0);

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

    bool SwapChain::createSwapChain(Device& device, WindowHandle windowHandle)
    {
        if (!getRHI().getNativeFactory())
        {
            RHI_LOG_ERROR(L"device.getFactory() was null when SwapChain::create", RhiApi::DirectX12);
            return false;
        }

        if (windowHandle == nullptr)
        {
            RHI_LOG_ERROR(L"windowHandle was null when SwapChain::create", RhiApi::DirectX12);
            return false;
        }

        const DXGI_SWAP_CHAIN_DESC1 desc{
            .Width = width(),
            .Height = height(),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM, // TODO ABSTRACT
            .Stereo = false,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = BufferCount,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = 0,
        };

        MComPtr<IDXGISwapChain1> swapchain1;
        const HRESULT hrSwapChainCreated = getRHI().getNativeFactory()->CreateSwapChainForHwnd(
            device.getGraphicQueue().Get(), reinterpret_cast<HWND>(windowHandle), &desc, nullptr, nullptr, &swapchain1);

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

        return true;
    }
    bool SwapChain::queryBuffer()
    {
        for (uint32_t i = 0; i < BufferCount; ++i)
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
} // namespace TiRHI::DirectX12
