#ifndef TITANIUM_VULKAN_DESCRIPTOR_SET_H
#define TITANIUM_VULKAN_DESCRIPTOR_SET_H

#include <Titanium/RHI-BaseDescriptorSet.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Private/DescriptorSetHandle.hpp>

namespace TiRHI::Vulkan::Private
{
    class DescriporManager;
}

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;
    class Texture;
    class Sampler;
    class Buffer;

    class DescriptorSet : public BaseDescriptorSet<DescriptorSet, Device, Texture, Sampler, Buffer>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;
        using BaseType = typename BaseDescriptorSet<DescriptorSet, Device, Texture, Sampler, Buffer>;
        using BindingsLayout = typename BaseType::BindingsLayout;

        DescriptorSet() = delete;
        ~DescriptorSet() = default;
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet& operator=(const DescriptorSet&) = delete;

        DescriptorSet(DescriptorSet&& other) noexcept;
        DescriptorSet& operator=(DescriptorSet&& other) noexcept;

        DescriptorSet(Device& device)
            : BaseDescriptorSet(device)
        {
        }

        struct Private
        {
            static const TiRHI::Vulkan::Private::DescriptorSetHandle&
            getDescriptorSetHandle(const DescriptorSet& descriptorSet)
            {
                return descriptorSet.m_handle;
            }

            static vk::DescriptorSet getDescriptorSetFrame(const DescriptorSet& descriptorSet)
            {
                return descriptorSet.getDescriptorSetFrame();
            }
        };

        bool build();

       std::uint64_t getLastFrameUsed(uint32_t frameIndex) const noexcept;

       DescriptorSet& setLastFrameUsed(uint64_t value, uint32_t frameIndex) noexcept;

    private:
        friend Private;

        TiRHI::Vulkan::Private::DescriptorSetHandle m_handle{};

        void reset();

        size_t getNbrOfDescriptorSet();

        vk::DescriptorSet getDescriptorSetFrame() const;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DESCRIPTOR_SET_H
