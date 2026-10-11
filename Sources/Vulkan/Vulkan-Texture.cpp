#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI::Vulkan
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    bool Texture::build(Device& device)
    {
        return getImage() != VK_NULL_HANDLE;
    }

    bool Texture::build(Device& device, vk::Image image, vk::Format format)
    {
        m_image = image;

        vk::ImageViewCreateInfo viewInfo{};

        viewInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format)
            .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        m_uniqueView = device.getNativeDevice().createImageViewUnique(viewInfo);

        return getImage() != VK_NULL_HANDLE;
    }

} // namespace TiRHI::Vulkan
