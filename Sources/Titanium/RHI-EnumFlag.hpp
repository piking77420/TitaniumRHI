#ifndef TITANIUM_ENUM_FLAG_H
#define TITANIUM_ENUM_FLAG_H

#include <utility>

#define RHI_DEFINE_ENUM_FLAGS(Enum)                                                                                    \
    constexpr Enum operator|(Enum lhs, Enum rhs) noexcept                                                              \
    {                                                                                                                  \
        return static_cast<Enum>(std::to_underlying(lhs) | std::to_underlying(rhs));                                   \
    }                                                                                                                  \
                                                                                                                       \
    constexpr Enum operator&(Enum lhs, Enum rhs) noexcept                                                              \
    {                                                                                                                  \
        return static_cast<Enum>(std::to_underlying(lhs) & std::to_underlying(rhs));                                   \
    }                                                                                                                  \
                                                                                                                       \
    constexpr Enum operator~(Enum value) noexcept                                                                      \
    {                                                                                                                  \
        return static_cast<Enum>(~std::to_underlying(value));                                                          \
    }                                                                                                                  \
                                                                                                                       \
    constexpr Enum& operator|=(Enum& lhs, Enum rhs) noexcept                                                           \
    {                                                                                                                  \
        return lhs = lhs | rhs;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    constexpr Enum& operator&=(Enum& lhs, Enum rhs) noexcept                                                           \
    {                                                                                                                  \
        return lhs = lhs & rhs;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    constexpr bool hasFlag(Enum flags, Enum flag) noexcept                                                             \
    {                                                                                                                  \
        return (flags & flag) == flag;                                                                                 \
    }

#endif // TITANIUM_ENUM_FLAG_H
