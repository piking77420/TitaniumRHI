#include <DirectX12-RHI.hpp>

#include <format>

#include <Titanium/Log.hpp>

namespace TiRHI
{
    DirectX12RHI::DirectX12RHI(const RhiCreate& rhiCreate)
        : RHI<DirectX12RHI>(rhiCreate)
        , m_factory()
        , m_adaptater(m_factory.getFactory())
    {
        // Set up queue
        {
            // GFX
            {
                const D3D12_COMMAND_QUEUE_DESC desc{
                    .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                };

                /**
                 * DX12 can create queue 'on the fly' after device creation.
                 * No need to specify in advance how many queues will be used by the device object.
                 */
                const HRESULT hrGFXCmdQueueCreated =
                    m_adaptater.getDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_graphicsQueue));
                if (FAILED(hrGFXCmdQueueCreated))
                {
                    RHI_LOG_ERROR(std::format(L"Create Graphics Queue failed!\nError Code: 0x{:08X}",
                                              static_cast<unsigned long>(hrGFXCmdQueueCreated)),
                                  RhiApi::DirectX12, RhiMessageSeverity::Error);

                    return;
                }
                else
                {
                    const LPCWSTR name = L"GraphicsQueue";
                    m_graphicsQueue->SetName(name);
                    {
                        RHI_LOG_INFO(std::format(L"Create Graphics Queue success. Name: {} Address: {}", name,
                                                 static_cast<void*>(m_graphicsQueue.Get())),
                                     RhiApi::DirectX12);
                    }
                }

                // Sync
                {
                    m_synchronisation.deviceFenceEvent = CreateEvent(nullptr, false, false, nullptr);
                    if (!m_synchronisation.deviceFenceEvent)
                    {
                        RHI_LOG_ERROR(L"Create Device Fence Event failed!", RhiApi::DirectX12);
                        return;
                    }
                    else
                    {
                        RHI_LOG_INFO(L"Create Device Fence Event success.", RhiApi::DirectX12);
                    }

                    const HRESULT hrDeviceFenceCreated = m_adaptater.getDevice()->CreateFence(
                        0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_synchronisation.deviceFence));
                    if (FAILED(hrDeviceFenceCreated))
                    {
                        RHI_LOG_ERROR(
                            std::format(L"Create Device Fence failed! \n Error Code: {}", hrDeviceFenceCreated),
                            RhiApi::DirectX12);

                        return;
                    }
                    else
                    {
                        const LPCWSTR name = L"DeviceFence";
                        m_synchronisation.deviceFence->SetName(name);
                        RHI_LOG_INFO(
                            std::format(L"Create Swapchain Fence success. [{}] [{}]", name, hrDeviceFenceCreated),
                            RhiApi::DirectX12);
                    }
                }
            }
        }
    }

    void DirectX12RHI::waitImpl()
    {
    }

} // namespace TiRHI
