#ifndef TITANIUM_DIRECTX12_DESCRIPTOR_SET_H
#define TITANIUM_DIRECTX12_DESCRIPTOR_SET_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class Texture;
    class Sampler;
    class Buffer;

    class DescriptorSet : public BaseDescriptorSet<DescriptorSet, RHI, Texture, Sampler, Buffer>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;
        using BaseType = typename BaseDescriptorSet<DescriptorSet, RHI, Texture, Sampler, Buffer>;
        using BindingsLayout = typename BaseType::BindingsLayout;

        DescriptorSet() = delete;
        ~DescriptorSet() = default;
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet& operator=(const DescriptorSet&) = delete;

        DescriptorSet(DescriptorSet&& other) noexcept;
        DescriptorSet& operator=(DescriptorSet&& other) noexcept;

        DescriptorSet(RHI& rhi)
            : BaseDescriptorSet(rhi)
        {
        }

        bool build(Device& device);

    private:
    };

} // TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_DESCRIPTOR_SET_H
