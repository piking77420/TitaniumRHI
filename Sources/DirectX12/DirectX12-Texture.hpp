#ifndef TITANIUM_DIRECTX12_TEXTURE_H
#define TITANIUM_DIRECTX12_TEXTURE_H

#include <Titanium/RHI-BaseTexture.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class Texture : public BaseTexture<Texture, RHI>
    {
    public:
        Texture() = delete;
        ~Texture() = default;
        RHI_MOVE_ONLY(Texture);
        Texture(RHI& rhi);

        struct Private
        {
            static bool build(Texture& texture, MComPtr<ID3D12Resource> image)
            {
                return texture.build(image);
            }

            static ID3D12Resource* getImage(Texture& texture)
            {
                return texture.m_image.Get();
            }
        };

        bool build(Texture& texture);

    private:
        friend Private;
        MComPtr<ID3D12Resource> m_image;

        bool build(MComPtr<ID3D12Resource> image);
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_TEXTURE_H
