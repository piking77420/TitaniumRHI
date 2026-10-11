#include <DirectX12/DirectX12-Texture.hpp>

#include <DirectX12/DirectX12-RHI.hpp>

namespace TiRHI::DirectX12
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    bool Texture::build(Device& device)
    {
        const std::wstring name = getNameW();
        m_image->SetName(name.c_str());
        RHI_LOG_WARNING(L"TODO", RhiApi::DirectX12);
        return false;
    }

    bool Texture::build(Device& device, MComPtr<ID3D12Resource> image)
    {
        m_image = image;
        const std::wstring name = getNameW();
        m_image->SetName(name.data());

        D3D12_CPU_DESCRIPTOR_HANDLE h;
        if (!getRtv(device, h))
            return false;

        return m_image;
    }

    bool Texture::getRtv(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle)
    {
        std::optional<D3D12_CPU_DESCRIPTOR_HANDLE> h =
            device.getDescriptorAllocator().getHandle(device, m_rtvCpuHandle);
        if (h)
            handle = *h;

        return h.has_value();
    }

} // namespace TiRHI::DirectX12
