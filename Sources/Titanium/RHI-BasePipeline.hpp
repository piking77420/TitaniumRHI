#ifndef TITANIUM_PIPELINE_H
#define TITANIUM_PIPELINE_H

#include <span>
#include <memory>
#include <vector>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHI-EnumToString.hpp>

namespace TiRHI
{
#define RHI_SHADER_STAGE_LIST(X)                                                                                       \
    X(Vertex)                                                                                                          \
    X(Hull)                                                                                                            \
    X(Domain)                                                                                                          \
    X(Geometry)                                                                                                        \
    X(Pixel)                                                                                                           \
    X(Amplification)                                                                                                   \
    X(Mesh)                                                                                                            \
    X(Compute)                                                                                                         \
    X(RayGeneration)                                                                                                   \
    X(AnyHit)                                                                                                          \
    X(ClosestHit)                                                                                                      \
    X(Miss)                                                                                                            \
    X(Intersection)                                                                                                    \
    X(Callable)

    enum struct ShaderStage : uint8_t
    {
#define X(name) name,
        RHI_SHADER_STAGE_LIST(X)
#undef X
            Count
    };

    IMPLEMENT_TO_STRING_TITANIUM(ShaderStage, RHI_SHADER_STAGE_LIST)

    template<typename T, typename TRHI>
    class BasePipeline : public Object<T, TRHI>
    {
    public:
        BasePipeline() = delete;
        ~BasePipeline() = default;
        RHI_MOVE_ONLY(BasePipeline)
        BasePipeline(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BasePipeline<T, TRHI>>);
        }

        BasePipeline& addModule(ShaderStage shaderStage, std::unique_ptr<uint8_t[]> data, uint32_t size)
        {
            m_binaryModule.emplace_back(BinaryModule{shaderStage, size, std::move(data)});
            return static_cast<T&>(*this);
        }

    protected:
        struct BinaryModule
        {
            ShaderStage stage;
            uint32_t size;
            std::unique_ptr<uint8_t[]> data;
        };

        std::span<const BinaryModule> getModules()
        {
            return m_binaryModule;
        }

        template<ShaderStage stage>
        bool getModule(BinaryModule** const module)
        {
            auto it =
                std::ranges::find_if(m_binaryModule, [](const BinaryModule& binary) { return binary.stage == stage; });

            if (it == m_binaryModule.end())
            {
                *module = nullptr;
                return false;
            }

            *module = &(*it);
            return true;
        }

    private:
        std::vector<BinaryModule> m_binaryModule;
    };

    template<typename T, typename TRHI>
    class BaseGraphicPipiline : public BasePipeline<T, TRHI>
    {
    public:
        BaseGraphicPipiline() = delete;
        ~BaseGraphicPipiline() = default;
        RHI_MOVE_ONLY(BaseGraphicPipiline)
        BaseGraphicPipiline(TRHI& rhi)
            : BasePipeline<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseGraphicPipiline<T, TRHI>>);
        }

    private:
    };

} // namespace TiRHI

#endif // TITANIUM_PIPELINE_H
