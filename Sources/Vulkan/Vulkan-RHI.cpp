#include <Vulkan/Vulkan-RHI.hpp>

#include <Titanium/Log.hpp>

namespace TiRHI
{
    VulkanRHI::VulkanRHI(const TiRHI::RhiCreate& create)
        : RHI<VulkanRHI>(create)
        , m_device(m_instance, m_adapters)
    {
    }

    void VulkanRHI::waitImpl()
    {
    }

    bool VulkanRHI::createDeviceImpl()
    {
        m_device.createDevice(m_adapters);
        return m_device.getDevice() != VK_NULL_HANDLE;
    }

    bool VulkanRHI::createDeviceImpl(const Adapter& adapter)
    {
        size_t index = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < m_adapters.size(); i++)
        {
            if (m_adapters[i].getName() == adapter.getName())
            {
                index = i;
                break;
            }
        }
        if (index == std::numeric_limits<size_t>::max())
            RHI_LOG_FATAL(L"Couldn't find request adapter", RhiApi::Vulkan);

        m_device.createDevice(index, m_adapters);
        return m_device.getDevice() != VK_NULL_HANDLE;
    }
    const Adapter* VulkanRHI::getUsedAdapterImpl() const
    {
        const size_t index = m_device.getSelectPhyscialDeviceIndex();

        if (index == std::numeric_limits<size_t>::max() || index >= m_adapters.size())
            return nullptr;

        return &m_adapters[index];
    }
} // namespace TiRHI
