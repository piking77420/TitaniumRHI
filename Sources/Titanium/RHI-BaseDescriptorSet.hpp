#ifndef TITANIUM_DESCRIPTOR_SET_H
#define TITANIUM_DESCRIPTOR_SET_H

#include <algorithm>
#include <ranges>
#include <format>
#include <span>
#include <variant>
#include <vector>
#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHI-SpecialMembers.hpp>
#include <Titanium/RHITypes.hpp>
#include <Titanium/Log.hpp>

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
            uint32_t bindingIndex{0};
            uint32_t count{0};
            ShaderStageFlags shaderStageFlag{0};

            auto operator<=>(const BaseBinding&) const = default;

            uint32_t hash() const
            {
                uint32_t hashv = 0;
                hashCombine(hashv, bindingIndex);
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

        struct BindingsLayout
        {
            uint32_t cachedHash;
            std::vector<Binding> bindings;

            bool operator==(const BindingsLayout& other) const
            {
                if (cachedHash != other.cachedHash)
                    return false;

                if (bindings.size() != other.bindings.size())
                    return false;

                for (size_t i = 0; i < bindings.size(); ++i)
                {
                    if (bindings[i].index() != other.bindings[i].index())
                        return false;

                    auto getBase = [](const Binding& binding) -> const BaseBinding&
                    {
                        return std::visit([](const auto& value) -> const BaseBinding& { return value; }, binding);
                    };

                    if (getBase(bindings[i]) != getBase(other.bindings[i]))
                        return false;
                }

                return true;
            }

            bool operator!=(const BindingsLayout& other) const
            {
                return !(*this == other);
            }

            void computeHash()
            {
                cachedHash = 0;

                for (const Binding& binding : bindings)
                {
                    const uint32_t bindingHash = std::visit([](const auto& value) { return value.hash(); }, binding);

                    hashCombine(cachedHash, static_cast<uint32_t>(binding.index()));
                    hashCombine(cachedHash, bindingHash);
                }
            }
        };

        struct BindingsLayoutHash
        {
            std::size_t operator()(const BindingsLayout& bindings) const
            {
                return static_cast<size_t>(bindings.cachedHash);
            }
        };

        struct BindingsLayoutEqual
        {
            bool operator()(const BindingsLayout& a, const BindingsLayout& b) const
            {
                return a == b;
            }
        };

        BaseDescriptorSet() = delete;
        ~BaseDescriptorSet() = default;
        RHI_MOVE_ONLY(BaseDescriptorSet);
        BaseDescriptorSet(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseDescriptorSet<T, TRHI, TTexture, TSampler>>);
        }

        T& bindSampledTexture(ShaderStageFlags shaderStageFlags, Texture* texture, Sampler* sampler,
                              uint32_t bindingIndex, uint32_t count = 1)
        {
            if (texture && sampler)
            {
                Binding& binding = m_bindings.emplace_back();
                SampledTexture& sampledTexture = binding.emplace<SampledTexture>();
                sampledTexture.bindingIndex = bindingIndex;
                sampledTexture.count = count;
                sampledTexture.shaderStageFlag = shaderStageFlags;
                sampledTexture.texture = texture;
                sampledTexture.sampler = sampler;
            }

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

        bool updateSampledTexture(Texture* texture, Sampler* sampler, uint32_t index)
        {
            if (!texture || !sampler)
            {
                RHI_LOG_ERROR(std::format(L"{} invalid param", this->getNameW()), Object::DerivedRhiApi);
                return false;
            }

            if (!onUpdateBindingCheck())
                return false;

            if (!std::holds_alternative<SampledTexture>(m_bindings[index]))
            {
                return false;
            }
            SampledTexture& sampledTexture = std::get<SampledTexture>(m_bindings[index]);
            sampledTexture.texture = texture;
            sampledTexture.sampler = sampler;

            return true;
        }

    protected:
        const std::vector<Binding>& getBindings() const
        {
            return m_bindings;
        }

    private:
        std::vector<Binding> m_bindings;

        DescriptorSetType m_DescriptorSetType = DescriptorSetType::Dynamic;

        bool m_isDirty = false;

        bool onUpdateBindingCheck()
        {
            if (m_DescriptorSetType != DescriptorSetType::Dynamic)
            {
                RHI_LOG_ERROR(std::format(L"{} you cannot update an Immutable descriptor set", this->getNameW()),
                              Object::DerivedRhiApi);
                return false;
            }

            return true;
        }
    };

} // namespace TiRHI

#endif // TITANIUM_DESCRIPTOR_SET_H
