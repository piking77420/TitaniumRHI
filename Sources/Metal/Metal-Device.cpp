#include <Metal/Metal-Device.hpp>
#include <Metal/Metal-RHI.hpp>

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