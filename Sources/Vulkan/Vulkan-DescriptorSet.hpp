#ifndef TITANIUM_VULKAN_DESCRIPTOR_SET_H
#define TITANIUM_VULKAN_DESCRIPTOR_SET_H

#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <Vulkan/Vulkan-Header.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class Texture;

    class DescriptorSet : public BaseDescriptorSet<DescriptorSet, RHI, Texture>
    {
    public:
        DescriptorSet() = delete;
        ~DescriptorSet() = default;
        RHI_MOVE_ONLY(DescriptorSet);
        DescriptorSet(RHI& rhi)
            : BaseDescriptorSet(rhi)
        {
        }

        bool build(Device& device);

    private:
        vk::UniqueDescriptorSet m_set;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DESCRIPTOR_SET_H
