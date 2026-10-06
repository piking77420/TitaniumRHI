#ifndef TITANIUM_VULKAN_COMMAND_LIST_H
#define TITANIUM_VULKAN_COMMAND_LIST_H

#include <Vulkan/vulkan.hpp>
#include <Titanium/RHI-BaseCommandList.hpp>

namespace TiRHI::Vulkan
{
    class RHI;
    class Device;

    class CommandList : public BaseCommandList<CommandList, RHI>
    {
    public:
        CommandList() = delete;
        ~CommandList() = default;
        explicit CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        vk::CommandBuffer getcurrentFrameCmb();

    private:
        vk::UniqueCommandPool m_commandPool;

        std::vector<vk::UniqueCommandBuffer> m_commandBuffer;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_COMMAND_LIST_H
