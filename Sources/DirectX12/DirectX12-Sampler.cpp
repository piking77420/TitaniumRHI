#include <DirectX12/DirectX12-Sampler.hpp>
#include <DirectX12/DirectX12-Device.hpp>

namespace TiRHI::DirectX12
{
    Sampler::~Sampler() = default;

    Sampler::Sampler(RHI& rhi)
        : BaseSampler(rhi)
    {
    }

    bool Sampler::build(Device& device)
    {
        if (device.getNativeDevice())
            return false;

        m_samplerCpuHandle = Device::Private::getDescriptorAllocatorSampler(device).create(device);

        D3D12_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 16;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;

        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

        device.getNativeDevice()->CreateSampler(&samplerDesc, m_samplerCpuHandle->nativeHandle);

        return true;
    }

} // namespace TiRHI::DirectX12
