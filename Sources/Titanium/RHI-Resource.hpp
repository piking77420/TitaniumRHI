#ifndef TITANIUM_RESOURCE_H
#define TITANIUM_RESOURCE_H

#include <Titanium/RHI-EnumToString.hpp>
#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class Resource : public Object<T, TRHI>
    {
    public:
        Resource() = default;
        ~Resource() = default;
        RHI_MOVE_ONLY(Resource)
        Resource(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, Resource<T, TRHI>>);
        }

        ResourceState getState() const
        {
            return m_state;
        }

        Resource& setState(ResourceState newState) const noexcept
        {
            m_state = newState;

            return reinterpret_cast<T&>(*this);
        }

    private:
        ResourceState m_state = ResourceState::Undefined;
    };

} // TiRHI

#endif // TITANIUM_RESOURCE_H
