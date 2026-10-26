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
            CA::MetalLayer* getLayer(RHI& rhi)
            {
                return rhi.m_layer;
            }
        };

        [[nodiscard]] Device newDevice();

        [[nodiscard]] RhiApi getApi() const;

    private:
        friend Private;

        MTL::Device* m_device = nullptr;

        CA::MetalLayer* m_layer = nullptr;
    };
} // TiRHI::Metal

#endif // TITANIUM_METAL_RHI_H
