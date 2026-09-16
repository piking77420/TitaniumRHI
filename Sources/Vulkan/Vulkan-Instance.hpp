#ifndef TITANIUM_VULKAN_INSTANCE_H
#define TITANIUM_VULKAN_INSTANCE_H

#include <set>
#include <string>

#include <vulkan/vulkan.hpp>
#include <Vulkan-Functions.hpp>

namespace TiRHI::Vulkan
{
    class Instance
    {
    public:
        Instance();
        ~Instance();

        vk::Instance getInstance() noexcept
        {
            return m_instance;
        }

    private:
        vk::Instance m_instance;

        VulkanFunctions m_vulkanFunctions;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_INSTANCE_H
