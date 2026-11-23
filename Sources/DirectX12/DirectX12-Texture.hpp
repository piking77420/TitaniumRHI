#ifndef TITANIUM_DIRECTX12_TEXTURE_H
#define TITANIUM_DIRECTX12_TEXTURE_H

#include <D3D12MemAlloc.h>

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseTexture.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/Private/DescriptorAllocator.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class Texture : public BaseTexture<Texture, Device>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        Texture() = delete;
        ~Texture();
        RHI_MOVE_ONLY(Texture);
        Texture(Device& device);

        struct Private
        {
            static bool build(Texture& texture, MComPtr<ID3D12Resource> image)
            {
                return texture.build(image);
            }

            static ID3D12Resource* getImage(const Texture& texture)
            {
                return texture.m_image.Get();
            }

            static const std::optional<TiRHI::DirectX12::Private::DescriptorHandle>&
            getRenderTargetViewHandle(const Texture& texture)
            {
                return texture.m_renderTargetView;
            }

            static const std::optional<TiRHI::DirectX12::Private::DescriptorHandle>&
            getShaderRenderViewHandle(const Texture& texture)
            {
                return texture.m_shaderRenderView;
            }
        };

        bool build();

        bool isValid() const
        {
            return m_image;
        }

        bool isInvalid() const
        {
            return !isValid();
        }

        operator bool() const
        {
            return isValid();
        }

        void setSubmissionValue(uint64_t value)
        {
            m_lastSubmitedValue = value;
        }

    private:
        friend Private;

        MComPtr<ID3D12Resource> m_image;

        MComPtr<D3D12MA::Allocation> m_allocation;

        std::optional<TiRHI::DirectX12::Private::DescriptorHandle> m_renderTargetView;

        std::optional<TiRHI::DirectX12::Private::DescriptorHandle> m_shaderRenderView;

        uint64_t m_lastSubmitedValue = 0;

        bool build(MComPtr<ID3D12Resource> image);

        bool createHandle();
    };
} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_TEXTURE_H
