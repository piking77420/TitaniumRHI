#ifndef TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H
#define TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H

#include <vector>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-DescriptorSet.hpp>

namespace TiRHI::DirectX12::Private
{
    struct RootParameterMapping
    {
        uint32_t rootParameterIndex;
    };

    struct OffsetAndCount
    {
        uint32_t offset;
        uint32_t count; // count because for exemple TextureSampler are combined
    };

    using RootParameterTable = std::array<std::vector<RootParameterMapping>, static_cast<size_t>(ShaderStage::Count)>;
    using BindingsSetTableOffset = std::array<std::vector<OffsetAndCount>, static_cast<size_t>(ShaderStage::Count)>;

    struct SpaceDescriptor
    {
        // map rhiBinding to set
        RootParameterTable rootParameterTable;
        // offset for each set in each table of rootParameterTable
        BindingsSetTableOffset bindingsSetTableOffset;

        uint32_t pushConstantParameterIndex = 0;
    };

    struct D3D12DescriptorRootSignatureLayout
    {
        MComPtr<ID3D12RootSignature> rootSignature;
        SpaceDescriptor spaceDescriptor;
    };

    class DescriptorManager
    {
    public:
        using RootSignatureLayoutPtr = std::unique_ptr<D3D12DescriptorRootSignatureLayout>;

        DescriptorManager();
        ~DescriptorManager() = default;

        bool build(MComPtr<ID3D12Device> device);

        // TODO make a struct
        // to hash that have DescriptorSetLayoutEntries
        // push constant
        // and input assembler
        D3D12DescriptorRootSignatureLayout* getOrCreateD3D12DescriptorRootSignatureLayout(
            const std::span<const DescriptorSet::DescriptorSetLayoutEntry>& layouts,
            const std::optional<PushConstant> pushConstant, bool allowInputAssembler);

        D3D12DescriptorRootSignatureLayout* getEmptyDescriptorRootSignatureLayout();

    private:
        struct RootSignatureLayoutKeyView
        {
            std::span<const DescriptorSet::DescriptorSetLayoutEntry> descriptorsLayouts;
            std::optional<PushConstant> pushConstant;
            bool allowInputAssembler = false;
        };

        struct RootSignatureLayoutKey
        {
            std::vector<DescriptorSet::DescriptorSetLayoutEntry> descriptorsLayouts;
            std::optional<PushConstant> pushConstant;
            bool allowInputAssembler = false;

            [[nodiscard]]
            RootSignatureLayoutKeyView view() const
            {
                return {descriptorsLayouts, pushConstant, allowInputAssembler};
            }
        };

        struct BindingsLayoutsHash
        {
            using is_transparent = void;

            size_t operator()(const RootSignatureLayoutKeyView& key) const noexcept
            {
                uint32_t hash = 0;

                for (const auto& entry : key.descriptorsLayouts)
                {
                    hashCombine(hash, entry.setIndex);
                    hashCombine(hash, entry.layout.cachedHash);
                }

                hashCombine(hash, key.pushConstant.has_value());

                if (key.pushConstant)
                {
                    hashCombine(hash, key.pushConstant->flag);
                    hashCombine(hash, key.pushConstant->size);
                }

                hashCombine(hash, key.allowInputAssembler);

                return static_cast<size_t>(hash);
            }

            size_t operator()(const RootSignatureLayoutKey& key) const noexcept
            {
                return (*this)(key.view());
            }
        };

        struct BindingsLayoutsEqual
        {
            using is_transparent = void;

            bool operator()(const RootSignatureLayoutKeyView& a, const RootSignatureLayoutKeyView& b) const
            {
                if (a.allowInputAssembler != b.allowInputAssembler)
                    return false;

                if (a.pushConstant.has_value() != b.pushConstant.has_value())
                    return false;

                if (a.pushConstant)
                {
                    if (a.pushConstant->flag != b.pushConstant->flag || a.pushConstant->size != b.pushConstant->size)
                        return false;
                }

                return std::ranges::equal(
                    a.descriptorsLayouts, b.descriptorsLayouts, [](const auto& lhs, const auto& rhs)
                    { return lhs.setIndex == rhs.setIndex && lhs.layout.cachedHash == rhs.layout.cachedHash; });
            }

            bool operator()(const RootSignatureLayoutKey& a, const RootSignatureLayoutKey& b) const
            {
                return (*this)(a.view(), b.view());
            }

            bool operator()(const RootSignatureLayoutKey& a, const RootSignatureLayoutKeyView& b) const
            {
                return (*this)(a.view(), b);
            }

            bool operator()(const RootSignatureLayoutKeyView& a, const RootSignatureLayoutKey& b) const
            {
                return (*this)(a, b.view());
            }
        };

        ID3D12Device* m_device{nullptr};

        std::unordered_map<RootSignatureLayoutKey, RootSignatureLayoutPtr, BindingsLayoutsHash, BindingsLayoutsEqual>
            m_rootSignatureMap;

        D3D12DescriptorRootSignatureLayout m_emptyDescriptorRootSignatureLayout;
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H
