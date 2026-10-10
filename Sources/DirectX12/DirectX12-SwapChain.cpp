#include <DirectX12/DirectX12-SwapChain.hpp>

#include <format>
#include <Titanium/Log.hpp>

#include <DirectX12/DirectX12-Device.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Surface.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>
#include <DirectX12/DirectX12-RenderTargets.hpp>

namespace TiRHI::DirectX12
{
    SwapChain::SwapChain(RHI& rhi)
        : BaseSwapChain(rhi)
    {
    }

    SwapChain::~SwapChain()
    {
    }

    bool SwapChain::build(Device& device, Surface& surface)
    {
        return createSwapChain(device, surface);
    }

    AcquiredFrame SwapChain::acquireNextImage()
    {

        // Update frame index.
        m_swapchainFrameIndex = m_swapchain->GetCurrentBackBufferIndex();

        AcquiredFrame acquire(true);

        acquire.setSwapChainImageIndex(m_swapchainFrameIndex);

        return acquire;
    }

    bool SwapChain::present()
    {
        // Automatically present using internal present queue if possible.
        const HRESULT hrPresent = m_swapchain->Present(m_vsync ? 1 : 0, 0);

        if (FAILED(hrPresent))
        {
            RHI_LOG_ERROR(std::format(L"SwapChain Present failed!\nError Code: {}", hrPresent), RhiApi::DirectX12);
            return false;
        }

        return true;
    }

    bool SwapChain::recreateSwapChain(Device& device, Surface& surface)
    {
        m_textures.clear();

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
            device.getNativeGraphicQueue().Get(), reinterpret_cast<HWND>(surface.getWindowHandle()), &desc, nullptr,
            nullptr, &swapchain1);

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

        return queryBuffer() && createRenderPassDescriptor(device) && createRenderTarget(device);
    }

    bool SwapChain::queryBuffer()
    {
        m_textures.clear();
        m_textures.reserve(getImageCount());
        for (uint32_t i = 0; i < getImageCount(); ++i)
        {
            MComPtr<ID3D12Resource> image;
            const HRESULT hrSwapChainGetBuffer = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&image));
            if (FAILED(hrSwapChainGetBuffer))
            {
                RHI_LOG_ERROR(
                    std::format(L"Get SwapChain Buffer {} failed! \n Error Code: {}", i, hrSwapChainGetBuffer),
                    RhiApi::DirectX12);
                return false;
            }
            else
            {
                auto& texture = m_textures.emplace_back(getRHI());
                texture.setName("SwapchainBackBuffer [" + std::to_string(i) + "]");
                Texture::Private::build(texture, image);

                RHI_LOG_INFO(std::format(L"Get SwapChain Buffer [%1] success.\n handle: {}, name: {}", i,
                                         texture.getNameW(), static_cast<void*>(Texture::Private::getImage(texture))),
                             RhiApi::DirectX12);
            }
        }

        return true;
    }

    bool SwapChain::createRenderTarget(Device& device)
    {
        m_rtvHeap.Reset();
        ID3D12Device* d3d12Device = device.getNativeDevice();

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.NumDescriptors = static_cast<UINT>(m_textures.size());
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        const HRESULT result = d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));

        if (FAILED(result))
        {
            RHI_LOG_ERROR(L"Failed to create swapChain descriptor heap", RhiApi::DirectX12);
            return false;
        }

        m_rtvDescriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

        m_renderTargets.clear();
        m_renderTargets.reserve(heapDesc.NumDescriptors);
        for (UINT i = 0; i < heapDesc.NumDescriptors; ++i)
        {
            auto& currentRenderTarget = m_renderTargets.emplace_back(getRHI());
            currentRenderTarget.setWidth(getWidth())
                .setHeight(getHeight())
                .setName(std::format("SwapChain Rendertarget {}", i))
                .setRenderPassDescriptor(&m_renderPassDescriptor);

            if (!RenderTargets::Private::build(currentRenderTarget, device, rtvHandle,
                                               Texture::Private::getImage(m_textures[i])))
            {
                RHI_LOG_ERROR(L"Failed to build render targets of swapChain image", RhiApi::DirectX12);
            }

            rtvHandle.ptr += m_rtvDescriptorSize;
        }

        return true;
    }

    const RenderTargets& SwapChain::getCurrentRenderTargets() const
    {
        return m_renderTargets[m_swapchainFrameIndex];
    }

    RenderTargets& SwapChain::getCurrentRenderTargets()
    {
        return m_renderTargets[m_swapchainFrameIndex];
    }

    const Texture& SwapChain::getCurrentSwapChainTexture() const
    {
        return m_textures[m_swapchainFrameIndex];
    }

    Texture& SwapChain::getCurrentSwapChainTexture()
    {
        return m_textures[m_swapchainFrameIndex];
    }

} // namespace TiRHI::DirectX12
