#ifndef TITANIUM_DIRECTX12_COMMAND_LIST_H
#define TITANIUM_DIRECTX12_COMMAND_LIST_H

#include <vector>
#include <Titanium/RHI-BaseCommandList.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <d3d12.h>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class CommandList : public BaseCommandList<CommandList, RHI>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        ID3D12GraphicsCommandList1* getCommandListNative()
        {
            return m_commandList.Get();
        }

    private:
        std::vector<MComPtr<ID3D12CommandAllocator>> m_allocators;
        MComPtr<ID3D12GraphicsCommandList1> m_commandList;
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_COMMAND_LIST_H
