#ifndef TITANIUM_VULKAN_TEXTURE_H
#define TITANIUM_VULKAN_TEXTURE_H

#include <variant>
#include <Titanium/RHI-BaseTexture.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <vma/vk_mem_alloc.h>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Texture : public BaseTexture<Texture, RHI>
    {
    public:
        Texture() = delete;
        ~Texture();
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        Texture(RHI& rhi);

        struct Private
        {
            static bool build(Texture& texture, Device& device, vk::Image image, vk::Format format)
            {
                return texture.build(device, image, format);
            }

            static vk::Image getImage(Texture& texture)
            {
                return texture.m_image;
            }

            static vk::ImageView getImageView(Texture& texture)
            {
                return texture.m_uniqueView.get();
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        vk::Image m_image = VK_NULL_HANDLE;

        vk::UniqueImageView m_uniqueView;

        VmaAllocation m_allocation = nullptr;

        bool build(Device& device, vk::Image image, vk::Format format);

        void destroy();
    };
}

#endif // TITANIUM_VULKAN_TEXTURE_H
