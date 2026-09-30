#include <DirectX12-RHI.hpp>

#include <format>

#include <Titanium/Log.hpp>

namespace TiRHI
{
    DirectX12RHI::DirectX12RHI(const RhiCreate& rhiCreate)
        : RHI<DirectX12RHI>(rhiCreate)
        , m_factory()
        , m_device(m_factory.getFactory(), m_adapters)
    {
    }

    void DirectX12RHI::waitImpl()
    {
    }

    bool DirectX12RHI::createDeviceImpl()
    {
        m_device.createDevice(m_device.getSelectPhyscialDeviceIndex(), m_factory.getFactory());
        return m_device.getDevice() != nullptr;
    }

    bool DirectX12RHI::createDeviceImpl(size_t adapterIndex)
    {
        m_device.createDevice(adapterIndex, m_factory.getFactory());
        return m_device.getDevice() != nullptr;
    }

    const Adapter* DirectX12RHI::getUsedAdapterImpl() const
    {
        const size_t index = m_device.getSelectPhyscialDeviceIndex();

#undef max // :)
        if (index == std::numeric_limits<size_t>::max() || index >= m_adapters.size())
            return nullptr;

        return &m_adapters[index];
    }

} // namespace TiRHI
