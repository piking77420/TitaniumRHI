#include <DirectX12/DirectX12-Sampler.hpp>
#include <DirectX12/DirectX12-Device.hpp>

namespace TiRHI::DirectX12
{
    Sampler::~Sampler()
    {
        if (!m_device)
            return;

        auto& defferdDestroy = Device::Private::getDeferredDeletionQueue(*m_device);
        defferdDestroy.retire(m_lastSubmitedValue,
                              [device = m_device, sampler = std::move(m_samplerCpuHandle),
                               cpuHandle = std::move(m_samplerCpuHandle)]() mutable
                              {
                                  Device::Private::getDescriptorAllocatorRTV(*device).release(*cpuHandle);
                                  sampler.reset();
                              });

        m_lastSubmitedValue = 0;
    }

    Sampler::Sampler(RHI& rhi)
        : BaseSampler(rhi)
    {
    }

    bool Sampler::build(Device& device)
    {
        if (!device.getNativeDevice())
            return false;

        m_samplerCpuHandle = Device::Private::getDescriptorAllocatorSampler(device).create(device);

        if (!m_samplerCpuHandle || !*m_samplerCpuHandle)
            return false;

        D3D12_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 16;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

        device.getNativeDevice()->CreateSampler(&samplerDesc, m_samplerCpuHandle->cpuHandle);

        return true;
    }

} // namespace TiRHI::DirectX12
