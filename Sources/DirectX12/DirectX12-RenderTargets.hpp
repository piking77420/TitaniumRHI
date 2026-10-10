#ifndef TITANIUM_DIRECTX12_RENDER_TARGETS_H
#define TITANIUM_DIRECTX12_RENDER_TARGETS_H

#include <span>
#include <vector>
#include <Titanium/RHI-BaseRenderTargets.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-RenderPassDescriptor.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class RenderTargets : public BaseRenderTargets<RenderTargets, RHI, RenderPassDescriptor>
    {
    public:
        RenderTargets() = delete;
        ~RenderTargets() = default;
        RHI_MOVE_CONSTRUCT_ONLY(RenderTargets)
        RenderTargets(RHI& rhi);

        struct Private
        {
            static bool build(RenderTargets& renderTargets, Device& device, D3D12_CPU_DESCRIPTOR_HANDLE handle,
                              ID3D12Resource* image)
            {
                return renderTargets.build(device, handle, image);
            }
        };

        struct Attachement
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle;
            ID3D12Resource* image;
        };

        bool build(Device& device);

        std::span<const Attachement> getAttachements() const
        {
            return m_attachments;
        }

    private:
        friend Private;

        std::vector<Attachement> m_attachments;

        bool build(Device& device, D3D12_CPU_DESCRIPTOR_HANDLE handle, ID3D12Resource* image);
    };

} // namespace TiRHI::DirectX12

#endif // TITANIUM_DIRECTX12_RENDER_TARGETS_H
