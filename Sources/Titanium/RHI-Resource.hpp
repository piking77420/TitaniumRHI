#ifndef TITANIUM_RESOURCE_H
#define TITANIUM_RESOURCE_H

#include <Titanium/RHI-EnumToString.hpp>
#include <Titanium/RHI-Object.hpp>

#define RESOURCE_STATE_LIST(X)                                                                                         \
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

namespace TiRHI
{
    template<typename T, typename TRHI>
    class Resource : public Object<T, TRHI>
    {
    public:
        struct enum State
        {
#define X(name) name,
            RESOURCE_STATE_LIST(X)
#undef X
        };

        IMPLEMENT_TO_STRING_TITANIUM(State, RESOURCE_STATE_LIST)

        Resource() = default;
        ~Resource() = default;
        Resource(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, Resource<T, TRHI>>);
        }

        T& defaultState(State state)
        {
            m_initaleState = state;
            return reinterpret_cast<T&>(*this);
        }

        bool create()
        {
            m_state = m_initaleState;
        }

    private:
        State m_initaleState = State::Undefined;
        State m_state = State::Undefined;
    };

} // TiRHI

#endif // TITANIUM_RESOURCE_H
