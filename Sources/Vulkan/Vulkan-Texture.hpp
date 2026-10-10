#ifndef TITANIUM_VULKAN_TEXTURE_H
#define TITANIUM_VULKAN_TEXTURE_H

#include <variant>
#include <Vulkan/Vulkan-Header.hpp>
#include <Titanium/RHI-BaseTexture.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Texture : public BaseTexture<Texture, RHI>
    {
    public:
        Texture() = delete;
        ~Texture() = default;
        RHI_MOVE_ONLY(Texture)
        Texture(RHI& rhi);

        struct Private
        {
            static bool build(Texture& texture, Device& device, vk::Image image)
            {
                return texture.build(device, image);
            }

            static vk::Image getImage(Texture& texture)
            {
                return texture.getImage();
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        //
        std::variant<std::monostate, vk::UniqueImage, vk::Image> m_image;

        vk::Image getImage() const
        {
            return std::visit(overloads{[](auto&& v) -> vk::Image { return VK_NULL_HANDLE; },
                                        [](const vk::UniqueImage& image) -> vk::Image { return image.get(); },
                                        [](vk::Image image) -> vk::Image { return image; }},
                              m_image);
        };

        bool build(Device& device, vk::Image image);
    };
}

#endif // TITANIUM_VULKAN_TEXTURE_H
