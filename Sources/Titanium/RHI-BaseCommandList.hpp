#ifndef TITANIUM_COMMAND_LIST_H
#define TITANIUM_COMMAND_LIST_H

#include <Titanium/RHI-Object.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI>
    class BaseCommandList : public Object<T, TRHI>
    {
    public:
        BaseCommandList() = delete;
        ~BaseCommandList() = default;
        BaseCommandList(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
        }

    private:
    };
} // namespace TiRHI

#endif // TITANIUM_COMMAND_LIST_H
