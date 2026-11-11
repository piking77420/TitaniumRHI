#include <DirectX12/DirectX12-Texture.hpp>

#include <Titanium/Log.hpp>

#include <DirectX12/DirectX12-RHI.hpp>

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

        m_cpuHandle = Device::Private::getDescriptorAllocatorRTV(device).create(device);

        RHI_LOG_FATAL(L"TODO", RhiApi::DirectX12);

        device.getNativeDevice()->CreateRenderTargetView(m_image.Get(), nullptr, m_cpuHandle->nativeHandle);
        return false;
    }

    bool Texture::build(Device& device, MComPtr<ID3D12Resource> image)
    {
        m_image = image;
        const std::wstring name = getNameW();
        m_image->SetName(name.data());

        m_cpuHandle = Device::Private::getDescriptorAllocatorRTV(device).create(device);

        if (!m_cpuHandle)
            return false;

        device.getNativeDevice()->CreateRenderTargetView(m_image.Get(), nullptr, m_cpuHandle->nativeHandle);

        return m_image;
    }

    bool Texture::isValid() const
    {
        return m_image && m_cpuHandle;
    }

} // namespace TiRHI::DirectX12
