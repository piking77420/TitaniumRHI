#ifndef TITANIUM_DIRECTX12_SURFACE_H
#define TITANIUM_DIRECTX12_SURFACE_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Surface.hpp>
#include <DirectX12/DirectX12-Header.hpp>

namespace TiRHI::DirectX12
{
    class RHI;

    class Surface : public BaseSurface<Surface, RHI>
    {
    public:
        Surface() = delete;
        ~Surface() = default;
        explicit Surface(RHI& rhi);

        bool build(WindowHandle handle);

        HANDLE getWindowHandle() const
        {
            return m_windowHandle;
        }

    private:
        HANDLE m_windowHandle{nullptr};
    };

}

#endif // TITANIUM_DIRECTX12_SURFACE_H
