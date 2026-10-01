#ifndef TITANIUM_BASE_DEVICE_H
#define TITANIUM_BASE_DEVICE_H

#include <Titanium/RHI-Adapter.hpp>

namespace TiRHI
{
    class Adapter;

    class BaseDevice
    {
    public:
        BaseDevice() = default;
        ~BaseDevice() = default;

    protected:
        size_t m_adapterIndex = 0;

        int64_t getAdapterScore(const Adapter& adapter) const;
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_DEVICE_H
