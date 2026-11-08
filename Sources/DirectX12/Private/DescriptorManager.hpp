#ifndef TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H
#define TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H

#include <vector>
#include <DirectX12/DirectX12-Header.hpp>
#include <DirectX12/DirectX12-DescriptorSet.hpp>

namespace TiRHI::DirectX12::Private
{
    struct RootParameterMapping
    {
        uint32_t setIndex;
        uint32_t bindingIndex;
        ShaderStage stage;
        uint32_t rootParameterIndex;
    };

    struct D3D12DescriptorRootSignatureLayout
    {
        std::vector<RootParameterMapping> rootParameterBindings;
        MComPtr<ID3D12RootSignature> rootSignature;
    };

    class DescriptorManager
    {
    public:
        using RootSignatureLayoutPtr = std::unique_ptr<D3D12DescriptorRootSignatureLayout>;

        DescriptorManager();
        ~DescriptorManager() = default;

        bool build(MComPtr<ID3D12Device> device);

        D3D12DescriptorRootSignatureLayout*
        getOrCreateD3D12DescriptorRootSignatureLayout(const std::span<const DescriptorSet::BindingsLayout>& layouts);

        D3D12DescriptorRootSignatureLayout* getEmptyDescriptorRootSignatureLayout();

    private:
        struct BindingsLayoutsHash
        {
            using is_transparent = void;

            size_t operator()(std::span<const DescriptorSet::BindingsLayout> layouts) const noexcept
            {
                uint32_t hash = 0;

                for (const auto& layout : layouts)
                    hashCombine(hash, layout.cachedHash);

                return static_cast<size_t>(hash);
            }
        };

        struct BindingsLayoutsEqual
        {
            using is_transparent = void;

            bool operator()(std::span<const DescriptorSet::BindingsLayout> a,
                            std::span<const DescriptorSet::BindingsLayout> b) const
            {
                return std::ranges::equal(a, b);
            }
        };

        ID3D12Device* m_device{nullptr};

        std::unordered_map<std::vector<DescriptorSet::BindingsLayout>, RootSignatureLayoutPtr, BindingsLayoutsHash,
                           BindingsLayoutsEqual>
            m_rootSignatureMap;

        D3D12DescriptorRootSignatureLayout m_emptyDescriptorRootSignatureLayout;
    };

} // namespace TiRHI::DirectX12::Private

#endif // TITANIUM_DIRECTX12_PRIVATE_DESCRIPTOR_MANAGER_H
