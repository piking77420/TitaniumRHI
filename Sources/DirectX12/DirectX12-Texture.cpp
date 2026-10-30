#include <DirectX12/DirectX12-Texture.hpp>

#include <DirectX12/DirectX12-RHI.hpp>

namespace TiRHI::DirectX12
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    Texture::~Texture()
    {
        if (m_device && m_rtvCpuHandle)
        {
            Device::Private::getDescriptorAllocatorRTV(*m_device).release(*m_rtvCpuHandle);
        }
    }

    bool Texture::build(Device& device)
    {
        m_device = &device;
        const std::wstring name = getNameW();
        m_image->SetName(name.c_str());
        RHI_LOG_WARNING(L"TODO", RhiApi::DirectX12);
        return false;
    }

    bool Texture::build(Device& device, MComPtr<ID3D12Resource> image)
    {
        m_device = &device;

        m_image = image;
        const std::wstring name = getNameW();
        m_image->SetName(name.data());

        m_rtvCpuHandle = Device::Private::getDescriptorAllocatorRTV(device).create(device);

        if (!m_rtvCpuHandle)
            return false;

        device.getNativeDevice()->CreateRenderTargetView(m_image.Get(), nullptr, m_rtvCpuHandle->nativeHandle);

        return m_image;
    }

    bool Texture::getRtv(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle)
    {
        if (m_rtvCpuHandle)
            handle = m_rtvCpuHandle->nativeHandle;

        return m_rtvCpuHandle.has_value();
    }

    bool Texture::getRtv(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle) const
    {
        if (m_rtvCpuHandle)
            handle = m_rtvCpuHandle->nativeHandle;

        return m_rtvCpuHandle.has_value();
    }

} // namespace TiRHI::DirectX12
