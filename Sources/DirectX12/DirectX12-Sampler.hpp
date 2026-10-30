#ifndef TITANIUM_DIRECTX12_SAMPLER_H
#define TITANIUM_DIRECTX12_SAMPLER_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-BaseSampler.hpp>
#include <DirectX12/DirectX12-Header.hpp>
#include <Private/DescriptorAllocator.hpp>

namespace TiRHI::DirectX12
{
    class RHI;
    class Device;

    class Sampler : public BaseSampler<Sampler, RHI>
    {
    public:
        Sampler() = delete;
        ~Sampler();
        Sampler(Sampler&&) noexcept;
        Sampler& operator=(Sampler&&) noexcept;
        explicit Sampler(RHI& rhi);

        bool build(Device& device);

    private:
        Device* m_device = nullptr;

        std::optional<CpuDescriptorHandle> m_samplerCpuHandle;
    };

}

#endif // TITANIUM_DIRECTX12_SAMPLER_H
