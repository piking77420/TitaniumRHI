#ifndef TITANIUM_METAL_SURFACE_H
#define TITANIUM_METAL_SURFACE_H

#include <Titanium/RHI-Surface.hpp>

namespace TiRHI::Metal
{
    class RHI;

    class Surface : public BaseSurface<Surface, RHI>
    {
    public:
        Surface();
        ~Surface();
        RHI_MOVE_ONLY(Surface)
        Surface(RHI& rhi)
            : BaseSurface(rhi)
        {
        }

    private:
    };

}

#endif // TITANIUM_METAL_SURFACE_H
