#include <DirectX12/DirectX12-Texture.hpp>

#include <Titanium/Log.hpp>

#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    Texture::~Texture()
    {
        m_image.Reset();
        m_allocation.Reset();
    };

    bool Texture::build(Device& device)
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

        D3D12MA::Allocator& allocator = Device::Private::getAllocator(device);

        const HRESULT hr = allocator.CreateResource(&allocDesc, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                    m_allocation.ReleaseAndGetAddressOf(),
                                                    IID_PPV_ARGS(m_image.ReleaseAndGetAddressOf()));

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"Faile to create texture {}", getNameW()), RhiApi::DirectX12);
            return false;
        }

        const std::wstring name = getNameW();
        m_image->SetName(name.c_str());

        return m_image && createHandle(device);
    }

    bool Texture::build(Device& device, MComPtr<ID3D12Resource> image)
    {
        m_image = image;
        const std::wstring name = getNameW();
        m_image->SetName(name.data());

        return m_image && createHandle(device);
    }

    bool Texture::isValid() const
    {
        return m_image && m_renderTargetView && m_shaderRenderView;
    }

    bool Texture::createHandle(Device& device)
    {
        bool isOk = true;

        if ((getFlags() & TextureFlags::RenderTarget) != TextureFlags::None)
        {
            m_renderTargetView = Device::Private::getDescriptorAllocatorRTV(device).create(device);
            assert(m_renderTargetView);
            device.getNativeDevice()->CreateRenderTargetView(m_image.Get(), nullptr, m_renderTargetView->cpuHandle);
            if (!m_renderTargetView)
                isOk = false;
        }

        if ((getFlags() & TextureFlags::ShaderResource) != TextureFlags::None)
        {
            m_shaderRenderView = Device::Private::getDescriptorAllocatorCbvSrvUav(device).create(device);
            assert(m_shaderRenderView);
            device.getNativeDevice()->CreateShaderResourceView(m_image.Get(), nullptr, m_shaderRenderView->cpuHandle);
            if (!m_shaderRenderView)
                isOk = false;
        }

        return isOk;
    }

} // namespace TiRHI::DirectX12
