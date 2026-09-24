#include <Titanium/RHI-Adapter.hpp>

namespace TiRHI
{
    Adapter::Adapter(const std::string& name, const std::vector<Features>& features, uint64_t m_vramMemoryGb,
                     uint32_t vendorId)
        : m_name(name)
        , m_vendor(vendorName(vendorId))
        , m_features(features)
        , m_vramMemoryGb(m_vramMemoryGb)
    {
    }
} // namespace TiRHI
