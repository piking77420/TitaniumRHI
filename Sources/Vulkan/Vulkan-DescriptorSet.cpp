#include <Vulkan/Vulkan-DescriptorSet.hpp>
#include <Vulkan/Private/DescriptorManager.hpp>
#include <Vulkan/Vulkan-Texture.hpp>
#include <Vulkan/Vulkan-Sampler.hpp>

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

} // namespace TiRHI::Vulkan
