#ifndef TITANIUM_BASE_TEXTURE_H
#define TITANIUM_BASE_TEXTURE_H

#include <Titanium/RHI-Resource.hpp>
#include <Titanium/RHI-EnumFlag.hpp>

namespace TiRHI
{
#define RHI_TEXTURE_FLAG_LIST(X)                                                                                       \
    X(None, 0)                                                                                                         \
    X(ShaderResource, 1u << 0)                                                                                         \
    X(RenderTarget, 1u << 1)                                                                                           \
    X(UnorderedAccess, 1u << 3)                                                                                        \
    X(CopySource, 1u << 4)                                                                                             \
    X(CopyDestination, 1u << 5)

    enum struct TextureFlags : uint32_t
    {
#define X(name, value) name = value,
        RHI_TEXTURE_FLAG_LIST(X)
#undef X
    };

    template<typename T, typename TRHI>
    class BaseTexture : public Resource<T, TRHI>
    {
    public:
        BaseTexture() = delete;
        ~BaseTexture() = default;
        RHI_MOVE_ONLY(BaseTexture)
        BaseTexture(TRHI& rhi)
            : Resource<T, TRHI>(rhi)
        {
        }

        TextureFlags getFlag() const
        {
            return m_flags;
        }

        T& setFlag(TextureFlags newFlag) const
        {
            m_flags = newFlag;
            return reinterpret_cast<T&>(*this);
        }

    protected:
        TextureFlags m_flags;
    };

    RHI_DEFINE_ENUM_FLAGS(TextureFlags);
    IMPLEMENT_TO_STRING_FLAGS_TITANIUM(TextureFlags, RHI_TEXTURE_FLAG_LIST)
} // namespace TiRHI

#endif // TITANIUM_BASE_TEXTURE_H
