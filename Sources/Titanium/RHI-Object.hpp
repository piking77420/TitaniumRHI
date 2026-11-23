#ifndef TITANIUM_OBJECT_H
#define TITANIUM_OBJECT_H

#include <concepts>
#include <string>
#include <cassert>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    // TODO may add and submited value in order to deffered destory

    template<typename T, typename TDevice>
    class Object
    {
    public:
        using _Device = TDevice;

        static constexpr TiRHI::RhiApi DerivedRhiApi = T::Api;

        Object() = delete;
        ~Object() = default;
        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        Object(Object&&) noexcept = default;
        Object& operator=(Object&& other) noexcept
        {
            assert(&getDevice() == &other.getDevice());
            if (this != &other)
                m_name = std::move(other.m_name);
            return *this;
        }

        explicit Object(_Device& rhi)
            : m_device(rhi)
        {
            static_assert(std::derived_from<T, Object<T, _Device>>);
        }

        _Device& getDevice()
        {
            return m_device;
        };

        const _Device& getDevice() const
        {
            return m_device;
        };

        auto& getRHI()
        {
            return m_device.getRHI();
        }

        const auto& getRHI() const
        {
            return m_device.getRHI();
        }

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
            m_name = std::move(newName);
            return static_cast<T&>(*this);
        }

        T& setName(const std::string& newName)
        {
            m_name = newName;
            return static_cast<T&>(*this);
        }

        T& setName(const std::wstring& newName)
        {
            if (!newName.empty())
            {
                m_name = std::string(newName.begin(), newName.end());
            }
            return static_cast<T&>(*this);
        }

    private:
        _Device& m_device;

        std::string m_name;
    };

} // namespace TiRHI

#endif
