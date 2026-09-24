#ifndef TITANIUM_DIRECTX12_FACTORY_H
#define TITANIUM_DIRECTX12_FACTORY_H

#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <dxgi1_4.h>

#include <Titanium/RHITypes.hpp>

#include <DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class Factory
    {
    public:
        Factory();
        ~Factory();

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

        void setupValidationLayer();

        void createFactory();
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_FACTORY_H
