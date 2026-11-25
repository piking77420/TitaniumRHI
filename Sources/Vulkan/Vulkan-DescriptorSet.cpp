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
        , m_handle(std::exchange(other.m_handle, {}))
    {
    }
    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
    {
        if (this == &other)
            return *this;

        BaseDescriptorSet::operator=(std::move(other));
        reset();
        m_handle = std::exchange(other.m_handle, {});

        return *this;
    }

    bool DescriptorSet::build()
    {
        TiRHI::Vulkan::Private::DescriporManager& descriporManager =
            Device::Private::getDescriptorManager(getDevice());

        const std::optional<TiRHI::Vulkan::Private::DescriptorSetHandle> handle =
            descriporManager.allocAndWriteDescriptors(getDevice().getNativeDevice(), getBindingsLayout(),
                                                          getNbrOfDescriptorSet());
        if (!handle)
            return false;

        for (size_t i = 0; i < handle->descriptorCount; i++)
        {
            vk::DebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.objectType = vk::ObjectType::eDescriptorSet;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkDescriptorSet>(handle->descriptorSets[i].descriptorSet));
            nameInfo.pObjectName = getName().data();

            getDevice().getNativeDevice().setDebugUtilsObjectNameEXT(nameInfo);
        }

        m_handle = *handle;
        return true;
    }

    void DescriptorSet::reset()
    {
        // TODO defreed destroy
        auto& defferedDeleteQueue = Device::Private::getDeferredDeletionQueue(getDevice());
     

        if (m_handle.descriptorCount > 0)
        {
            TiRHI::Vulkan::Private::DescriporManager& descriporManager =
                Device::Private::getDescriptorManager(getDevice());

            bool areEqual = false;
            uint64_t lastFrameUsed = m_handle.descriptorSets[0].getLastFrameUsed();
            for (size_t i = 1; i < m_handle.descriptorCount; i++)
            {
                areEqual = lastFrameUsed == m_handle.descriptorSets[i].getLastFrameUsed();
            }

            if (areEqual)
            {
                defferedDeleteQueue.retire(
                    lastFrameUsed,
                                           [descriptorM = &descriporManager, layout = getBindingsLayout(),
                                            handle = std::move(m_handle)]() mutable
                    {
                        descriptorM->release(layout, handle);

                    });

            }
            else
            {
                // TODO optimse this
                // expensive copy of layout for each vkdescriptor
                // may use an better hash map
                // -> cached value hash unique or use imutable map addrss

                for (size_t i = 0; i < m_handle.descriptorCount; i++)
                {
                    defferedDeleteQueue.retire(m_handle.descriptorSets[i].getLastFrameUsed(),
                                               [descriptorM = &descriporManager, layout = getBindingsLayout(),
                                                handle = std::move(m_handle)]() mutable
                                               {
                                                   descriptorM->release(layout, handle);
                                               });
                }
            }

        }
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
