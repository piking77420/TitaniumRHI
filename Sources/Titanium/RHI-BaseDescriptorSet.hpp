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
    template<typename T, typename TRHI, typename TTexture, typename TSampler>
    class BaseDescriptorSet : public Object<T, TRHI>
    {
    public:
        using Texture = TTexture;
        using Sampler = TSampler;

        struct BaseBinding
        {
            uint32_t index{0};
            uint32_t count{0};
            ShaderStageFlags shaderStageFlag{0};

            auto operator<=>(const BaseBinding&) const = default;

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
            Sampler* sampler{nullptr};
            auto operator<=>(const SampledTexture&) const = default;
        };

        using Binding = std::variant<SampledTexture>;
        using BindingsLayout = std::vector<Binding>;

        BaseDescriptorSet() = delete;
        ~BaseDescriptorSet() = default;
        RHI_MOVE_ONLY(BaseDescriptorSet);
        BaseDescriptorSet(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseDescriptorSet<T, TRHI, TTexture, TSampler>>);
        }

        T& bindSampledTexture(ShaderStageFlags shaderStageFlags, Texture& texture, Sampler& sampler, uint32_t index,
                              uint32_t count = 1)
        {
            Binding& binding = m_bindings.emplace_back();
            SampledTexture& sampledTexture = binding.emplace<SampledTexture>();
            sampledTexture.index = index;
            sampledTexture.count = count;
            sampledTexture.shaderStageFlag = shaderStageFlags;
            sampledTexture.texture = &texture;
            sampledTexture.sampler = &sampler;

            return static_cast<T&>(*this);
        };

        DescriptorSetType getDescriptorSetType() const
        {
            return m_DescriptorSetType;
        }

        T& setDescriptorType(DescriptorSetType descriptorSetType)
        {
            m_DescriptorSetType = descriptorSetType;
            return static_cast<T&>(*this);
        }

        struct BindingsLayoutHash
        {
            std::size_t operator()(const BindingsLayout& bindings) const
            {
                std::uint32_t hash = 0;

                for (const Binding& binding : bindings)
                {
                    const uint32_t bindingHash = std::visit([](const auto& value) { return value.hash(); }, binding);

                    hashCombine(hash, static_cast<uint32_t>(binding.index()));
                    hashCombine(hash, bindingHash);
                }

                return static_cast<size_t>(hash);
            }
        };

        struct BindingsLayoutEqual
        {
            bool operator()(const BindingsLayout& a, const BindingsLayout& b) const
            {
                if (a.size() != b.size())
                    return false;

                for (size_t i = 0; i < a.size(); ++i)
                {
                    if (a[i].index() != b[i].index())
                        return false;

                    auto getBase = [](const Binding& binding) -> const BaseBinding&
                    {
                        return std::visit([](const auto& value) -> const BaseBinding& { return value; }, binding);
                    };

                    if (getBase(a[i]) != getBase(b[i]))
                        return false;
                }

                return true;
            }
        };

    protected:
        const std::vector<Binding>& getBindings() const
        {
            return m_bindings;
        }

    private:
        std::vector<Binding> m_bindings;

        DescriptorSetType m_DescriptorSetType = DescriptorSetType::Dynamic;
    };

} // namespace TiRHI

#endif // TITANIUM_DESCRIPTOR_SET_H
