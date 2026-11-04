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
        BaseTexture(BaseTexture&&) noexcept = default;
        BaseTexture& operator=(BaseTexture&&) noexcept = default;
        BaseTexture(TRHI& rhi)
            : Resource<T, TRHI>(rhi)
        {
        }

        uint32_t getWidth() const noexcept
        {
            return m_width;
        }

        T& setWidth(uint32_t width)
        {
            m_width = width;
            return static_cast<T&>(*this);
        }

        uint32_t getHeight() const noexcept
        {
            return m_height;
        }

        T& setHeight(uint32_t height)
        {
            m_height = height;
            return static_cast<T&>(*this);
        }

        TextureType getType() const noexcept
        {
            return m_type;
        }

        T& setType(TextureType type)
        {
            m_type = type;
            return static_cast<T&>(*this);
        }

        TextureFlags getFlags() const noexcept
        {
            return m_flags;
        }

        T& setFlags(TextureFlags flags)
        {
            m_flags = flags;
            return static_cast<T&>(*this);
        }

    protected:
        uint32_t m_width = 0;

        uint32_t m_height = 0;

        TextureType m_type = TextureType::Texture2D;

        TextureFlags m_flags;
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_TEXTURE_H
