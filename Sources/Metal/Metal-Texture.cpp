#include <Metal/Metal-Texture.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Metal
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {
    }

    Texture::~Texture() = default;

    bool Texture::Private::build(Texture& texture, CA::MetalDrawable* metalDrawable)
    {
        texture.m_texture = metalDrawable->texture();
        if (texture.m_texture == nullptr)
            return false;

        texture.m_ownBySwapChain = true;
        return true;
    }

    bool Texture::build()
    {
        RHI_LOG_ERROR(L"Texture::build TODO", RhiApi::Metal);
        return false;
    }

} // namespace TiRHI::Metal