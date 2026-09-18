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
        switch (rhiMessageSeverity)
        {
        case RhiMessageSeverity::Verbose:
            return "Verbose";
        case RhiMessageSeverity::Info:
            return "Info";
        case RhiMessageSeverity::Warning:
            return "Warning";
        case RhiMessageSeverity::Error:
            return "Error";
        case RhiMessageSeverity::Fatal:
            return "Fatal";
        }

        return "Unknown";
    }

    enum struct RhiMessageLocation
    {
        Rhi,
        Api
    };

    constexpr std::string_view toString(RhiMessageLocation rhiMessageLocation)
    {
        switch (rhiMessageLocation)
        {
        case RhiMessageLocation::Rhi:
            return "Rhi";
        case RhiMessageLocation::Api:
            return "Api";
        }
        return "Unknown";
    }

    enum struct RhiApi
    {
        DirectX12,
        Metal,
        Vulkan
    };

    constexpr std::string_view toString(RhiApi rhiApi)
    {
        switch (rhiApi)
        {
        case RhiApi::DirectX12:
            return "DirectX12";
        case RhiApi::Metal:
            return "Metal";
        case RhiApi::Vulkan:
            return "Vulkan";
        }
        return "Unknown";
    }

    struct RhiCreate
    {
        std::function<void(const std::wstring&, RhiMessageLocation, RhiApi, RhiMessageSeverity)> logCallback;
    };

}

#endif // TITANIUM_RHI_TYPES_H
