#ifndef TITANIUM_DESCRIPTOR_SET_H
#define TITANIUM_DESCRIPTOR_SET_H

#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;
    class Texture;
    class Sampler;

    class DescriptorSet : public BaseDescriptorSet<DescriptorSet, RHI, Texture, Sampler>
    {
    public:
    private:
    };

} // TiRHI::DirectX12

#endif // TITANIUM_DESCRIPTOR_SET_H
