#include <Metal/Metal-GraphicPipeline.hpp>
#include <Metal/Metal-Device.hpp>
#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#include <Metal/Metal-RenderPassDescriptor.hpp>
#include <Private/RhiToMetal.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI::Metal
{
    struct MetalShaderModule
    {
        MTL::Library* library = nullptr;
        MTL::Function* function = nullptr;
    };

    MetalShaderModule createShaderModule(
        MTL::Device* device,
        const std::span<const uint8_t>& code,
        const std::string& entryPoint)
    {
        NS::Error* error = nullptr;

        if (code.empty())
        {
            RHI_LOG_ERROR(L"Metal shader source is empty", RhiApi::Metal);
            return {};
        }

        const std::string source(reinterpret_cast<const char*>(code.data()), code.size());
        MTL::Library* library = device->newLibrary(
            NS::String::string(source.c_str(), NS::UTF8StringEncoding),
            nullptr,
            &error
        );

        if (!library)
        {
            std::wstring message = L"Failed to create Metal shader library";
            if (error)
            {
                NS::String* description = error->localizedDescription();
                if (description)
                {
                    message += L": ";
                    for (NS::UInteger i = 0; i < description->length(); ++i)
                    {
                        uint32_t character = description->character(i);
                        if (character >= 0xD800 && character <= 0xDBFF &&
                            i + 1 < description->length())
                        {
                            const uint32_t lowSurrogate = description->character(i + 1);
                            if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF)
                            {
                                character = 0x10000 + ((character - 0xD800) << 10) + (lowSurrogate - 0xDC00);
                                ++i;
                            }
                        }
                        message += static_cast<wchar_t>(character);
                    }
                }
            }
            RHI_LOG_ERROR(message, RhiApi::Metal);
            return {};
        }

        MTL::Function* function = library->newFunction(
            NS::String::string(
                entryPoint.c_str(),
                NS::UTF8StringEncoding
            )
        );

        if (!function)
        {
            RHI_LOG_ERROR(L"Failed to find Metal shader function", RhiApi::Metal);
            library->release();
            return {};
        }

        return { library, function };
    }
    
    GraphicPipeline::GraphicPipeline(RHI& rhi)
        : BaseGraphicPipiline(rhi)
    {

    }

    GraphicPipeline::~GraphicPipeline()
    {
        m_metalRenderPSO->release();
    }

    bool GraphicPipeline::build(Device& device)
    {
        MTL::Device* metalDevice = Device::Private::getDevice(device);

        if (!metalDevice)
            return false;
        
        const _RenderPassDescriptor* renderPassDescriptor = getRenderPassDescriptor(); 
        if (!renderPassDescriptor)
            return false;

        const auto vertex = getModule<ShaderStage::Vertex>();
        const auto frag = getModule<ShaderStage::Pixel>();

        if (!vertex || !frag || !*vertex || !*frag)
            return false;

        const BinaryModule& vert = **vertex;
        const BinaryModule& pixel = **frag;

        MetalShaderModule vertModule = createShaderModule(
            metalDevice,
            std::span<const uint8_t>(vert.data.get(), vert.size),
            vert.entryPoint
        );

        MetalShaderModule fragModule = createShaderModule(
            metalDevice,
            std::span<const uint8_t>(pixel.data.get(), pixel.size),
            pixel.entryPoint
        );

        MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        renderPipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline", NS::ASCIIStringEncoding));
        renderPipelineDescriptor->setVertexFunction(vertModule.function);
        renderPipelineDescriptor->setFragmentFunction(fragModule.function);
        assert(renderPipelineDescriptor);

        static constexpr size_t MSL_MAX_ATTACHEMENT = 8;
        const std::span<const AttachmentDescriptor> colorAttachments = renderPassDescriptor->getColorAttachements();
        if (colorAttachments.size() > MSL_MAX_ATTACHEMENT)
        {
            RHI_LOG_ERROR(L"Metal supports at most 8 color attachments", RhiApi::Metal);
            renderPipelineDescriptor->release();
            return false;
        }

        size_t i = 0;
        for (; i < colorAttachments.size(); ++i)
        {
            const AttachmentDescriptor& attachmentDescriptor = colorAttachments[i];
            renderPipelineDescriptor->colorAttachments()->object(i)->setPixelFormat(
                TiRHI::Metal::Private::toMetal(attachmentDescriptor.format));
        }

        for (size_t j = i; j < MSL_MAX_ATTACHEMENT; ++j)
        {
            renderPipelineDescriptor->colorAttachments()->object(j)->setPixelFormat(MTL::PixelFormatInvalid);
        }

        if (const std::optional<AttachmentDescriptor>& depthAttachement = renderPassDescriptor->getDepthAttachement())
        {
            renderPipelineDescriptor->setDepthAttachmentPixelFormat(TiRHI::Metal::Private::toMetal(depthAttachement->format));
        }
    

        NS::Error* error = nullptr;
        m_metalRenderPSO = metalDevice->newRenderPipelineState(renderPipelineDescriptor, &error);
        renderPipelineDescriptor->release();

        if (!m_metalRenderPSO)
        {
            RHI_LOG_ERROR(L"Failed to create Metal render pipeline state", RhiApi::Metal);
            return false;
        }

        return true;
    }

} // namespace TiRHI::Metal