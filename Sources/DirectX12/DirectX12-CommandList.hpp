#ifndef TITANIUM_DIRECTX12_COMMAND_LIST_H
#define TITANIUM_DIRECTX12_COMMAND_LIST_H

#include <Titanium/RHI-BaseCommandList.hpp>

namespace TiRHI::DirectX12
{
    class RHI;

    class CommandList : public BaseCommandList<CommandList, RHI>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        CommandList(RHI& rhi);

    private:
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_COMMAND_LIST_H
