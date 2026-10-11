#ifndef TITANIUM_STRONG_TYPE_ID_H
#define TITANIUM_STRONG_TYPE_ID_H

#include <concepts>
#include <limits>
#include <type_traits>
namespace TiRHI::Private
{
    template<typename T>
    constexpr bool IsChar =
        std::is_same_v<std::remove_cvref_t<T>, char> || std::is_same_v<std::remove_cvref_t<T>, wchar_t>;

    // TODO add tag
    template<typename T, typename Tag>
    requires(std::is_fundamental_v<std::remove_cvref_t<T>> && !std::is_floating_point_v<std::remove_cvref_t<T>> &&
             !IsChar<T>)
    class StrongTypeID
    {
    public:
        using _Underlying = T;
        static constexpr _Underlying NullValue = std::numeric_limits<_Underlying>::max();
        explicit StrongTypeID() = default;
        ~StrongTypeID() = default;

        static StrongTypeID FromUnderlying(_Underlying value) noexcept
        {
            StrongTypeID result;
            result.m_value = value;
            return result;
        }

        static StrongTypeID Null() noexcept
        {
            StrongTypeID result;
            result.m_value = NullValue;
            return result;
        }

        _Underlying toUnderlying() const noexcept
        {
            return m_value;
        }

        bool isValid() const noexcept
        {
            return m_value != NullValue;
        }

        explicit operator bool() const noexcept
        {
            return isValid();
        }

    private:
        _Underlying m_value = NullValue;
    };

}

#endif // TITANIUM_STRONG_TYPE_ID_H
