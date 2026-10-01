#include <Titanium/RHI-Adapter.hpp>

namespace TiRHI
{
    Adapter::Adapter(const std::string_view& name, const std::vector<Features>& features, const Properties& properties,
                     uint32_t vendorId)
        : m_name(name)
        , m_vendor(vendorName(vendorId))
        , m_features(features)
        , m_properties(properties)
    {
    }
} // namespace TiRHI
