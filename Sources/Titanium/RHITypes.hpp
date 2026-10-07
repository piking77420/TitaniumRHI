#ifndef TITANIUM_RHI_TYPES_H
#define TITANIUM_RHI_TYPES_H

#include <functional>

#include <Titanium/RHI-EnumToString.hpp>

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

#define RHI_API_LIST(X)                                                                                                \
    X(None)                                                                                                            \
    X(DirectX12)                                                                                                       \
    X(Metal)                                                                                                           \
    X(Vulkan)

    enum struct RhiApi
    {
#define X(name) name,
        RHI_API_LIST(X)
#undef X
    };
    IMPLEMENT_TO_STRING_TITANIUM(RhiApi, RHI_API_LIST)

#define RHI_FORMAT_LIST(X)                                                                                             \
    X(R8_UNorm)                                                                                                        \
    X(R8G8_UNorm)                                                                                                      \
    X(R8G8B8A8_UNorm)                                                                                                  \
    X(R8G8B8A8_UNorm_SRGB)                                                                                             \
    X(B8G8R8A8_UNorm)                                                                                                  \
    X(B8G8R8A8_UNorm_SRGB)                                                                                             \
    X(R16_Float)                                                                                                       \
    X(R16G16_Float)                                                                                                    \
    X(R16G16B16A16_Float)                                                                                              \
    X(R32_Float)                                                                                                       \
    X(R32G32_Float)                                                                                                    \
    X(R32G32B32A32_Float)                                                                                              \
    X(R32_UInt)                                                                                                        \
    X(R32G32_UInt)                                                                                                     \
    X(R32G32B32A32_UInt)                                                                                               \
    X(D16_UNorm)                                                                                                       \
    X(D24_UNorm_S8_UInt)                                                                                               \
    X(D32_Float)                                                                                                       \
    X(D32_Float_S8_UInt)

    enum struct Format
    {
#define X(name) name,
        RHI_FORMAT_LIST(X)
#undef X
    };

    IMPLEMENT_TO_STRING_TITANIUM(Format, RHI_FORMAT_LIST)

#define RHI_LOAD_OP_LIST(X)                                                                                            \
    X(LoadOp)                                                                                                          \
    X(Clear)                                                                                                           \
    X(DontCare)

    enum struct LoadOp
    {
#define X(name) name,
        RHI_LOAD_OP_LIST(X)
#undef X
    };
    IMPLEMENT_TO_STRING_TITANIUM(LoadOp, RHI_LOAD_OP_LIST)

#define RHI_STORE_OP_LIST(X)                                                                                           \
    X(Store)                                                                                                           \
    X(DontCare)                                                                                                        \
    X(None)

    enum struct StoreOp
    {
#define X(name) name,
        RHI_STORE_OP_LIST(X)
#undef X
    };

    IMPLEMENT_TO_STRING_TITANIUM(StoreOp, RHI_STORE_OP_LIST)

#define RHI_SAMPLE_COUNT_LIST(X)                                                                                       \
    X(Count1)                                                                                                          \
    X(Count2)                                                                                                          \
    X(Count4)                                                                                                          \
    X(Count8)                                                                                                          \
    X(Count16)                                                                                                         \
    X(Count32)                                                                                                         \
    X(Count64)

    enum struct SampleCount
    {
#define X(name) name,
        RHI_SAMPLE_COUNT_LIST(X)
#undef X
    };

    IMPLEMENT_TO_STRING_TITANIUM(SampleCount, RHI_SAMPLE_COUNT_LIST)

#define RHI_RESOURCE_STATE_LIST(X)                                                                                     \
    X(Undefined)                                                                                                       \
    X(Common)                                                                                                          \
    X(VertexBuffer)                                                                                                    \
    X(IndexBuffer)                                                                                                     \
    X(ConstantBuffer)                                                                                                  \
    X(ShaderResource)                                                                                                  \
    X(UnorderedAccess)                                                                                                 \
    X(RenderTarget)                                                                                                    \
    X(DepthWrite)                                                                                                      \
    X(DepthRead)                                                                                                       \
    X(CopySource)                                                                                                      \
    X(CopyDestination)                                                                                                 \
    X(IndirectArgument)                                                                                                \
    X(Present)

    enum struct ResourceState
    {
#define X(name) name,
        RHI_RESOURCE_STATE_LIST(X)
#undef X
    };

    IMPLEMENT_TO_STRING_TITANIUM(ResourceState, RHI_RESOURCE_STATE_LIST)

#define RHI_PIPELINE_TYPE_LIST(X)                                                                                      \
    X(Graphics)                                                                                                        \
    X(Compute)                                                                                                         \
    X(RayTracing)

    enum struct PipelineType
    {
#define X(name) name,
        RHI_PIPELINE_TYPE_LIST(X)
#undef X
    };
    IMPLEMENT_TO_STRING_TITANIUM(PipelineType, RHI_PIPELINE_TYPE_LIST)

    using LogCallBackSignature = void (*)(const std::wstring&, RhiApi, RhiMessageSeverity);

    struct RhiCreate
    {
        bool useDebugLabel = false;
        size_t frameInFlight = 2;
        LogCallBackSignature logCallback;
    };

    using WindowHandle = void*;

} // namespace TiRHI

#endif // TITANIUM_RHI_TYPES_H
