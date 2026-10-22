#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>

#include <Metal/Metal-RHI.hpp>


namespace TiRHI::Metal
{
    RHI::~RHI()
    {
    }

    RHI::RHI(const RhiCreate& rhiCreate)
        : BaseRHI<RHI>(rhiCreate)
        , m_device(MTL::CreateSystemDefaultDevice())
    {

        // Enumerate all adapter / device
        NS::Array* devices = MTL::CopyAllDevices();

        for (NS::UInteger i = 0; i < devices->count(); ++i)
        {
            MTL::Device* device = devices->object<MTL::Device>(i);
            if (!device)
                continue;

            std::vector<Adapter::Features> features;

            // Metal ray tracing (ray queries / intersectors)
            if (device->supportsRaytracing())
                features.push_back(Adapter::Features::RayQuery);

            // Metal object + mesh shaders
            if (device->supportsFamily(MTL::GPUFamilyMetal3))
                features.push_back(Adapter::Features::MeshShader);

            // Metal has no direct equivalent to Vulkan's
            // VK_KHR_ray_tracing_pipeline.

            Adapter::Properties property{};

            // Metal does not directly expose a PCI vendor ID.
            uint32_t vendorId = 0;
            // Apple GPU
            if (m_device->supportsFamily(MTL::GPUFamilyApple7))
                vendorId = 0x106B;

            property.limits.minUniformBufferOffset = 256;
            property.memoryLimits.vramMemoryBytes = device->recommendedMaxWorkingSetSize();
            property.deviceType = device->isLowPower() ? Adapter::Properties::Type::IntegratedGpu
                                                       : Adapter::Properties::Type::DiscreteGpu;

            m_adapters.emplace_back(Adapter(device->name()->cString(NS::StringEncoding::ASCIIStringEncoding), features,
                                            property, vendorId));
        }

        devices->release();
    }

    Device RHI::newDevice()
    {
        return Device(*this);
    }

    [[nodiscard]] RhiApi RHI::getApi() const
    {
        return RhiApi::Metal;
    }

} // TiRHI::Metal
