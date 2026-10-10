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

    bool Texture::build(Device& device, vk::Image image)
    {
        m_image = image;

        return getImage() != VK_NULL_HANDLE;
    }

} // namespace TiRHI::Vulkan
