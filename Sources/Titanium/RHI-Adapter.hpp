#ifndef TITANIUM_ADAPTER_H
#define TITANIUM_ADAPTER_H

#include <span>
#include <string>
#include <string_view>

#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    class Adapter
    {
    public:
        enum struct Features
        {
            RayTracing,
            MeshShader,
            VariableRateShading
        };

        Adapter() = default;
        ~Adapter() = default;

        Adapter(Adapter&& other) noexcept = default;
        Adapter(const Adapter& other) = default;

        Adapter& operator=(Adapter&& other) noexcept = default;
        Adapter& operator=(const Adapter& other) = default;

        Adapter(const std::string& name, const std::vector<Features>& features, uint64_t vramMemoryGb,
                uint32_t vendorId);

        const std::string_view getName() const noexcept
        {
            return m_name;
        }

        const std::string_view getVendor() const noexcept
        {
            return m_name;
        }

        const std::span<const Features> getFeatures() const noexcept
        {
            return std::span<const Features>(m_features);
        }

    private:
        std::string m_name;

        std::string m_vendor;

        std::vector<Features> m_features;

        uint64_t m_vramMemoryGb{0ull};

        constexpr static std::string_view vendorName(uint32_t vendorId)
        {
            using namespace std::literals;

            switch (vendorId)
            {
            case 0x10DE:
                return "NVIDIA"sv;

            case 0x1002:
            case 0x1022:
                return "AMD"sv;

            case 0x8086:
                return "Intel"sv;

            case 0x13B5:
                return "ARM"sv;

            case 0x5143:
                return "Qualcomm"sv;

            case 0x106B:
                return "Apple"sv;

            default:
                return "Unknown"sv;
            }
        }
    };
} // namespace TiRHI

#endif // TITANIUM_ADAPTER_H
