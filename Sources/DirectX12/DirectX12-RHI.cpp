#include <DirectX12-RHI.hpp>

#include <format>

#include <Titanium/Log.hpp>
#include <DirectX12/DirectX12-Utils.hpp>

namespace TiRHI::DirectX12
{
    std::vector<Adapter::Features> getAdapterFeatures(ID3D12Device* device)
    {
        std::vector<Adapter::Features> features;

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5{};

        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
        {
            if (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0)
            {
                features.push_back(Adapter::Features::RayTracingPipeline);
            }

            if (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1)
            {
                features.push_back(Adapter::Features::RayQuery);
            }
        }

        D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7{};

        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))))
        {
            if (options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
            {
                features.push_back(Adapter::Features::MeshShader);
            }
        }

        return features;
    }

    Adapter::Properties getAdapterProperties(const DXGI_ADAPTER_DESC1& desc, ID3D12Device* device)
    {
        Adapter::Properties rhiProperty{};
        rhiProperty.limits.minUniformBufferOffset = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

        rhiProperty.memoryLimits.vramMemoryBytes = static_cast<uint64_t>(desc.DedicatedVideoMemory);

        D3D12_FEATURE_DATA_ARCHITECTURE1 architecture{};
        architecture.NodeIndex = 0;

        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &architecture, sizeof(architecture))))
        {
            rhiProperty.deviceType =
                architecture.UMA ? Adapter::Properties::Type::IntegratedGpu : Adapter::Properties::Type::DiscreteGpu;
        }
        else
        {
            rhiProperty.deviceType = Adapter::Properties::Type::Unknow;
        }

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            rhiProperty.deviceType = Adapter::Properties::Type::Cpu;
        }

        return rhiProperty;
    }

    RHI::RHI(const RhiCreate& rhiCreate)
        : BaseRHI<RHI>(rhiCreate)
    {
        enumerateAvailableAdapter();
    }

    void RHI::enumerateAvailableAdapter()
    {
        m_adapters.clear();
        std::vector<MComPtr<IDXGIAdapter1>> dxAdatpers = Internal::getAllNativeAdapters(getNativeFactory());
        m_adapters.reserve(dxAdatpers.size());

        for (auto& dxAdatper : dxAdatpers)
        {
            DXGI_ADAPTER_DESC1 desc{};
            dxAdatper->GetDesc1(&desc);

            const std::wstring_view wideName = desc.Description;
            const std::string name(wideName.begin(), wideName.end());

            MComPtr<ID3D12Device> device;

            const bool isSoftware = (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
            if (isSoftware)
                continue;

            if (FAILED(D3D12CreateDevice(dxAdatper.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
            {
                RHI_LOG_ERROR(std::format(L"Failed to Create fake device to check featurs of the adapter {}", wideName),
                              RhiApi::DirectX12);
                continue;
            }

            m_adapters.emplace_back(name, getAdapterFeatures(device.Get()), getAdapterProperties(desc, device.Get()),
                                    desc.VendorId);
        }
    }

    Device RHI::newDevice()
    {
        return Device(*this);
    }

    SwapChain RHI::newSwapChain()
    {
        return SwapChain(*this);
    }

    IDXGIFactory6* RHI::getNativeFactory()
    {
        return m_factory.getFactory().Get();
    }

} // namespace TiRHI::DirectX12
