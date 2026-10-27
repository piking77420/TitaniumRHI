#include <Metal/Metal-Device.hpp>
#include <Metal/Metal-RHI.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Metal
{
    Device::~Device() = default;

    Device::Device(RHI& rhi)
        : BaseDevice(rhi)
    {
    }

    void Device::beginFrame()
    {
    }

    bool Device::build(RHI& rhi, Surface& surface, const std::span<const Adapter>& adapters,
                       std::optional<size_t> index)
    {
        onBuild(adapters, index);
        RHI_LOG_INFO(L"Build device", RhiApi::Metal);
        NS::Array* devices = MTL::CopyAllDevices();

        if (m_adapterIndex >= devices->count())
        {
            devices->release();
            return false;
        }
        
        MTL::Device* selectedDevice = devices->object<MTL::Device>(m_adapterIndex);
        if (!selectedDevice)
        {
            devices->release();
            return false;
        }
        m_device = selectedDevice;
        m_device->retain();
        devices->release();
        onCreateDeviceSuccess(adapters[m_adapterIndex], m_device);
        return true;
    }

    void Device::wait()
    {
        for (auto& commandList : m_pendingCommandList)
        {
            if (commandList == nullptr)
                continue;
            //
            // TODO
            // commandList->waitUntilCompleted();
        }
    }

    void Device::submit(std::span<const AcquiredFrame> acquiredFrames, std::span<CommandList*> cmdLists)
    {
        m_pendingCommandList.clear();
        m_pendingCommandList.insert(m_pendingCommandList.end(), cmdLists.begin(), cmdLists.end());
    }

    bool Device::isValid()
    {
        return m_device;
    }

} // TiRHI::Metal