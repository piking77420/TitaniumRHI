#include <Vulkan/Vulkan-Sampler.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Private/RHIToVulkan.hpp>

namespace TiRHI::Vulkan
{
    Sampler::~Sampler()
    {

        auto& defferdDestroy = Device::Private::getDeferredDeletionQueue(getDevice());
        defferdDestroy.retire(getLastFrameUsed(),
                              [uniqueSampler = std::move(m_sampler)]() mutable { uniqueSampler.reset(); });
    }

    bool Sampler::build()
    {
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

        m_sampler = getDevice().getNativeDevice().createSamplerUnique(info);

        if (m_sampler)
        {
            vk::DebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.objectType = vk::ObjectType::eSampler;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkSampler>(m_sampler.get()));
            nameInfo.pObjectName = getName().data();

            getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);
        }

        return m_sampler.get();
    }

} // namespace TiRHI::Vulkan
