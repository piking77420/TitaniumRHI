#ifndef TITANIUM_DIRECTX12_INSTANCE_H
#define TITANIUM_DIRECTX12_INSTANCE_H

#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <dxgi1_4.h>

#include <DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class Instance
    {
    public:
        Instance();
        ~Instance();

        UINT getFactoryFlags() const
        {
            return m_dxgiFactoryFlags;
        }

        MComPtr<IDXGIFactory6>& getFactory()
        {
            return m_factory;
        }

    private:
        UINT m_dxgiFactoryFlags = 0;

        MComPtr<IDXGIFactory6> m_factory;

        MComPtr<IDXGIAdapter3> m_adapter;

        void setupValidationLayer();

        void createFactory();

        void createAdapter();
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_INSTANCE_H
