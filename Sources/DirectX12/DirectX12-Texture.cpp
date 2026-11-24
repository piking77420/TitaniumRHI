#include <DirectX12/DirectX12-Texture.hpp>

#include <Titanium/Log.hpp>

#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12
{
    Texture::Texture(Device& device)
        : BaseTexture(device)
    {
    }

    Texture::~Texture()
    {
        // there an allocation if this an not an swap chain image
        if (m_allocation)
        {
            auto& deferredDestroy = Device::Private::getDeferredDeletionQueue(getDevice());
            deferredDestroy.retire(
                getLastFrameUsed(),
                [device = &getDevice(), image = std::move(m_image), alloc = std::move(m_allocation),
                 renderTargetView = std::exchange(m_renderTargetView, std::nullopt),
                 shaderRenderView = std::exchange(m_shaderRenderView, std::nullopt)]() mutable
                {
                    if (renderTargetView)
                    {
                        Device::Private::getDescriptorAllocatorRTV(*device).release(*renderTargetView);
                    }

                    if (shaderRenderView)
                    {
                        Device::Private::getDescriptorAllocatorCbvSrvUav(*device).release(*shaderRenderView);
                    }

                    if (alloc)
                        alloc.Reset();

                    if (image)
                        image.Reset();
                });
        }
        else // Swapchain images must be released immediately before resizing the swapchain.
             // See SwapChain::recreate() for synchronization details.
        {
            if (m_renderTargetView)
            {
                Device::Private::getDescriptorAllocatorRTV(getDevice()).release(*m_renderTargetView);
                m_renderTargetView.reset();
            }

            if (m_shaderRenderView)
            {
                Device::Private::getDescriptorAllocatorCbvSrvUav(getDevice()).release(*m_shaderRenderView);
                m_shaderRenderView.reset();
            }

            m_allocation.Reset();
            if (m_image)
                m_image.Reset();
        }
    };

    bool Texture::build()
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = static_cast<UINT64>(getWidth());
        desc.Height = static_cast<UINT64>(getHeight());
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DirectX12::Private::toDirectX12(getFormat());
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = DirectX12::Private::textureFlagtoDirectX12(getFlags(), getFormat());

        D3D12MA::Allocator& allocator = Device::Private::getAllocator(getDevice());

        const HRESULT hr = allocator.CreateResource(&allocDesc, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                    m_allocation.ReleaseAndGetAddressOf(),
                                                    IID_PPV_ARGS(m_image.ReleaseAndGetAddressOf()));

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"Faile to create texture {}", getNameW()), RhiApi::DirectX12);
            return false;
        }

        const std::wstring name = getNameW();
        m_allocation->GetResource()->SetName(name.c_str());

        return m_image && createHandle();
    }

    bool Texture::build(MComPtr<ID3D12Resource> image)
    {
        m_image = image;
        const std::wstring name = getNameW();
        m_image->SetName(name.data());

        return m_image && createHandle();
    }

    bool Texture::createHandle()
    {
        bool isOk = true;
        auto& device = getDevice();
        MComPtr<ID3D12Device> nativeDevice = device.getNativeDevice();

        const bool isDepth = isDepthFormat(getFormat());

        // RTV / DSV
        if ((getFlags() & TextureFlags::RenderTarget) != TextureFlags::None)
        {
            if (isDepth)
            {
                m_renderTargetView = Device::Private::getDescriptorAllocatorDsv(device).create(device);

                if (m_renderTargetView)
                {
                    D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
                    desc.Format = DirectX12::Private::toDirectX12(getFormat());
                    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    desc.Flags = D3D12_DSV_FLAG_NONE;
                    desc.Texture2D.MipSlice = 0;

                    nativeDevice->CreateDepthStencilView(m_image.Get(), &desc, m_renderTargetView->cpuHandle);
                }
                else
                {
                    isOk = false;
                }
            }
            else
            {
                m_renderTargetView = Device::Private::getDescriptorAllocatorRTV(device).create(device);

                if (m_renderTargetView)
                {
                    D3D12_RENDER_TARGET_VIEW_DESC desc{};
                    desc.Format = DirectX12::Private::toDirectX12(getFormat());
                    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = 0;
                    desc.Texture2D.PlaneSlice = 0;

                    nativeDevice->CreateRenderTargetView(m_image.Get(), &desc, m_renderTargetView->cpuHandle);
                }
                else
                {
                    isOk = false;
                }
            }
        }

        // SRV
        if ((getFlags() & TextureFlags::ShaderResource) != TextureFlags::None)
        {
            m_shaderRenderView = Device::Private::getDescriptorAllocatorCbvSrvUav(device).create(device);

            if (m_shaderRenderView)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC desc{};

                desc.Format = isDepth ? DirectX12::Private::toDirectX12SRVFormat(getFormat())
                                      : DirectX12::Private::toDirectX12(getFormat());

                desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                desc.Texture2D.MostDetailedMip = 0;
                desc.Texture2D.MipLevels = m_image->GetDesc().MipLevels;
                desc.Texture2D.PlaneSlice = 0;
                desc.Texture2D.ResourceMinLODClamp = 0.0f;

                nativeDevice->CreateShaderResourceView(m_image.Get(), &desc, m_shaderRenderView->cpuHandle);
            }
            else
            {
                isOk = false;
            }
        }

        return isOk;
    }

} // namespace TiRHI::DirectX12
