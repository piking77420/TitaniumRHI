#ifndef TITANIUM_DIRECTX12_TEXTURE_H
#define TITANIUM_DIRECTX12_TEXTURE_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseTexture.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/Private/DescriptorAllocator.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class Texture : public BaseTexture<Texture, RHI>
    {
    public:
        Texture() = delete;
        ~Texture();
        RHI_MOVE_ONLY(Texture);
        Texture(RHI& rhi);

        struct Private
        {
            static bool build(Texture& texture, Device& device, MComPtr<ID3D12Resource> image)
            {
                return texture.build(device, image);
            }

            static ID3D12Resource* getImage(const Texture& texture)
            {
                return texture.m_image.Get();
            }

            static std::optional<TiRHI::DirectX12::Private::DescriptorHandle> getDescriptorHandle(Texture& texture)
            {
                return texture.m_cpuHandle;
            }

            static std::optional<TiRHI::DirectX12::Private::DescriptorHandle>
            getDescriptorHandle(const Texture& texture)
            {
                return texture.m_cpuHandle;
            }
        };

        bool build(Device& device);

        bool isValid() const;

    private:
        friend Private;

        MComPtr<ID3D12Resource> m_image;

        std::optional<TiRHI::DirectX12::Private::DescriptorHandle> m_cpuHandle;

        bool build(Device& device, MComPtr<ID3D12Resource> image);
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_TEXTURE_H
