#include <Metal/Metal-RHI.hpp>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Metal/Metal.hpp>
#include <MetalFX/MetalFX.hpp>

namespace TiRHI::Metal
{
    RHI::~RHI()
    {
        if (m_device)
            m_device->release();
    }

    RHI::RHI(const RhiCreate& rhiCreate)
        : BaseRHI<RHI>(rhiCreate)
        , m_device(MTL::CreateSystemDefaultDevice())
    {
        if (!m_device)
            return;

        const std::string adapterName = m_device->name()->utf8String();

        std::vector<Adapter::Features> features;

        // Metal ray tracing (ray queries / intersectors)
        if (m_device->supportsRaytracing())
            features.push_back(Adapter::Features::RayQuery);

        // Metal object + mesh shaders
        if (m_device->supportsFamily(MTL::GPUFamilyMetal3))
            features.push_back(Adapter::Features::MeshShader);

        // Metal has no direct equivalent to Vulkan's
        // VK_KHR_ray_tracing_pipeline.

        Adapter::Properties property{};

        // Metal does not directly expose a PCI vendor ID.
        uint32_t vendorId = 0;

        // Apple GPU
        if (m_device->supportsFamily(MTL::GPUFamilyApple7))
            vendorId = 0x106B;

        m_adapters.emplace_back(adapterName, features, property, vendorId);
    }

} // TiRHI::Metal
