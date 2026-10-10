#include <DirectX12/DirectX12-Texture.hpp>

#include <DirectX12/DirectX12-RHI.hpp>

namespace TiRHI::DirectX12
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    bool Texture::build(Texture& texture)
    {
        const std::wstring name = getNameW();
        m_image->SetName(name.c_str());
        return false;
    }

    bool Texture::build(MComPtr<ID3D12Resource> image)
    {
        m_image = image;
        const std::wstring name = getNameW();
        m_image->SetName(name.data());

        return m_image;
    }
} // namespace TiRHI::DirectX12
