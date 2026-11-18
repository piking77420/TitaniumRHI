#include <DirectX12/Private/DescriptorManager.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12::Private
{
    struct DescriptorRangeKey
    {
        D3D12_DESCRIPTOR_RANGE_TYPE type;
        uint32_t count;
        uint32_t bindingIndex;
        uint32_t setIndex;
        D3D12_DESCRIPTOR_RANGE_FLAGS flags;
        uint32_t offset;

        bool operator==(const DescriptorRangeKey&) const = default;
    };

    struct DescriptorRangeKeyHash
    {
        uint32_t operator()(const DescriptorRangeKey& key) const noexcept
        {
            uint32_t hash = 0;

            hashCombine(hash, static_cast<uint32_t>(key.type));
            hashCombine(hash, key.count);
            hashCombine(hash, key.bindingIndex);
            hashCombine(hash, key.setIndex);
            hashCombine(hash, static_cast<uint32_t>(key.flags));
            hashCombine(hash, key.offset);

            return hash;
        }
    };

    struct RangeBuilder
    {
        struct PendingRangeReference
        {
            uint32_t rootParameterIndex;
            uint32_t rangeIndex;
        };

        std::unordered_map<DescriptorRangeKey, uint32_t, DescriptorRangeKeyHash> rangeIndices;
        std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;
        std::vector<PendingRangeReference> pendingRanges;

        uint32_t getOrCreateRangeIndex(const DescriptorRangeKey& key)
        {
            if (auto it = rangeIndices.find(key); it != rangeIndices.end())
            {
                return it->second;
            }

            const uint32_t index = static_cast<uint32_t>(ranges.size());

            ranges.push_back({.RangeType = key.type,
                              .NumDescriptors = key.count,
                              .BaseShaderRegister = key.bindingIndex,
                              .RegisterSpace = key.setIndex,
                              .Flags = key.flags,
                              .OffsetInDescriptorsFromTableStart = key.offset});

            rangeIndices.emplace(key, index);

            return index;
        }

        void addReference(uint32_t rootParameterIndex, uint32_t rangeIndex)
        {
            pendingRanges.push_back({rootParameterIndex, rangeIndex});
        }

        void resolve(std::vector<D3D12_ROOT_PARAMETER1>& parameters) const
        {
            for (const auto& reference : pendingRanges)
            {
                auto& parameter = parameters[reference.rootParameterIndex];

                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

                parameter.DescriptorTable.NumDescriptorRanges = 1;
                parameter.DescriptorTable.pDescriptorRanges = &ranges[reference.rangeIndex];
            }
        }
    };

    DescriptorManager::DescriptorManager()
    {
    }

    bool DescriptorManager::build(MComPtr<ID3D12Device> device)
    {
        m_device = device.Get();
        if (!m_device)
            return false;

        D3D12_ROOT_SIGNATURE_DESC rootDesc{};
        rootDesc.NumParameters = 0;
        rootDesc.pParameters = nullptr;
        rootDesc.NumStaticSamplers = 0;
        rootDesc.pStaticSamplers = nullptr;
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        MComPtr<ID3DBlob> signatureBlob;
        MComPtr<ID3DBlob> errorBlob;

        HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

        hr = m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
                                           IID_PPV_ARGS(&m_emptyDescriptorRootSignatureLayout.rootSignature));
        if (FAILED(hr))
        {
            RHI_LOG_ERROR(L"Failed to create empty root signature", RhiApi::DirectX12);
        }

        return true;
    }

    auto DescriptorManager::getOrCreateD3D12DescriptorRootSignatureLayout(
        const std::span<const DescriptorSet::DescriptorSetLayoutEntry>& DescriptorSetLayoutEntries)
        -> D3D12DescriptorRootSignatureLayout*
    {
        if (!m_device)
            return nullptr;

        auto it = m_rootSignatureMap.find(DescriptorSetLayoutEntries);
        if (it != m_rootSignatureMap.end())
        {
            return it->second.get();
        }

        static_assert(static_cast<size_t>(ShaderStage::Count) == 14);
        static_assert(static_cast<size_t>(ShaderStageFlags::Callable) == 1u << 13);

        RangeBuilder rangesBuilder;
        SpaceDescriptor spaceDescriptor;
        RootParameterTable& rootParameterTable = spaceDescriptor.rootParameterTable;
        BindingsSetTableOffset& bindingsSetTableOffset = spaceDescriptor.bindingsSetTableOffset;
        std::vector<D3D12_ROOT_PARAMETER1> parameters;

        // D3D12 has a 64-DWORD root signature limit. Each descriptor table root parameter costs one DWORD.
        {
            size_t rangeCounterIndex = 0;
            for (const auto& entry : DescriptorSetLayoutEntries)
            {
                size_t descriptorSetLocalBindingIndex = 0;
                for (const auto& binding : entry.layout.bindings)
                {
                    std::visit(
                        [&](auto&& value)
                        {
                            using T = std::remove_cvref_t<decltype(value)>;

                            if constexpr (std::is_same_v<T, DirectX12::DescriptorSet::SampledTexture>)
                            {
                                auto getKey = [&](D3D12_DESCRIPTOR_RANGE_TYPE type) -> DescriptorRangeKey
                                {
                                    DescriptorRangeKey descriptorRangeKey;
                                    descriptorRangeKey.type = type;
                                    descriptorRangeKey.count = value.count;
                                    descriptorRangeKey.bindingIndex = value.bindingIndex;
                                    descriptorRangeKey.setIndex = entry.setIndex;
                                    descriptorRangeKey.flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                                    descriptorRangeKey.offset = 0;

                                    return descriptorRangeKey;
                                };

                                const uint32_t textureRangeIndex = rangesBuilder.getOrCreateRangeIndex(
                                    getKey(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV));

                                const uint32_t samplerRangeIndex = rangesBuilder.getOrCreateRangeIndex(
                                    getKey(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER));

                                forEachShaderStage(
                                    value.shaderStageFlag,
                                    [&](TiRHI::ShaderStageFlags flagBit, uint32_t index) -> void
                                    {
                                        auto& offsetAndCount = bindingsSetTableOffset[index].emplace_back();
                                        offsetAndCount.offset = rootParameterTable[index].size();
                                        offsetAndCount.count = 2;

                                        rootParameterTable[index].resize(rootParameterTable[index].size() + 2);

                                        const uint32_t textureParameterIndex = static_cast<uint32_t>(parameters.size());
                                        const uint32_t samplerParameterIndex = textureParameterIndex + 1;
                                        RootParameterMapping& rootParameterMappingTexture =
                                            rootParameterTable[index][rootParameterTable[index].size() - 2];
                                        RootParameterMapping& rootParameterMappingSampler =
                                            rootParameterTable[index][rootParameterTable[index].size() - 1];
                                        rootParameterMappingTexture.rootParameterIndex = textureParameterIndex;
                                        rootParameterMappingSampler.rootParameterIndex = samplerParameterIndex;

                                        parameters.resize(parameters.size() + 2);

                                        rangesBuilder.addReference(textureParameterIndex, textureRangeIndex);
                                        rangesBuilder.addReference(textureParameterIndex + 1, samplerRangeIndex);
                                    });
                            }
                            else
                            {
                                static_assert(sizeof(T) == 0, "Unsupported binding type");
                            }
                        },
                        binding);
                }
            }
        }

        rangesBuilder.resolve(parameters);

        D3D12_ROOT_SIGNATURE_DESC1 desc{};
        desc.NumParameters = static_cast<UINT>(parameters.size());
        desc.pParameters = parameters.data();
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc{};
        versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        versionedDesc.Desc_1_1 = desc;

        MComPtr<ID3DBlob> signature;
        MComPtr<ID3DBlob> errors;

        HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &signature, &errors);

        if (FAILED(hr))
        {
            const char* errorMessage =
                errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Unknown root signature error";
            std::wstring wideMessage(errorMessage, errorMessage + std::strlen(errorMessage));
            RHI_LOG_ERROR(std::format(L"Failed to create root signature: {}", std::wstring_view(wideMessage)),
                          RhiApi::DirectX12);

            return nullptr;
        }

        MComPtr<ID3D12RootSignature> rootSignature;

        hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                           IID_PPV_ARGS(&rootSignature));

        if (FAILED(hr))
            return nullptr;

        auto rootLayout = std::make_unique<D3D12DescriptorRootSignatureLayout>();
        D3D12DescriptorRootSignatureLayout* result = rootLayout.get();
        result->rootSignature = rootSignature;
        result->spaceDescriptor = std::move(spaceDescriptor);

        m_rootSignatureMap.emplace(std::vector<DescriptorSet::DescriptorSetLayoutEntry>(
                                       DescriptorSetLayoutEntries.begin(), DescriptorSetLayoutEntries.end()),
                                   std::move(rootLayout));

        return result;
    }

    D3D12DescriptorRootSignatureLayout* DescriptorManager::getEmptyDescriptorRootSignatureLayout()
    {
        return &m_emptyDescriptorRootSignatureLayout;
    }

} // TiRHI::DirectX12::Private
