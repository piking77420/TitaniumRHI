#ifndef TITANIUM_VULKAN_INSTANCE_H
#define TITANIUM_VULKAN_INSTANCE_H

#include <set>
#include <string>

#include <vulkan/vulkan.hpp>

#include <Titanium/RHITypes.hpp>

namespace TiRHI::Vulkan
{
    class RHI;

    class Instance
    {
    public:
        Instance(const RHI& rhi);
        ~Instance();

        vk::Instance getInstance() noexcept
        {
            return m_instance;
        }

    private:
        vk::Instance m_instance;

        vk::DebugUtilsMessengerEXT m_debugUtilsMessenger;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_INSTANCE_H
