#ifndef TITANIUM_VULKAN_SURFACE_H
#define TITANIUM_VULKAN_SURFACE_H

#if defined(_WIN32)
#define NOMINMAX
#endif

#include <Titanium/RHI-Surface.hpp>
#include <Titanium/RHITypes.hpp>
#include <vulkan/vulkan.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Surface : public BaseSurface<Surface, Device>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        Surface() = delete;
        ~Surface();
        Surface(Surface&& other);
        Surface& operator=(Surface&& other);
        Surface(Device& device)
            : BaseSurface(device)
        {
        }

        bool build(WindowHandle windowHandle);

        vk::SurfaceKHR getSurfaceNative() const;

    private:
        vk::SurfaceKHR m_surface = VK_NULL_HANDLE;

        void destroy();
    };
} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SURFACE_H
