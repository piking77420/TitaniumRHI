#ifndef TITANIUM_VULKAN_INSTANCE_H
#define TITANIUM_VULKAN_INSTANCE_H

#include <set>
#include <string>

#include <vulkan/vulkan.hpp>

#include <Titanium/RHITypes.hpp>

#include <Vulkan-Functions.hpp>

namespace TiRHI::Vulkan
{
    class Instance
    {
    public:
        Instance(const RhiCreate& rhiCreate);
        ~Instance();

        vk::Instance getInstance() noexcept
        {
            return m_instance;
        }

    private:
        vk::Instance m_instance;

        VulkanFunctions m_vulkanFunctions;

        vk::DebugUtilsMessengerEXT m_debugUtilsMessenger;

        void queryVulkanFunctions();
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_INSTANCE_H
