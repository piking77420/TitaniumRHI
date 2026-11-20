#include <Vulkan/Vulkan-DescriptorSet.hpp>
#include <Vulkan/Private/DescriptorManager.hpp>
#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Sampler.hpp>
#include <Vulkan/Vulkan-Device.hpp>
#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI::Vulkan
{
    DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
        : BaseDescriptorSet(std::move(other))
        , m_descriptorManager(std::exchange(other.m_descriptorManager, nullptr))
        , m_handle(std::exchange(other.m_handle, {}))
    {
    }
    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
    {
        if (this == &other)
            return *this;

        BaseDescriptorSet::operator=(std::move(other));
        reset();
        m_descriptorManager = std::exchange(other.m_descriptorManager, nullptr);
        m_handle = std::exchange(other.m_handle, {});

        return *this;
    }

    bool DescriptorSet::build(Device& device)
    {
        m_descriptorManager = &Device::Private::getDescriptorManager(device);
        if (!m_descriptorManager)
            return false;

        const std::optional<TiRHI::Vulkan::Private::DescriptorSetHandle> handle =
            m_descriptorManager->allocAndWriteDescriptors(device.getNativeDevice(), getBindingsLayout(),
                                                          getNbrOfDescriptorSet());
        if (!handle)
            return false;

        m_handle = *handle;
        return true;
    }

    void DescriptorSet::reset()
    {
        // TODO defreed destroy
        if (m_descriptorManager && m_handle.descriptorCount > 0)
        {
            m_descriptorManager->release(getBindingsLayout(), m_handle);
        }

        m_descriptorManager = nullptr;
    }

    size_t DescriptorSet::getNbrOfDescriptorSet()
    {
        switch (getDescriptorSetType())
        {
        case DescriptorSetType::Dynamic:
            return getRHI().getFrameInFlight();
        case DescriptorSetType::Immutable:
            return 1;
        default:
            RHI_LOG_ERROR(std::format(L"Unknow DescriptorSetType {}", getNameW()), Api);
            break;
        }

        return 0;
    }

    vk::DescriptorSet DescriptorSet::getDescriptorSetFrame() const
    {
        if (m_handle.descriptorCount == 0)
            return {};

        switch (getDescriptorSetType())
        {
        case DescriptorSetType::Dynamic:
            return m_handle.descriptorSets[getRHI().getCurrentFrame()];
        case DescriptorSetType::Immutable:
            return m_handle.descriptorSets[0];
        default:
            RHI_LOG_ERROR(std::format(L"Unknow DescriptorSetType {}", getNameW()), Api);
            break;
        }

        return {};
    }

} // namespace TiRHI::Vulkan
