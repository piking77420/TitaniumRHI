#include <DirectX12-CommandList.hpp>

#include <format>

#if defined(TITANIUM_USE_PIX)
#include <pix3.h>
#endif // defined(TITANIUM_USE_PIX)

#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-RHI.hpp>
#include <DirectX12/DirectX12-Device.hpp>

static uint8_t toPixColor(float value)
{
    return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}

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

        const HRESULT hrAllocator = m_allocators[currentFrame]->Reset();

        if (FAILED(hrAllocator))
            return false;

        const HRESULT hrCommandList = m_commandList->Reset(m_allocators[currentFrame].Get(), nullptr);

        if (FAILED(hrCommandList))
            return false;

        return true;
    }

    bool CommandList::endRecord()
    {
        const HRESULT hr = m_commandList->Close();

        if (FAILED(hr))
        {
            RHI_LOG_ERROR(std::format(L"Close Command List failed: {}", hr), RhiApi::DirectX12);

            return false;
        }

        onEndRecord();

        return true;
    }

    void CommandList::beginDebugLabel(std::string_view name,
                                      [[maybe_unused]] std::optional<std::span<const float, 4>> color)
    {
        if (!getRHI().getUseDebugLabel())
            return;

#if defined(TITANIUM_USE_PIX)
        UINT64 colorPix = PIX_COLOR_DEFAULT;
        if (color)
        {
            const std::span<const float, 4> colors = *color;
            const auto toByte = [](float v) -> uint8_t
            { return static_cast<uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f); };

            colorPix = PIX_COLOR(toByte(colors[0]), toByte(colors[1]), toByte(colors[2]));
        }
        PIXBeginEvent(m_commandList.Get(), colorPix, name.data());
#endif
    }

    void CommandList::endDebugLabel()
    {
        if (!getRHI().getUseDebugLabel())
            return;

#if defined(TITANIUM_USE_PIX)
        PIXEndEvent(m_commandList.Get());
#endif // defined(TITANIUM_USE_PIX)
    }

}
// namespace TiRHI::DirectX12
