#include <DirectX12/DirectX12-Surface.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <format>
#include <Titanium/Log.hpp>

namespace TiRHI::DirectX12
{
    Surface::Surface(RHI& rhi)
        : BaseSurface(rhi)
    {
    }

    bool Surface::build(WindowHandle window)
    {
        if (!window)
        {
            RHI_LOG_ERROR(std::format(L"Failed to create Surface : {}", getNameW()), RhiApi::Vulkan);
            return false;
        }

        m_windowHandle = window;
        return m_windowHandle;
    }
}
