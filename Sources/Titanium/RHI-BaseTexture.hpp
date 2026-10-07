#ifndef TITANIUM_BASE_TEXTURE_H
#define TITANIUM_BASE_TEXTURE_H

#include <Titanium/RHI-Resource.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseTexture : public Resource<T, TRHI>
    {
    public:
        BaseTexture() = delete;
        ~BaseTexture() = default;

        BaseTexture(TRHI& rhi)
            : Object<T, TRHI>()
        {
        }

    private:
    };
} // namespace TiRHI

#endif // TITANIUM_BASE_TEXTURE_H
