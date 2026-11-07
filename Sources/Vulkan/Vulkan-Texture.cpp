#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    Texture::~Texture()
    {
        destroy();
    }

    Texture::Texture(Texture&& other) noexcept
        : BaseTexture<Texture, RHI>(std::move(other))
        , m_device(std::exchange(other.m_device, nullptr))
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
        m_device = std::exchange(other.m_device, nullptr);
        m_image = std::exchange(other.m_image, vk::Image{});
        m_uniqueView = std::move(other.m_uniqueView);
        m_allocation = std::exchange(other.m_allocation, nullptr);

        return *this;
    }

    bool Texture::build(Device& device)
    {
        m_device = &device;

        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.setImageType(vk::ImageType::e2D)
            .setExtent(vk::Extent3D{m_width, m_height, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setFormat(Vulkan::Private::toVulkan(getFormat()))
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(Vulkan::Private::toVulkan(m_flags))
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined);

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        const VkImageCreateInfo nativeInfo = static_cast<VkImageCreateInfo>(imageCreateInfo);

        const vk::Result result =
            static_cast<vk::Result>(vmaCreateImage(Device::Private::getAllocator(device), &nativeInfo, &allocationInfo,
                                                   reinterpret_cast<VkImage*>(&m_image), &m_allocation, nullptr));

        if (result != vk::Result::eSuccess)
        {
            RHI_LOG_ERROR(std::format(L"Failed to create image : {} ", getNameW()), Api);
        }

        vk::ImageSubresourceRange imageSubresourceRange{};
        imageSubresourceRange.setAspectMask(Vulkan::Private::imageAspectFlagBitsFromFormat(m_format))
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.setImage(m_image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(imageCreateInfo.format)
            .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
            .setSubresourceRange(imageSubresourceRange);

        m_uniqueView = device.getNativeDevice().createImageViewUnique(viewInfo);

        return m_image != VK_NULL_HANDLE;
    }

    bool Texture::build(Device& device, vk::Image image, Format format)
    {
        m_image = image;
        m_device = &device;

        vk::ImageViewCreateInfo viewInfo{};

        viewInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(Vulkan::Private::toVulkan(format))
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
        if (m_allocation && m_device)
        {
            vmaDestroyImage(Device::Private::getAllocator(*m_device), static_cast<VkImage>(m_image), m_allocation);
        }

        m_image = vk::Image{};
        m_allocation = nullptr;
    }

} // namespace TiRHI::Vulkan
