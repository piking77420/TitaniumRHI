#ifndef TITANIUM_DEBUG_SCOPE_H
#define TITANIUM_DEBUG_SCOPE_H

#include <span>
#include <string_view>
#include <Titanium/TitaniumHeader.hpp>

namespace TiRHI
{
    class DebugScope
    {
    public:
        DebugScope(CommandList& commandList, std::string_view name, std::span<const float, 4> color);
        DebugScope(CommandList& commandList, std::string_view name);
        ~DebugScope();

    private:
        CommandList& m_commandList;
    };

} // amespace TiRHI

#endif // TITANIUM_VULKAN_DEBUG_SCOPE_H
