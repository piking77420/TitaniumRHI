#ifndef TITANIUM_OBJECT_H
#define TITANIUM_OBJECT_H

#include <concepts>
#include <string>

namespace TiRHI
{
#define RHI_MOVE_CONSTRUCT_ONLY(Type)                                                                                  \
    Type(const Type&) = delete;                                                                                        \
    Type& operator=(const Type&) = delete;                                                                             \
    Type(Type&&) noexcept = default;                                                                                   \
    Type& operator=(Type&&) = delete;

    template<typename T, typename TRHI>
    class Object
    {
    public:
        Object() = delete;
        ~Object() = default;
        RHI_MOVE_CONSTRUCT_ONLY(Object)
        Object(TRHI& rhi)
            : m_rhi(rhi)
        {
            static_assert(std::is_nothrow_move_constructible_v<Object<T, TRHI>>);
        }

        TRHI& getRHI()
        {
            return m_rhi;
        };
        const TRHI& getRHI() const
        {
            return m_rhi;
        };

        std::string_view getName() const
        {
            return m_name;
        }

        std::wstring getNameW() const
        {
            return m_name.empty() ? std::wstring(L"") : std::wstring(m_name.begin(), m_name.end());
        }

        T& setName(std::string&& newName)
        {
            static_assert(std::derived_from<T, Object<T, TRHI>>);
            m_name = std::move(newName);
            return reinterpret_cast<T&>(*this);
        }

        T& setName(const std::string& newName)
        {
            static_assert(std::derived_from<T, Object<T, TRHI>>);
            m_name = newName;
            return reinterpret_cast<T&>(*this);
        }

        T& setName(const std::wstring& newName)
        {
            static_assert(std::derived_from<T, Object<T>>);
            if (!newName.empty())
            {
                m_name = std::string(newName.begin(), newName.end());
            }
            return reinterpret_cast<T&>(*this);
        }

    private:
        TRHI& m_rhi;

        std::string m_name;
    };

} // namespace TiRHI

#endif
