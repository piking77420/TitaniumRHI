#ifndef TITANIUM_RHI_BASE_RHI_H
#define TITANIUM_RHI_BASE_RHI_H

#include <concepts>
#include <span>

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Adapter.hpp>

namespace TiRHI
{
    struct RHIGlobalState
    {
        static inline LogCallBackSignature logCallBack{nullptr};
    };

    template<typename Derived>
    class BaseRHI : public RHIGlobalState
    {
    public:
        using _Derived = Derived;

        BaseRHI() = delete;
        ~BaseRHI() = default;

        BaseRHI(BaseRHI&& otherBaseRHI) noexcept = default;
        BaseRHI(const BaseRHI& otherBaseRHI) = default;

        BaseRHI& operator=(BaseRHI&& otherBaseRHI) noexcept = default;
        BaseRHI& operator=(const BaseRHI& otherBaseRHI) = default;

        BaseRHI(const RhiCreate& rhiCreate);

        const std::span<const Adapter> getAdapters() const noexcept
        {
            return std::span<const Adapter>(m_adapters);
        }

    protected:
        std::vector<Adapter> m_adapters;
    };

    template<typename Derived>
    inline BaseRHI<Derived>::BaseRHI(const RhiCreate& rhiCreate)
    {
        static_assert(std::derived_from<Derived, BaseRHI<Derived>>, "Derived must inherit from RHI<Derived>");

        logCallBack = rhiCreate.logCallback;
    }

}

#endif // TITANIUM_RHI_BASE_RHI_H
