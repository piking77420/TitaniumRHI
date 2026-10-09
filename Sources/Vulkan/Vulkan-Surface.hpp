#ifndef TITANIUM_VULKAN_SURFACE_H
#define TITANIUM_VULKAN_SURFACE_H

#include <Titanium/RHI-Surface.hpp>
#include <Titanium/RHITypes.hpp>
#include <vulkan/vulkan.hpp>

namespace TiRHI::Vulkan
{
    class RHI;

    class Surface : public BaseSurface<Surface, RHI>
    {
    public:
        Surface() = delete;
        ~Surface();
        RHI_MOVE_CONSTRUCT_ONLY(Surface)
        Surface(RHI& rhi)
            : BaseSurface(rhi)
        {
        }

        bool build(WindowHandle windowHandle);

        vk::SurfaceKHR getSurfaceNative() const;

    private:
        vk::SurfaceKHR m_surface = VK_NULL_HANDLE;
    };
} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SURFACE_H
