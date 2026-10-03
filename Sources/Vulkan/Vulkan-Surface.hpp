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
        ~Surface() = default;
        Surface(RHI& rhi)
            : BaseSurface(rhi)
        {
        }

        bool build(WindowHandle windowHandle);

        vk::SurfaceKHR getSurfaceNative() const;

    private:
        vk::UniqueSurfaceKHR m_surface;
    };
} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SURFACE_H
