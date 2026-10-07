#include <Private/VulkanToRHI.hpp>
#include <Titanium/Log.hpp>
#include <Vulkan/vk_enum_string_helper.h>

namespace TiRHI::Vulkan::Private
{
    Format toRhi(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eR8G8B8A8Unorm:
            return Format::R8G8B8A8_UNorm;
        case vk::Format::eB8G8R8A8Unorm:
            return Format::B8G8R8A8_UNorm;
        default:
        {
            const std::string s = vk::to_string(format);
            const std::wstring ws{s.begin(), s.end()};
            RHI_LOG_ERROR(std::format(L"Unsuported toRhi format {}", ws), RhiApi::Vulkan);
        }
        }
        return Format::R8_UNorm;
    }

} // namespace TiRHI::Vulkan::Private
