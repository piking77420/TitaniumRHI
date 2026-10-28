#ifndef TITANIUM_METAL_SURFACE_H
#define TITANIUM_METAL_SURFACE_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Surface.hpp>

namespace TiRHI::Metal
{
    class RHI;

    class Surface : public BaseSurface<Surface, RHI>
    {
    public:
        Surface() = delete;
        ~Surface();
        RHI_MOVE_ONLY(Surface);
        Surface(RHI& rhi)
            : BaseSurface(rhi)
        {
        }

        struct Private 
        {
            static WindowHandle getWindowHandle(Surface& Surface)
            {
                return Surface.m_windowHandle;
            }
        };


        bool build(WindowHandle windowHandle);

    private:
        friend Private;
        WindowHandle m_windowHandle{nullptr};
    };
}

#endif // TITANIUM_METAL_SURFACE_H
