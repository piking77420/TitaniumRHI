#ifndef TITANIUM_BASE_DEVICE_H
#define TITANIUM_BASE_DEVICE_H

#include <algorithm>
#include <format>
#include <string_view>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseRHI.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    class Adapter;

    template<typename T, typename TRHI, typename TAcquireFrame, typename TCommandList>
    class BaseDevice 
    {
    public:
        BaseDevice() = delete;
        ~BaseDevice() = default;
        BaseDevice(BaseDevice&& other) = default;
        BaseDevice& operator=(BaseDevice&& other) = delete;
        BaseDevice(TRHI& rhi)
            : m_rhi(rhi)
        {
            static_assert(std::derived_from<T, BaseDevice<T, TRHI, TAcquireFrame, TCommandList>>);
        }

        TRHI& getRHI()
        {
            return m_rhi;
        }

        const TRHI& getRHI() const 
        {
            return m_rhi;
        }

        std::string_view getName() const
        {
            return m_name;
        }

        std::wstring getNameW() const
        {
            return m_name.empty() ? std::wstring{m_name.begin(), m_name.end()} : std::wstring{};
        }

        T& setName(std::string name)
        {
            m_name = std::move(name);
            return static_cast<T&>(*this);
        }

         T& setName(const std::wstring& newName)
        {
            if (!newName.empty())
            {
                m_name = std::string(newName.begin(), newName.end());
            }
            return reinterpret_cast<T&>(*this);
        }


        struct SubmitPresentDesc
        {
            TCommandList* commandList;
            std::span<TAcquireFrame> acquireFrames;
        };

        const Adapter& getSourceAdapter(const BaseRHI<TRHI>& rhi) const
        {
            return rhi.getAdapters()[m_adapterIndex];
        }

        std::span<const Adapter::Features> getFeaturesEnable() const
        {
            return m_featuresEnable;
        }

        T& getFeaturesEnable(const std::span<const Adapter::Features>& features)
        {
            m_featuresEnable.insert(m_featuresEnable.end(), features.begin(), features.end());
            return reinterpret_cast<T&>(*this);
        }

        uint32_t alignConstantBuffer(uint32_t size)
        {
            if (m_adapter.getProperties().limits.minUniformBufferOffset == 0)
                return 0;

            return alignUp(size, m_adapter.getProperties().limits.minUniformBufferOffset);
        }

        uint64_t alignConstantBuffer(uint64_t size)
        {
            if (m_adapter.getProperties().limits.minUniformBufferOffset == 0)
                return 0;

            return alignUp(size, static_cast<uint64_t>(m_adapter.getProperties().limits.minUniformBufferOffset));
        }

    protected:
        size_t m_adapterIndex = 0;

        std::vector<Adapter::Features> m_featuresEnable;

        Adapter m_adapter;

        void onBuild(const std::span<const Adapter>& adapters, std::optional<size_t> index = {})
        {
            m_adapterIndex = index ? *index : BaseDevice::getBestAdapter(adapters);
            if (m_adapterIndex >= adapters.size())
            {
                RHI_LOG_ERROR(std::format(L"Failed To Find adapter for device {}", this->getNameW()),
                              this->getRHI().getApi());
            }
        }

        void onCreateDeviceSuccess(const Adapter& adapter, bool success)
        {
            if (success)
            {
                RHI_LOG_INFO(std::format(L"Create Device success from adaptater {}",
                                         [&]()
                                         {
                                             const std::string_view name = adapter.getName();
                                             return std::wstring(name.begin(), name.end());
                                         }()),
                             this->getRHI().getApi());
            }
            else
            {
                RHI_LOG_INFO(std::format(L"Create Device failed from adaptater {}",
                                         [&]()
                                         {
                                             const std::string_view name = adapter.getName();
                                             return std::wstring(name.begin(), name.end());
                                         }()),
                             this->getRHI().getApi());
            }
        }

        static int64_t getAdapterScore(const Adapter& adapter)
        {
            int64_t score = 0;

            const std::span<const Adapter::Features> features = adapter.getFeatures();

            static constexpr int64_t scoreMajorFeatures = 4096;
            if (std::ranges::contains(features, Adapter::Features::RayQuery))
                score += scoreMajorFeatures;

            if (std::ranges::contains(features, Adapter::Features::RayTracingPipeline))
                score += scoreMajorFeatures;

            if (std::ranges::contains(features, Adapter::Features::MeshShader))
                score += scoreMajorFeatures;

            const Adapter::Properties& properties = adapter.getProperties();
            static constexpr int64_t scoreDiscretGpu = 2048;
            score +=
                properties.deviceType == Adapter::Properties::Type::DiscreteGpu ? scoreDiscretGpu : -scoreDiscretGpu;

            return std::max(static_cast<decltype(score)>(-1), score);
        }

        static size_t getBestAdapter(const std::span<const Adapter>& adapters)
        {
            std::vector<int64_t> scores;
            scores.reserve(adapters.size());
            for (const auto& adapter : adapters)
                scores.emplace_back(getAdapterScore(adapter));

            auto it = std::ranges::max_element(scores);
            const size_t index = std::distance(scores.begin(), it);

            return index;
        }

    private:
        TRHI& m_rhi;

        std::string m_name;
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_DEVICE_H
