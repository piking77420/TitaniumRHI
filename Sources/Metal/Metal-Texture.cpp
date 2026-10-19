#include <Metal/Metal-Texture.hpp>

namespace TiRHI::Metal
{
    Texture::Texture(RHI& rhi)
        : BaseTexture(rhi)
    {

    }

    Texture::~Texture() = default;

    bool build()
    {
        return false;
    }

} // namespace TiRHI::Metal