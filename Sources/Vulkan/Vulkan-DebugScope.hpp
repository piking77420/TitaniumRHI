#ifndef TITANIUM_VULKAN_DEBUG_SCOPE_H
#define TITANIUM_VULKAN_DEBUG_SCOPE_H

#include <span>
#include <string_view>

namespace TiRHI::Vulkan
{
    class CommandList;

    class DebugScope
    {
    public:
        DebugScope(CommandList& commandList, std::string_view name, std::span<const float, 4> color);
        DebugScope(CommandList& commandList, std::string_view name);
        ~DebugScope();

    private:
        CommandList& m_commandList;
    };

} // amespace TiRHI::Vulkan

#endif // TITANIUM_VULKAN_DEBUG_SCOPE_H
