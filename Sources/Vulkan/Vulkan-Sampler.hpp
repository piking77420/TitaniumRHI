#ifndef TITANIUM_VULKAN_SAMPLER_H
#define TITANIUM_VULKAN_SAMPLER_H

#include <Titanium/RHI-BaseSampler.hpp>
#include <Vulkan/Vulkan-Header.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Sampler : public TiRHI::BaseSampler<Sampler, RHI>
    {
    public:
        Sampler() = delete;
        ~Sampler();
        Sampler(Sampler&&) = default;
        Sampler& operator=(Sampler&&) = default;
        Sampler(RHI& rhi)
            : BaseSampler(rhi)
        {
        }

        struct Private
        {
            static vk::Sampler getSampler(Sampler& sampler)
            {
                return sampler.m_sampler.get();
            }
        };

        bool build(Device& device);

    private:
        friend Private;

        vk::UniqueSampler m_sampler;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SAMPLER_H
