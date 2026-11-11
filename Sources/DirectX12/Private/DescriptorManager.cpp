#include <DirectX12/Private/DescriptorManager.hpp>
#include <DirectX12/Private/RhiToDirectX12.hpp>

namespace TiRHI::DirectX12::Private
{
    struct PendingRootParameter
    {
        D3D12_ROOT_PARAMETER1 parameter;
        uint32_t setIndex;
        uint32_t bindingIndex;
        ShaderStage stage;
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

    std::vector<D3D12_DESCRIPTOR_RANGE1> getRanges(const std::span<const DescriptorSet::BindingsLayout>& layouts)
    {
        std::vector<D3D12_DESCRIPTOR_RANGE1> inRanges;
        size_t count = 0;
        for (const auto& layout : layouts)
            count += layout.bindings.size();
        inRanges.reserve(count);
        return inRanges;
    }

    auto DescriptorManager::getOrCreateD3D12DescriptorRootSignatureLayout(
        const std::span<const DescriptorSet::BindingsLayout>& layouts) -> D3D12DescriptorRootSignatureLayout*
    {
        if (!m_device)
            return nullptr;

        auto it = m_rootSignatureMap.find(layouts);
        if (it != m_rootSignatureMap.end())
        {
            return it->second.get();
        }

        std::vector<D3D12_DESCRIPTOR_RANGE1> ranges = getRanges(layouts);

        static_assert(static_cast<size_t>(ShaderStage::Count) == 14);
        static_assert(static_cast<size_t>(ShaderStageFlags::Callable) == 1u << 13);

        std::array<std::vector<PendingRootParameter>, static_cast<size_t>(ShaderStage::Count)> parametersTable;

        {
            size_t spaceCounterIndex = 0;
            size_t rangeCounterIndex = 0;
            for (const auto& layout : layouts)
            {
                for (const auto& binding : layout.bindings)
                {
                    D3D12_DESCRIPTOR_RANGE1& range = ranges[rangeCounterIndex++];
                    range.RegisterSpace = static_cast<UINT>(spaceCounterIndex);
                    range.OffsetInDescriptorsFromTableStart = 0;

                    std::visit(
                        [&](const auto& value)
                        {
                            using T = std::remove_cvref_t<decltype(value)>;

                            range.NumDescriptors = value.count;
                            range.BaseShaderRegister = value.bindingIndex;
                            if constexpr (std::is_same_v<T, DirectX12::DescriptorSet::SampledTexture>)
                            {
                                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                            }
                            else
                            {
                                static_assert(sizeof(T) == 0, "Unsupported binding type");
                            }
                            uint32_t flags = static_cast<uint32_t>(range.Flags);
                            while (flags != 0)
                            {
                                const uint32_t index = std::countr_zero(flags);

                                flags &= flags - 1;

                                if (index == 5 || index >= 8)
                                    continue;

                                auto& entry = parametersTable[index].emplace_back();
                                entry.parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

                                entry.parameter.ShaderVisibility =
                                    toDirectX12(static_cast<ShaderStageFlags>(1u << index));

                                entry.parameter.DescriptorTable.NumDescriptorRanges = 1;
                                entry.parameter.DescriptorTable.pDescriptorRanges = &range;

                                entry.setIndex = static_cast<uint32_t>(spaceCounterIndex);
                                entry.bindingIndex = value.bindingIndex;
                                entry.stage = static_cast<ShaderStage>(index);
                            }
                        },
                        binding);
                }
                spaceCounterIndex++;
            }
        }

        size_t finalParamSize = 0;
        {
            for (const auto& table : parametersTable)
            {
                finalParamSize += table.size();
            }
        }
        // flate array

        std::vector<D3D12_ROOT_PARAMETER1> finalParameters;
        finalParameters.reserve(finalParamSize);

        std::vector<RootParameterMapping> finalRootMapping;

        for (const auto& table : parametersTable)
        {
            for (const auto& entry : table)
            {
                const uint32_t rootIndex = static_cast<uint32_t>(finalParameters.size());

                finalParameters.emplace_back(entry.parameter);

                finalRootMapping.emplace_back(RootParameterMapping{.setIndex = entry.setIndex,
                                                                   .bindingIndex = entry.bindingIndex,
                                                                   .stage = entry.stage,
                                                                   .rootParameterIndex = rootIndex});
            }
        }

        D3D12_ROOT_SIGNATURE_DESC1 desc{};
        desc.NumParameters = static_cast<UINT>(finalParameters.size());
        desc.pParameters = finalParameters.data();
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

        m_rootSignatureMap.emplace(std::vector<DescriptorSet::BindingsLayout>(layouts.begin(), layouts.end()),
                                   std::move(rootLayout));

        return result;
    }

    D3D12DescriptorRootSignatureLayout* DescriptorManager::getEmptyDescriptorRootSignatureLayout()
    {
        return &m_emptyDescriptorRootSignatureLayout;
    }

} // TiRHI::DirectX12::Private
