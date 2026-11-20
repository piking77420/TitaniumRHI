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
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        Sampler() = delete;
        ~Sampler();
        Sampler(Sampler&&) noexcept = default;
        Sampler& operator=(Sampler&&) noexcept = default;
        explicit Sampler(RHI& rhi);

        bool build(Device& device);

        const std::optional<TiRHI::DirectX12::Private::DescriptorHandle>& getShaderViewHandle() const
        {
            return m_samplerCpuHandle;
        }

        void setSubmissionValue(uint64_t value)
        {
            m_lastSubmitedValue = value;
        }

    private:
        Device* m_device{nullptr};

        uint64_t m_lastSubmitedValue = 0;

        std::optional<TiRHI::DirectX12::Private::DescriptorHandle> m_samplerCpuHandle;
    };

}

#endif // TITANIUM_DIRECTX12_SAMPLER_H
