#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI::Vulkan
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    Texture::~Texture()
    {
    }

    Texture::Texture(Texture&& other) noexcept
        : BaseTexture<Texture, RHI>(std::move(other))
        , m_image(std::exchange(other.m_image, vk::Image{}))
        , m_uniqueView(std::move(other.m_uniqueView))
        , m_allocation(std::exchange(other.m_allocation, nullptr))
    {
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this == &other)
            return *this;

        destroy();
        BaseTexture<Texture, RHI>::operator=(std::move(other));
        m_image = std::exchange(other.m_image, vk::Image{});
        m_uniqueView = std::move(other.m_uniqueView);
        m_allocation = std::exchange(other.m_allocation, nullptr);

        return *this;
    }

    bool Texture::build(Device& device)
    {
        return m_image != VK_NULL_HANDLE;
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

        return m_image != VK_NULL_HANDLE;
    }

    void Texture::destroy()
    {
        // Destroy the view before the underlying image.
        m_uniqueView.reset();

        // Only VMA-allocated textures own their VkImage.
        // Swapchain images are owned by VkSwapchainKHR.
        // That allow swap chian to be used as an texture and particulary useful for command list tracking
        if (m_allocation)
        {
            // vmaDestroyImage(
            //, static_cast<VkImage>(m_image), m_allocation);
        }

        m_image = vk::Image{};
        m_allocation = nullptr;
    }

} // namespace TiRHI::Vulkan
