#ifndef TITANIUM_METAL_RHI_H
#define TITANIUM_METAL_RHI_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-BaseRHI.hpp>
#include <Metal/Metal-Header.hpp>

namespace CA
{
    class MetalLayer;
} // namespace CA

namespace TiRHI::Metal
{
    class RHI : public BaseRHI<RHI>
    {
    public:
        RHI() = delete;
        ~RHI();
        RHI(const RhiCreate& rhiCreate);

        struct Private
        {
            NS::AutoreleasePool* getAutoReleasePool(RHI& rhi)
            {
                return rhi.m_pool;
            }
        };

        [[nodiscard]] Device newDevice();

        [[nodiscard]] RhiApi getApi() const;

    private:
        friend Private;

        NS::AutoreleasePool* m_pool;

    };
} // TiRHI::Metal

#endif // TITANIUM_METAL_RHI_H
