#ifndef TITANIUM_METAL_TEXTURE_H
#define TITANIUM_METAL_TEXTURE_H

#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHI-BaseTexture.hpp>

namespace CA
{
    class MetalDrawable;
} // namespace MTL

namespace MTL
{
    class Texture;
} // namespace MTL

namespace TiRHI::Metal
{
    class RHI;

    class Texture : public BaseTexture<Texture, RHI>
    {
    public:
        Texture() = delete;
        ~Texture();
        RHI_MOVE_ONLY(Texture);
        Texture(RHI& rhi);

        struct Private
        {
            static bool build(Texture& texture, CA::MetalDrawable* metalDrawable);
        };

        bool build();

    private:
        friend Private;

        MTL::Texture* m_texture = nullptr;

        bool m_ownBySwapChain = false;
    };
} // namespace TiRHI::Metal

#endif // TITANIUM_METAL_TEXTURE_H