#ifndef TITANIUM_VULKAN_TEXTURE_H
#define TITANIUM_VULKAN_TEXTURE_H

#include <Titanium/RHI-BaseTexture.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Titanium/Private/FrameTracked.hpp>
#include <vma/vk_mem_alloc.h>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Texture : public BaseTexture<Texture, Device>, public TiRHI::Private::FrameTracked<Texture>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;
        Texture() = delete;
        ~Texture();
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        Texture(Device& device);

        struct Private
        {
            static bool build(Texture& texture, vk::Image image, Format format)
            {
                return texture.build(image, format);
            }

            static vk::Image getImage(Texture& texture)
            {
                return texture.m_image;
            }

            static vk::ImageView getImageView(const Texture& texture)
            {
                return texture.m_uniqueView.get();
            }
        };

        bool build();

        bool isValid() const
        {
            return m_image != VK_NULL_HANDLE;
        }

        bool isInvalid() const
        {
            return m_image == VK_NULL_HANDLE;
        }

        operator bool() const
        {
            return isValid();
        }

    private:
        friend Private;

        vk::Image m_image = VK_NULL_HANDLE;

        vk::UniqueImageView m_uniqueView;

        VmaAllocation m_allocation = nullptr;

        bool build(vk::Image image, Format format);

        void destroy();
    };
}

#endif // TITANIUM_VULKAN_TEXTURE_H
