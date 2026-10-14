#ifndef TITANIUM_METAL_RHI_H
#define TITANIUM_METAL_RHI_H

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-BaseRHI.hpp>

#include <QuartzCore/CAMetalLayer.hpp>
#include <Metal/Metal.hpp>

namespace TiRHI::Metal
{
    class RHI : public BaseRHI<RHI>
    {
    public:
        RHI() = delete;
        ~RHI();
        RHI(const RhiCreate& rhiCreate);

    private:
        MTL::Device* m_device = nullptr;

        CA::MetalLayer* m_layer = nullptr;
    };
} // TiRHI::Metal

#endif // TITANIUM_METAL_RHI_H
