#ifndef TITANIUM_VULKAN_SAMPLER_H
#define TITANIUM_VULKAN_SAMPLER_H

#include <Titanium/RHI-BaseSampler.hpp>
#include <Vulkan/Vulkan-Header.hpp>
#include <Vulkan/Private/FrameTracked.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class Sampler : public TiRHI::BaseSampler<Sampler, Device>, public Private::FrameTracked<Sampler>
    {
    public:
        Sampler() = delete;
        ~Sampler();
        Sampler(Sampler&&) = default;
        Sampler& operator=(Sampler&&) = default;
        Sampler(Device& device)
            : BaseSampler(device)
        {
        }

        struct Private
        {
            static vk::Sampler getSampler(Sampler& sampler)
            {
                return sampler.m_sampler.get();
            }
        };

        bool build();

    private:
        friend Private;

        Device* m_device{nullptr};

        vk::UniqueSampler m_sampler;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_SAMPLER_H
