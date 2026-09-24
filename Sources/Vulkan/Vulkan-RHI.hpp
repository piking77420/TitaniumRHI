#ifndef TITANIUM_VULKAN_RHI_H
#define TITANIUM_VULKAN_RHI_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI.hpp>

#include <Vulkan-Instance.hpp>
#include <Vulkan-Device.hpp>

namespace TiRHI
{
    class VulkanRHI : public RHI<VulkanRHI>
    {
    public:
        VulkanRHI(const RhiCreate& rhiCreate);
        ~VulkanRHI() = default;

        void waitImpl();

    private:
        Vulkan::Instance m_instance;
        Vulkan::Device m_device;
    };
}

#endif // TITANIUM_VULKAN_RHI_H
