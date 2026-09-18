#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI
{
    RHI::RHI()
        : m_device(m_instance)
    {
    }

    RHI::~RHI()
    {
    }

    void RHI::waitForDeviceIdle()
    {
    }
} // namespace TiRHI
