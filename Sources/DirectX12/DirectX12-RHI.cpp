#include <DirectX12-RHI.hpp>

#include <print>

namespace TiRHI
{
    RHI::RHI()
        : m_device(m_instance)
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
                    m_device.getDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_graphicsQueue));
                if (FAILED(hrGFXCmdQueueCreated))
                {
                    std::println("Create Graphics Queue failed! \n Error Code: {} \n", hrGFXCmdQueueCreated);
                    return;
                }
                else
                {
                    const LPCWSTR name = L"GraphicsQueue";
                    m_graphicsQueue->SetName(name);
                    std::println("Create Graphics Queue success. \n{} {}\n", std::string(name, name + lstrlenW(name)),
                                 static_cast<void*>(m_graphicsQueue.Get()));
                }
            }
        }

        // Sync
        {
            m_synchronisation.deviceFenceEvent = CreateEvent(nullptr, false, false, nullptr);
            if (!m_synchronisation.deviceFenceEvent)
            {
                std::println("Create Device Fence Event failed! \n");
                return;
            }
            else
            {
                std::println("Create Device Fence Event success. \n");
            }

            const HRESULT hrDeviceFenceCreated = m_device.getDevice()->CreateFence(
                0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_synchronisation.deviceFence));
            if (FAILED(hrDeviceFenceCreated))
            {
                std::println("Create Device Fence failed! \n Error Code: {} \n", hrDeviceFenceCreated);
                return;
            }
            else
            {
                const LPCWSTR name = L"DeviceFence";
                m_synchronisation.deviceFence->SetName(name);
                std::println("Create Swapchain Fence success.\n[{}] [{}] \n", std::string(name, name + lstrlenW(name)),
                             static_cast<void*>(m_synchronisation.deviceFence.Get()));
            }
        }
    }

    RHI::~RHI()
    {
    }

    void RHI::waitForDeviceIdle()
    {
    }
}
