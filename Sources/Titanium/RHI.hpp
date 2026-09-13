#ifndef TITANIUM_D3D12_DEVICE_H
#define TITANIUM_D3D12_DEVICE_H

#include <memory>

namespace TiRHI
{
    class RHI
    {
    public:
        RHI();
        ~RHI();

        void waitForDeviceIdle();

    private:
        struct Backend;
        std::unique_ptr<Backend> m_impl; // may found someting else try to avoid alloc
    };
}

#endif // TITANIUM_D3D12_DEVICE_H
