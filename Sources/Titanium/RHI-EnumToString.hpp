#ifndef RHI_ENUM_TO_STRING_H
#define RHI_ENUM_TO_STRING_H

#include <string>
#include <string_view>

#define WIDEN_IMPL_TITANIUM(x) L##x
#define WIDEN_TITANIUM(x) WIDEN_IMPL_TITANIUM(x)

#define TO_STRING_CASE_TITANIUM(name)                                                                                  \
    case EnumT::name:                                                                                                  \
        return #name;

#define TO_WSTRING_CASE_TITANIUM(name)                                                                                 \
    case EnumT::name:                                                                                                  \
        return WIDEN_TITANIUM(#name);

#define IMPLEMENT_TO_STRING_TITANIUM(EnumType, List)                                                                   \
    constexpr std::string_view toString(EnumType value)                                                                \
    {                                                                                                                  \
        using EnumT = EnumType;                                                                                        \
        switch (value)                                                                                                 \
        {                                                                                                              \
            List(TO_STRING_CASE_TITANIUM)                                                                              \
        }                                                                                                              \
        return "Unknown";                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    constexpr std::wstring_view toWstring(EnumType value)                                                              \
    {                                                                                                                  \
        using EnumT = EnumType;                                                                                        \
        switch (value)                                                                                                 \
        {                                                                                                              \
            List(TO_WSTRING_CASE_TITANIUM)                                                                             \
        }                                                                                                              \
        return L"Unknown";                                                                                             \
    }

#endif // RHI_ENUM_TO_STRING_H
