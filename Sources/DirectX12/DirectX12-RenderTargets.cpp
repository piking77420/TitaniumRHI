#include <DirectX12/DirectX12-RenderTargets.hpp>

#include <DirectX12/DirectX12-Device.hpp>

namespace TiRHI::DirectX12
{
    RenderTargets::RenderTargets(RHI& rhi)
        : BaseRenderTargets(rhi)
    {
    }

    bool RenderTargets::build(Device& device)
    {
        m_attachments.clear();
        return true;
    }

    bool RenderTargets::build(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE handle, ID3D12Resource* image)
    {
        m_attachments.clear();

        device.getNativeDevice()->CreateRenderTargetView(image, nullptr, handle);
        auto& att = m_attachments.emplace_back();
        att.handle = handle;
        att.image = image;

        return true;
    }

} // TiRHI::DirectX12
