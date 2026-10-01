#ifndef TITANIUM_BASE_DEVICE_H
#define TITANIUM_BASE_DEVICE_H

#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseRHI.hpp>

namespace TiRHI
{
    class Adapter;

    class BaseDevice
    {
    public:
        BaseDevice() = default;
        ~BaseDevice() = default;

        template<typename T>
        const Adapter& getSourceAdapter(const BaseRHI<T>& rhi) const
        {
            return rhi.getAdapters()[m_adapterIndex];
        }

    protected:
        size_t m_adapterIndex = 0;

        static int64_t getAdapterScore(const Adapter& adapter);

        static size_t getBestAdapter(const std::span<const Adapter>& adapters);
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_DEVICE_H
