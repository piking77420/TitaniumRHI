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

    Texture::~Texture() = default;

    bool Texture::build(Device& device)
    {
        const std::wstring name = getNameW();
        m_image->SetName(name.c_str());

        D3D12_HEAP_PROPERTIES heap{};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = static_cast<UINT64>(getWidth());
        desc.Height = static_cast<UINT64>(getHeight());
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DirectX12::Private::toDirectX12(getFormat());
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = DirectX12::Private::textureFlagtoDirectX12(getFlags(), getFormat());

        HRESULT hr = device.getNativeDevice()->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_image));

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"Faile to create texture {}", getNameW()), RhiApi::DirectX12);
            return false;
        }

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
        {
            m_renderTargetView = Device::Private::getDescriptorAllocatorRTV(device).create(device);
            assert(m_renderTargetView);
            device.getNativeDevice()->CreateRenderTargetView(m_image.Get(), nullptr, m_renderTargetView->cpuHandle);
        }

        {
            m_shaderRenderView = Device::Private::getDescriptorAllocatorCbvSrvUav(device).create(device);
            assert(m_shaderRenderView);
            device.getNativeDevice()->CreateShaderResourceView(m_image.Get(), nullptr, m_shaderRenderView->cpuHandle);
        }

        return m_renderTargetView && m_shaderRenderView;
    }

} // namespace TiRHI::DirectX12
