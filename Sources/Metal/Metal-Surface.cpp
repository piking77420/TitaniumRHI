#include <Metal/Metal-Surface.hpp>

namespace TiRHI::Metal
{
    bool Surface::build(WindowHandle windowHandle)
    {
        m_windowHandle = windowHandle;
        return m_windowHandle;
    }

} // namespace TiRHI::Metal
