#ifndef TITANIUM_METAL_TEXTURE_H
#define TITANIUM_METAL_TEXTURE_H

#include <Titanium/RHI-BaseTexture.hpp>

namespace TiRHI::Metal
{
    class RHI;

    class Texture : public BaseTexture<Texture, RHI>
    {
    public:
        Texture() = delete;
        ~Texture();
        RHI_MOVE_ONLY(Texture)
        Texture(RHI& rhi);

        bool build();

    private:
    };
} // namespace TiRHI::Metal


#endif // TITANIUM_METAL_TEXTURE_H