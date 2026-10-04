#include <Vulkan/Vulkan-Surface.hpp>
#include <format>
#include <Titanium/Log.hpp>
#include <Vulkan/Vulkan-Surface.hpp>
#include <format>
#include <Titanium/Log.hpp>
#include <Vulkan-RHI.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Volk/volk.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#else

#include <vulkan/vulkan.hpp>

#endif // _WIN32

namespace TiRHI::Vulkan
{
    Surface::~Surface()
    {
        if (m_surface)
            getRHI().getNativeInstance().destroySurfaceKHR(m_surface);
    }

    bool Surface::build(WindowHandle windowHandle)
    {
        if (!windowHandle)
        {
            RHI_LOG_ERROR(std::format(L"Failed to create Surface : {}", getNameW()), RhiApi::Vulkan);
            return false;
        }

        VkSurfaceKHR surface;
        bool isSurfaceOk = false;
#ifdef _WIN32

        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = static_cast<decltype(VkWin32SurfaceCreateInfoKHR::hwnd)>(windowHandle);
        createInfo.hinstance = GetModuleHandle(nullptr);

        isSurfaceOk = VulkanCheckErrorStatus(
            volk::vkCreateWin32SurfaceKHR(getRHI().getNativeInstance(), &createInfo, nullptr, &surface));

#endif // _WIN32

        m_surface = surface;

        return isSurfaceOk;
    }

    vk::SurfaceKHR Surface::getSurfaceNative() const
    {
        return m_surface;
    }

} // namespace TiRHI::Vulkan
