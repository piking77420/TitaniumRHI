#include <Vulkan/Vulkan-RHI.hpp>

namespace TiRHI
{
    RHI::RHI(const TiRHI::RhiCreate& create)
        : m_instance(create)
        , m_device(m_instance)
    {
    }

    RHI::~RHI()
    {
    }

    void RHI::waitForDeviceIdle()
    {
    }
} // namespace TiRHI
