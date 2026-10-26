#include <Metal/Metal-Surface.hpp>

namespace TiRHI::Metal
{
    Surface::~Surface() = default;

    bool Surface::build(WindowHandle windowHandle)
    {
        m_windowHandle = windowHandle;
        return m_windowHandle;
    }

} // namespace TiRHI::Metal
