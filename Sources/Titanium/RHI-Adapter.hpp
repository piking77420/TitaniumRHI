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
        };

        static constexpr std::string_view toString(Features features) noexcept
        {
            using namespace std::literals;

            switch (features)
            {
            case TiRHI::Adapter::Features::RayTracing:
                return "RayTracing"sv;
            case TiRHI::Adapter::Features::MeshShader:
                return "MeshShader"sv;
            default:
                return "Unknow"sv;
            }
        };

        static constexpr std::wstring_view toWString(Features features) noexcept
        {
            using namespace std::literals;

            switch (features)
            {
            case TiRHI::Adapter::Features::RayTracing:
                return L"RayTracing"sv;
            case TiRHI::Adapter::Features::MeshShader:
                return L"MeshShader"sv;
            default:
                return L"Unknow"sv;
            }
        };

        struct Properties
        {
            struct Limits
            {
                uint32_t minUniformBufferOffset;
            } limits;

            struct MemoryLimits
            {
                uint64_t vramMemoryBytes{0ull};
            } memoryLimits;

            enum struct Type
            {
                DiscreteGpu,
                IntegratedGpu,
                VirtualGpu,
                Cpu,
                Unknow
            } deviceType;

            static constexpr std::string_view toString(Type type) noexcept
            {
                using namespace std::literals;

                switch (type)
                {
                case Type::DiscreteGpu:
                    return "DiscreteGpu"sv;
                case Type::IntegratedGpu:
                    return "IntegratedGpu"sv;
                case Type::VirtualGpu:
                    return "VirtualGpu"sv;
                case Type::Cpu:
                    return "Cpu"sv;
                case Type::Unknow:
                default:
                    return "Unknow"sv;
                }
            };

            static constexpr std::wstring_view toWString(Type type) noexcept
            {
                using namespace std::literals;

                switch (type)
                {
                case Type::DiscreteGpu:
                    return L"DiscreteGpu"sv;
                case Type::IntegratedGpu:
                    return L"IntegratedGpu"sv;
                case Type::VirtualGpu:
                    return L"VirtualGpu"sv;
                case Type::Cpu:
                    return L"Cpu"sv;
                case Type::Unknow:
                default:
                    return L"Unknow"sv;
                }
            };

            auto operator<=>(const Properties& other) const = default;
        };

        Adapter() = default;
        ~Adapter() = default;

        Adapter(Adapter&& other) noexcept = default;
        Adapter(const Adapter& other) = default;

        Adapter& operator=(Adapter&& other) noexcept = default;
        Adapter& operator=(const Adapter& other) = default;

        Adapter(const std::string_view& name, const std::vector<Features>& features, const Properties& property,
                uint32_t vendorId);

        const std::string_view getName() const noexcept
        {
            return m_name;
        }

        const std::string_view getVendor() const noexcept
        {
            return m_vendor;
        }

        const Properties& getProperties() const noexcept
        {
            return m_properties;
        }

        const std::span<const Features> getFeatures() const noexcept
        {
            return std::span<const Features>(m_features);
        }

        template<Features F>
        bool supportFeatures()
        {
            return std::ranges::find_if(m_features, F) != m_features.end();
        }

        auto operator<=>(const Adapter& other) const = default;

    private:
        std::string m_name;

        std::string m_vendor;

        Properties m_properties;

        // TODO may use flag
        std::vector<Features> m_features;

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
