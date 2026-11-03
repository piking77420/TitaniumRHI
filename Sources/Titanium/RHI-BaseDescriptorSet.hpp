#ifndef TITANIUM_DESCRIPTOR_SET_H
#define TITANIUM_DESCRIPTOR_SET_H

#include <algorithm>
#include <ranges>
#include <span>
#include <variant>
#include <vector>
#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI
{
    template<typename T, typename TRHI, typename TTexture>
    class BaseDescriptorSet : public Object<T, TRHI>
    {
    public:
        using Texture = TTexture;

        struct BaseBinding
        {
            uint32_t index{0};
            uint32_t count{0};
            ShaderStageFlags shaderStageFlag{0};

            auto operator<=>(const BaseBinding&) = default;

            uint32_t hash() const
            {
                uint32_t hashv = 0;
                hashCombine(hashv, index);
                hashCombine(hashv, count);
                hashCombine(hashv, shaderStageFlag);

                return hashv;
            }
        };

        struct SampledTexture : public BaseBinding
        {
            Texture* texture{nullptr};
            auto operator<=>(const BaseBinding&) = default;
        };

        using Binding = std::variant<SampledTexture>;
        using BindingsLayout = std::vector<DescriptorSet::Binding>;

        BaseDescriptorSet() = delete;
        ~BaseDescriptorSet() = default;
        RHI_MOVE_ONLY(BaseDescriptorSet);
        BaseDescriptorSet(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseDescriptorSet<T, TRHI, TTexture>>);
        }

        T& bindSampledTexture(ShaderStageFlags shaderStageFlags, Texture& texture, uint32_t index, uint32_t count = 1)
        {
            Binding& binding = m_bindings.emplace_back();
            SampledTexture& sampledTexture = binding.emplace<SampledTexture>();
            sampledTexture.index = index;
            sampledTexture.count = count;
            sampledTexture.shaderStageFlag = shaderStageFlags;
            sampledTexture.texture = texture;

            return static_cast<T&>(texture);
        };

        template<>
        struct BindingsLayoutHash
        {
            std::size_t operator()(const std::vector<Binding>& bindings) const
            {
                size_t hash = 0;
                // clang-format off
                const auto getHash = overloads{
                        [](auto&& binding) { 
                           uint32_t hashB = binding.hash();
                           hashCombine(hashB, VariantIndexV<decltype(binding), Binding>)
                           return hashB;
                        }
                };

                std::ranges::for_each(bindings, [&hash, &getHash](const Binding& binding) 
                    {
                        hashCombine(hash, getHash(binding));
                    });
                // clang-format on
                return hash;
            }
        };

    protected:
        const std::vector<Binding>& getBindings() const
        {
            return m_bindings;
        }

    private:
        std::vector<Binding> m_bindings;
    };

} // namespace TiRHI

#endif // TITANIUM_DESCRIPTOR_SET_H
