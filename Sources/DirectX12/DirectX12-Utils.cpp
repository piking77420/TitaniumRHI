#include <DirectX12-Utils.hpp>

namespace TiRHI::DirectX12::Internal
{
    std::vector<MComPtr<IDXGIAdapter1>> getAllNativeAdapters(IDXGIFactory6* factory)
    {
        std::vector<MComPtr<IDXGIAdapter1>> result;

        if (!factory)
            return result;

        for (UINT i = 0;; ++i)
        {
            MComPtr<IDXGIAdapter1> adapter1;

            if (factory->EnumAdapters1(i, &adapter1) == DXGI_ERROR_NOT_FOUND)
                break;
            result.push_back(adapter1);
        }

        return result;
    }

} // namespace TiRHI::DirectX12::Internal
