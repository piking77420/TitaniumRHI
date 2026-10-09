#ifndef TITANIUM_VULKAN_COMMAND_LIST_H
#define TITANIUM_VULKAN_COMMAND_LIST_H

#include <optional>
#include <Volk/volk.h>
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
        RHI_MOVE_CONSTRUCT_ONLY(CommandList)
        explicit CommandList(RHI& rhi);

        bool build(Device& device);

        bool beginRecord();

        bool endRecord();

        void beginDebugLabel(std::string_view name, std::optional<std::span<const float, 4>> color = {});

        void endDebugLabel();

        vk::CommandBuffer getcurrentFrameCmb();

    private:
        vk::UniqueCommandPool m_commandPool;

        std::vector<vk::UniqueCommandBuffer> m_commandBuffer;

        const VolkDeviceTable* m_volkTable = nullptr;
    };

} // namespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_COMMAND_LIST_H
