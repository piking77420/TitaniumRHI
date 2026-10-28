#ifndef TITANIUM_DIRECTX12_TEXTURE_H
#define TITANIUM_DIRECTX12_TEXTURE_H

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
        RHI_MOVE_CONSTRUCT_ONLY(Texture);
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

            static bool getRtv(Texture& texture, Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle)
            {
                return texture.getRtv(device, handle);
            }

            static bool getRtv(const Texture& texture, Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle)
            {
                return texture.getRtv(device, handle);
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        Device* m_device = nullptr;

        MComPtr<ID3D12Resource> m_image;

        TiRHI::DirectX12::CpuDescriptorHandle m_rtvCpuHandle;

        bool build(Device& device, MComPtr<ID3D12Resource> image);

        bool getRtv(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle);

        bool getRtv(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE& handle) const;
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_TEXTURE_H
