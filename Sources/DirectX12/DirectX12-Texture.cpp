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
                m_lastSubmitedValue,
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

        if ((getFlags() & TextureFlags::RenderTarget) != TextureFlags::None)
        {
            m_renderTargetView = Device::Private::getDescriptorAllocatorRTV(getDevice()).create(getDevice());
            assert(m_renderTargetView);
            getDevice().getNativeDevice()->CreateRenderTargetView(m_image.Get(), nullptr,
                                                                  m_renderTargetView->cpuHandle);
            if (!m_renderTargetView)
                isOk = false;
        }

        if ((getFlags() & TextureFlags::ShaderResource) != TextureFlags::None)
        {
            m_shaderRenderView = Device::Private::getDescriptorAllocatorCbvSrvUav(getDevice()).create(getDevice());
            assert(m_shaderRenderView);
            getDevice().getNativeDevice()->CreateShaderResourceView(m_image.Get(), nullptr,
                                                                    m_shaderRenderView->cpuHandle);
            if (!m_shaderRenderView)
                isOk = false;
        }

        return isOk;
    }

} // namespace TiRHI::DirectX12
