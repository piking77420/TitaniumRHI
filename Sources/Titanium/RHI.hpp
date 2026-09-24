#ifndef TITANIUM_RHI_H
#define TITANIUM_RHI_H

#include <concepts>

#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    struct RHIGlobalState
    {
        static inline LogCallBackSignature logCallBack{nullptr};
    };

    template<typename Derived>
    class RHI : public RHIGlobalState
    {
    public:
        RHI() = delete;
        ~RHI() = default;

        RHI(RHI&& otherRHI) noexcept = default;
        RHI(const RHI& otherRHI) = default;

        RHI& operator=(RHI&& otherRHI) noexcept = default;
        RHI& operator=(const RHI& otherRHI) = default;

        RHI(const RhiCreate& rhiCreate);

        void wait();

    private:
    };

    template<typename Derived>
    inline RHI<Derived>::RHI(const RhiCreate& rhiCreate)
    {
        static_assert(std::derived_from<Derived, RHI<Derived>>, "Derived must inherit from RHI<Derived>");

        logCallBack = rhiCreate.logCallback;
    }

    template<typename Derived>
    inline void RHI<Derived>::wait()
    {
        static_cast<Derived&>(*this).waitImpl();
    }

}

#endif // TITANIUM_RHI_H
