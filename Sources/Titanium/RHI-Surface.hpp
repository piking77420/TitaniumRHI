#ifndef TITANIUM_SURFACE_H
#define TITANIUM_SURFACE_H

#include <Titanium/RHI-Object.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseSurface : public Object<T, TRHI>
    {
    public:
        BaseSurface() = delete;
        ~BaseSurface() = default;
        RHI_MOVE_CONSTRUCT_ONLY(BaseSurface)
        BaseSurface(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
        }

    private:
    };
}

#endif // TITANIUM_SURFACE_H
