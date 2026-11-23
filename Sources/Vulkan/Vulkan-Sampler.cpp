#include <Vulkan/Vulkan-Sampler.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    Sampler::~Sampler()
    {
        if (!m_device)
            return;

        auto& defferdDestroy = Device::Private::getDeferredDeletionQueue(*m_device);
        defferdDestroy.retire(submissionValue,
                              [uniqueSampler = std::move(m_sampler)]() mutable { uniqueSampler.reset(); });
    }

    bool Sampler::build(Device& device)
    {
        vk::Device vkDevice = device.getNativeDevice();

        if (vkDevice == VK_NULL_HANDLE)
            return false;
        m_device = &device;
        vk::SamplerCreateInfo info{};
        info.flags = {};
        info.pNext = nullptr;

        namespace toVk = TiRHI::Vulkan::Private;

        // Filtering
        info.magFilter = toVk::toVulkan(getMagFilter());
        info.minFilter = toVk::toVulkan(getMinFilter());
        info.mipmapMode = toVk::toVulkanSamplerMipMapMode(getMipMapFilter());

        // Addressing
        info.addressModeU = toVk::toVulkan(getAddressModeU());
        info.addressModeV = toVk::toVulkan(getAddressModeV());
        info.addressModeW = toVk::toVulkan(getAddressModeW());

        // Mipmapping
        info.mipLodBias = getMipLodBias();
        info.minLod = getMinLod();
        info.maxLod = getMaxLod();

        // Anisotropic filtering
        info.anisotropyEnable = getAnisotropyEnabled();
        info.maxAnisotropy = getMaxAnisotropy();

        // Depth comparison
        info.compareEnable = getDepthCompareEnabled();
        info.compareOp = toVk::toVulkan(getDepthCompareOp());

        // Border
        info.borderColor = vk::BorderColor::eFloatOpaqueBlack;

        // Coordinates
        info.unnormalizedCoordinates = VK_FALSE;

        m_sampler = vkDevice.createSamplerUnique(info);

        return m_sampler.get();
    }

} // namespace TiRHI::Vulkan
