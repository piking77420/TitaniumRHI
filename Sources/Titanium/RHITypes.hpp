#ifndef TITANIUM_RHI_TYPES_H
#define TITANIUM_RHI_TYPES_H

#include <string>
#include <functional>

namespace TiRHI
{
    enum struct RhiMessageSeverity
    {
        Verbose,
        Info,
        Warning,
        Error,
        Fatal
    };

    constexpr std::string_view toString(RhiMessageSeverity rhiMessageSeverity)
    {
        using namespace std::literals;

        switch (rhiMessageSeverity)
        {
        case RhiMessageSeverity::Verbose:
            return "Verbose"sv;
        case RhiMessageSeverity::Info:
            return "Info"sv;
        case RhiMessageSeverity::Warning:
            return "Warning"sv;
        case RhiMessageSeverity::Error:
            return "Error"sv;
        case RhiMessageSeverity::Fatal:
            return "Fatal"sv;
        }

        return "Unknown"sv;
    }

    constexpr std::wstring_view toWstring(RhiMessageSeverity rhiMessageSeverity)
    {
        using namespace std::literals;

        switch (rhiMessageSeverity)
        {
        case RhiMessageSeverity::Verbose:
            return L"Verbose"sv;
        case RhiMessageSeverity::Info:
            return L"Info"sv;
        case RhiMessageSeverity::Warning:
            return L"Warning"sv;
        case RhiMessageSeverity::Error:
            return L"Error"sv;
        case RhiMessageSeverity::Fatal:
            return L"Fatal"sv;
        }

        return L"Unknown"sv;
    }

    enum struct RhiApi
    {
        DirectX12,
        Metal,
        Vulkan
    };

    constexpr std::string_view toString(RhiApi rhiApi)
    {
        using namespace std::literals;

        switch (rhiApi)
        {
        case RhiApi::DirectX12:
            return "DirectX12"sv;
        case RhiApi::Metal:
            return "Metal"sv;
        case RhiApi::Vulkan:
            return "Vulkan"sv;
        }
        return "Unknown"sv;
    }

    constexpr std::wstring_view toWstring(RhiApi rhiApi)
    {
        using namespace std::literals;

        switch (rhiApi)
        {
        case RhiApi::DirectX12:
            return L"DirectX12"sv;
        case RhiApi::Metal:
            return L"Metal"sv;
        case RhiApi::Vulkan:
            return L"Vulkan"sv;
        }
        return L"Unknown"sv;
    }

    using LogCallBackSignature = void (*)(const std::wstring&, RhiApi, RhiMessageSeverity);

    struct RhiCreate
    {
        LogCallBackSignature logCallback;
    };

} // namespace TiRHI

#endif // TITANIUM_RHI_TYPES_H
