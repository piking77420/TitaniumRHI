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

        D3D12DescriptorRootSignatureLayout* getOrCreateD3D12DescriptorRootSignatureLayout(
            const std::span<const DescriptorSet::DescriptorSetLayoutEntry>& layouts);

        D3D12DescriptorRootSignatureLayout* getEmptyDescriptorRootSignatureLayout();

    private:
        struct BindingsLayoutsHash
        {
            using is_transparent = void;

            size_t operator()(
                std::span<const DescriptorSet::DescriptorSetLayoutEntry> DescriptorSetLayoutEntries) const noexcept
            {
                uint32_t hash = 0;

                for (const auto& entry : DescriptorSetLayoutEntries)
                {
                    hashCombine(hash, entry.setIndex);
                    hashCombine(hash, entry.layout.cachedHash);
                }

                return static_cast<size_t>(hash);
            }
        };

        struct BindingsLayoutsEqual
        {
            using is_transparent = void;

            bool operator()(std::span<const DescriptorSet::DescriptorSetLayoutEntry> a,
                            std::span<const DescriptorSet::DescriptorSetLayoutEntry> b) const
            {
                return std::ranges::equal(a, b);
            }
        };

        ID3D12Device* m_device{nullptr};

        std::unordered_map<std::vector<DescriptorSet::DescriptorSetLayoutEntry>, RootSignatureLayoutPtr,
                           BindingsLayoutsHash, BindingsLayoutsEqual>
            m_rootSignatureMap;

        D3D12DescriptorRootSignatureLayout m_emptyDescriptorRootSignatureLayout;
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H
