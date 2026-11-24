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

    bool DescriptorSet::build()
    {
        m_descriptorManager = &Device::Private::getDescriptorManager(getDevice());
        if (!m_descriptorManager)
            return false;

        const std::optional<TiRHI::Vulkan::Private::DescriptorSetHandle> handle =
            m_descriptorManager->allocAndWriteDescriptors(getDevice().getNativeDevice(), getBindingsLayout(),
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
            return m_handle.descriptorSets[getRHI().getCurrentFrame()].descriptorSet;
        case DescriptorSetType::Immutable:
            return m_handle.descriptorSets[0].descriptorSet;
        default:
            RHI_LOG_ERROR(std::format(L"Unknow DescriptorSetType {}", getNameW()), Api);
            break;
        }

        return {};
    }

    std::uint64_t DescriptorSet::getLastFrameUsed(uint32_t frameIndex) const noexcept
    {
        assert(frameIndex < m_handle.descriptorCount);
        if (frameIndex >= m_handle.descriptorCount) [[unlikely]]
        {
            RHI_LOG_FATAL(std::format(L"Invalid access to mark the descriptor set {}", getNameW()), Api);
        }

        return m_handle.descriptorSets[frameIndex].getLastFrameUsed();
    }

    DescriptorSet& DescriptorSet::setLastFrameUsed(uint64_t value, uint32_t frameIndex) noexcept
    {
        assert(frameIndex < m_handle.descriptorCount);
        if (frameIndex >= m_handle.descriptorCount) [[unlikely]]
        {
            RHI_LOG_FATAL(std::format(L"Invalid access to mark the descriptor set {}", getNameW()), Api);
        }

        m_handle.descriptorSets[frameIndex].setLastFrameUsed(value);

        return *this;
    }

} // namespace TiRHI::Vulkan
