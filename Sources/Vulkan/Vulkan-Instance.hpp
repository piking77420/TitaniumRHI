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
        static constexpr TiRHI::RhiApi Api = RhiApi::Vulkan;

        Instance() = delete;
        ~Instance();
        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;
        Instance(Instance&&) noexcept;
        Instance& operator=(Instance&&) noexcept;
        Instance(const RHI& rhi);

        vk::Instance getInstance() const noexcept
        {
            return m_instance;
        }

        uint32_t getApiVersion() const noexcept
        {
            return m_apiVersion;
        }

    private:
        void destroy();

        vk::Instance m_instance;

        vk::DebugUtilsMessengerEXT m_debugUtilsMessenger;

        uint32_t m_apiVersion = 0;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_INSTANCE_H
