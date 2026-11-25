#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>
#include <Vulkan/Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    Texture::Texture(Device& device)
        : BaseTexture(device)
    {
    }

    Texture::~Texture()
    {
        destroy();
    }

    Texture::Texture(Texture&& other) noexcept
        : BaseTexture(std::move(other))
        , TiRHI::Private::FrameTracked<Texture>(std::move(other))
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
        BaseTexture::operator=(std::move(other));
        TiRHI::Private::FrameTracked<Texture>::operator=(std::move(other));
        m_image = std::exchange(other.m_image, vk::Image{});
        m_uniqueView = std::move(other.m_uniqueView);
        m_allocation = std::exchange(other.m_allocation, nullptr);

        return *this;
    }

    bool Texture::build()
    {
        const vk::ImageAspectFlags aspectFlag = Vulkan::Private::imageAspectFlagBitsFromFormat(m_format);

        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.setImageType(vk::ImageType::e2D)
            .setExtent(vk::Extent3D{m_width, m_height, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setFormat(Vulkan::Private::toVulkan(getFormat()))
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(Vulkan::Private::toVulkan(m_flags, aspectFlag))
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined);

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        const VkImageCreateInfo nativeInfo = static_cast<VkImageCreateInfo>(imageCreateInfo);

        const vk::Result result =
            static_cast<vk::Result>(vmaCreateImage(Device::Private::getAllocator(getDevice()), &nativeInfo, &allocationInfo,
                                                   reinterpret_cast<VkImage*>(&m_image), &m_allocation, nullptr));

        if (result != vk::Result::eSuccess)
        {
            RHI_LOG_ERROR(std::format(L"Failed to create image : {} ", getNameW()), Api);
        }

        if (m_image)
        {
            vk::DebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.objectType = vk::ObjectType::eImage;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkImage>(m_image));
            nameInfo.pObjectName = getName().data();

            getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);
        }

        vk::ImageSubresourceRange imageSubresourceRange{};
        imageSubresourceRange.setAspectMask(aspectFlag)
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

        m_uniqueView = getDevice().getNativeDevice().createImageViewUnique(viewInfo);

        if (m_uniqueView)
        {
            const std::string uniqueViewName = std::string(getName()) + " view";
            vk::DebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.objectType = vk::ObjectType::eImageView;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkImageView>(m_uniqueView.get()));
            nameInfo.pObjectName = uniqueViewName.c_str();

            getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);
        }

        return m_image != VK_NULL_HANDLE;
    }

    bool Texture::build(vk::Image image, Format format)
    {
        m_image = image;

        vk::ImageViewCreateInfo viewInfo{};

        viewInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(Vulkan::Private::toVulkan(format))
            .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                            vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        m_uniqueView = getDevice().getNativeDevice().createImageViewUnique(viewInfo);

        return m_image != VK_NULL_HANDLE;
    }

    void Texture::destroy()
    {

        // Only VMA-allocated textures own their VkImage.
        // Swapchain images are owned by VkSwapchainKHR.
        // That allow swap chian to be used as an texture and particulary useful for command list tracking
        if (m_allocation)
        {
            TiRHI::Private::DeferredDeletionQueue& deferredDeletionQueue =
                Device::Private::getDeferredDeletionQueue(getDevice());

            deferredDeletionQueue.retire(
                getLastFrameUsed(), [allocator = Device::Private::getAllocator(getDevice()), image = std::move(m_image),
                                    allocation = std::move(m_allocation), view = std::move(m_uniqueView)]()
                { vmaDestroyImage(allocator, static_cast<VkImage>(image), allocation); });
        }
        else
        {
            // if swapchain
            TiRHI::Private::DeferredDeletionQueue& deferredDeletionQueue =
                Device::Private::getDeferredDeletionQueue(getDevice());

            deferredDeletionQueue.retire(getLastFrameUsed(), [view = std::move(m_uniqueView)]() {});
        }

        m_image = vk::Image{};
        m_allocation = nullptr;
        setLastFrameUsed(0);
    }

} // namespace TiRHI::Vulkan
