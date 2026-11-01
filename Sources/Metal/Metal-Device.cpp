#include <Metal/Metal-Device.hpp>
#include <Metal/Metal-RHI.hpp>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <Titanium/Log.hpp>
#include <Metal/Metal-CommandList.hpp>

namespace TiRHI::Metal
{
    Device::~Device() = default;

    Device::Device(RHI& rhi)
        : BaseDevice(rhi)
    {
    }

    void Device::beginFrame()
    {
        const size_t currentFrame = getRHI().getCurrentFrame();
        auto& commandLists = m_frame.commandList[currentFrame];
        waits(commandLists);
        commandLists.clear();
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
        m_frame.commandList.resize(rhi.getFrameInFlight());
        return true;
    }

    void Device::wait()
    {
        for (std::vector<CommandList*> frameCommandList : m_frame.commandList)
        {
            waits(frameCommandList);
            frameCommandList.clear();
        }
    }

    void Device::submit(std::span<const AcquiredFrame> acquiredFrames, std::span<CommandList*> cmdLists)
    {
        const size_t currentFrame = getRHI().getCurrentFrame();
        auto& commandLists = m_frame.commandList[currentFrame];

        commandLists.clear();
        commandLists.insert(commandLists.end(), cmdLists.begin(), cmdLists.end());
    }

    bool Device::isValid()
    {
        return m_device;
    }

    void Device::waits(std::span<CommandList*> commandLists)
    {
        for (CommandList* commandList : commandLists)
        {
            if (commandList == nullptr || (!CommandList::Private::getNativeHandle(*commandList)))
                continue;

            CommandList::Private::getNativeHandle(*commandList)->waitUntilCompleted();
        }
    }

} // TiRHI::Metal