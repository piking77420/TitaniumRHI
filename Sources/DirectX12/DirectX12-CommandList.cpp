#include <DirectX12-CommandList.hpp>

#include <format>

#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Device.hpp>

namespace TiRHI::DirectX12
{
    CommandList::CommandList(RHI& rhi)
        : BaseCommandList(rhi)
    {
    }

    bool CommandList::build(Device& device)
    {
        m_allocators.resize(getRHI().getFrameInFlight());

        for (uint32_t i = 0; i < m_allocators.size(); ++i)
        {
            auto& cmdAlloc = m_allocators[i];

            const HRESULT hrCmdAllocCreated = device.getNativeDevice()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
            if (FAILED(hrCmdAllocCreated))
            {
                RHI_LOG_ERROR(
                    std::format(L"Create Command Allocator {} failed!\n Error Code: {}", i, hrCmdAllocCreated),
                    RhiApi::DirectX12);
                return false;
            }
            else
            {
                const std::wstring name =
                    std::format(L"CommandAlloc {} [{}]", (getName().empty() ? L"" : getNameW()), i);
                cmdAlloc->SetName(name.c_str());

                RHI_LOG_INFO(std::format(L"Create Command Allocator {} success! name : {}", hrCmdAllocCreated, name),
                             RhiApi::DirectX12);
            }
        }

        const HRESULT hrCmdListCreated = device.getNativeDevice()->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));
        if (FAILED(hrCmdListCreated))
        {
            RHI_LOG_ERROR(std::format(L"Create Command List failed! {} failed! ", hrCmdListCreated), RhiApi::DirectX12);
            return false;
        }
        else
        {
            const LPCWSTR name = getName().empty() ? L"" : getNameW().c_str();
            m_commandList->SetName(name);
            RHI_LOG_INFO(std::format(L"Create Command List success.\nName : {}, Handle : {}", name,
                                     reinterpret_cast<void*>(m_commandList.Get())),
                         RhiApi::DirectX12);
        }

        m_commandList->Close();

        return true;
    }

    bool CommandList::beginRecord()
    {
        if (!onBeginRecord())
            return false;

        const size_t currentFrame = getRHI().getCurrentFrame();
        m_commandList->Reset(m_allocators[currentFrame].Get(), nullptr);

        return true;
    }

    void CommandList::endRecord()
    {
        onEndRecord();
        m_commandList->Close();
    }

}
// namespace TiRHI::DirectX12
