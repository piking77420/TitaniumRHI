#include <Titanium/RHI-DebugScope.hpp>

namespace TiRHI
{
    DebugScope::DebugScope(CommandList& commandList, std::string_view name, std::span<const float, 4> color)
        : m_commandList(commandList)
    {
        m_commandList.beginDebugLabel(name, color);
    }

    DebugScope::DebugScope(CommandList& commandList, std::string_view name)
        : m_commandList(commandList)
    {
        m_commandList.beginDebugLabel(name);
    }

    DebugScope::~DebugScope()
    {
        m_commandList.endDebugLabel();
    }
} // amespace TiRHI
