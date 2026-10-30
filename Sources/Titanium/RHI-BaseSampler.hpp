#ifndef TITANIUM_BASE_SAMPLER_H
#define TITANIUM_BASE_SAMPLER_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Object.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseSampler : public Object<T, TRHI>
    {
    public:
        BaseSampler() = delete;
        ~BaseSampler() = default;
        BaseSampler(BaseSampler&& other) = default;
        BaseSampler& operator=(BaseSampler&& other) = default;
        BaseSampler(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
        }

        // Filtering

        Filter getMagFilter() const
        {
            return m_magFilter;
        }

        T& setMagFilter(Filter newFilter)
        {
            m_magFilter = newFilter;
            return static_cast<T&>(*this);
        }

        Filter getMinFilter() const
        {
            return m_minFilter;
        }

        T& setMinFilter(Filter newFilter)
        {
            m_minFilter = newFilter;
            return static_cast<T&>(*this);
        }

        Filter getMipMapFilter() const
        {
            return m_mipmapFilter;
        }

        T& setMipMapFilter(Filter newFilter)
        {
            m_mipmapFilter = newFilter;
            return static_cast<T&>(*this);
        }

        // Address modes

        SamplerAddressMode getAddressModeU() const
        {
            return m_addressModeU;
        }

        T& setAddressModeU(SamplerAddressMode newAddressMode)
        {
            m_addressModeU = newAddressMode;
            return static_cast<T&>(*this);
        }

        SamplerAddressMode getAddressModeV() const
        {
            return m_addressModeV;
        }

        T& setAddressModeV(SamplerAddressMode newAddressMode)
        {
            m_addressModeV = newAddressMode;
            return static_cast<T&>(*this);
        }

        SamplerAddressMode getAddressModeW() const
        {
            return m_addressModeW;
        }

        T& setAddressModeW(SamplerAddressMode newAddressMode)
        {
            m_addressModeW = newAddressMode;
            return static_cast<T&>(*this);
        }

        // Mip LOD

        float getMipLodBias() const
        {
            return m_mipLodBias;
        }

        T& setMipLodBias(float newBias)
        {
            m_mipLodBias = newBias;
            return static_cast<T&>(*this);
        }

        float getMinLod() const
        {
            return m_minLod;
        }

        T& setMinLod(float newMinLod)
        {
            m_minLod = newMinLod;
            return static_cast<T&>(*this);
        }

        float getMaxLod() const
        {
            return m_maxLod;
        }

        T& setMaxLod(float newMaxLod)
        {
            m_maxLod = newMaxLod;
            return static_cast<T&>(*this);
        }

        // Anisotropic filtering

        bool getAnisotropyEnabled() const
        {
            return m_anisotropy;
        }

        T& setAnisotropyEnabled(bool enabled)
        {
            m_anisotropy = enabled;
            return static_cast<T&>(*this);
        }

        float getMaxAnisotropy() const
        {
            return m_maxAnisotropy;
        }

        T& setMaxAnisotropy(float newMaxAnisotropy)
        {
            m_maxAnisotropy = newMaxAnisotropy;
            return static_cast<T&>(*this);
        }

        // Depth comparison

        bool getDepthCompareEnabled() const
        {
            return m_depthCompare;
        }

        T& setDepthCompareEnabled(bool enabled)
        {
            m_depthCompare = enabled;
            return static_cast<T&>(*this);
        }

        CompareOp getDepthCompareOp() const
        {
            return m_depthCompareOp;
        }

        T& setDepthCompareOp(CompareOp newCompareOp)
        {
            m_depthCompareOp = newCompareOp;
            return static_cast<T&>(*this);
        }

    private:
        Filter m_magFilter = Filter::Linear;

        Filter m_minFilter = Filter::Linear;

        Filter m_mipmapFilter = Filter::Linear;

        SamplerAddressMode m_addressModeU = SamplerAddressMode::Repeat;

        SamplerAddressMode m_addressModeV = SamplerAddressMode::Repeat;

        SamplerAddressMode m_addressModeW = SamplerAddressMode::Repeat;

        float m_mipLodBias = 0.0f;

        float m_minLod = 0.0f;

        float m_maxLod = LodClampNone;

        bool m_anisotropy = false;

        float m_maxAnisotropy = 1.0f;

        bool m_depthCompare = false;

        CompareOp m_depthCompareOp = CompareOp::eLessOrEqual;
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_SAMPLER_H
