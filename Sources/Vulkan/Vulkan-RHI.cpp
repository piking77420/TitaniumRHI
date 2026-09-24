#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI
{
    VulkanRHI::VulkanRHI(const TiRHI::RhiCreate& create)
        : RHI<VulkanRHI>(create)
        , m_device(m_instance)
    {
    }

    void VulkanRHI::waitImpl()
    {
    }
} // namespace TiRHI
