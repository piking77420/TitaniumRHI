#ifndef TITANIUM_BASE_DEVICE_H
#define TITANIUM_BASE_DEVICE_H

#include <algorithm>
#include <format>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseRHI.hpp>
#include <Titanium/Log.hpp>

namespace TiRHI
{
    class Adapter;

    template<typename T, typename TRHI>
    class BaseDevice : public Object<T, TRHI>
    {
    public:
        BaseDevice() = delete;
        ~BaseDevice() = default;
        BaseDevice(BaseDevice&& other) = default;
        BaseDevice& operator=(BaseDevice&& other) = default;
        BaseDevice(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
        }

        const Adapter& getSourceAdapter(const BaseRHI<TRHI>& rhi) const
        {
            static_assert(std::derived_from<T, BaseDevice<T, TRHI>>);
            return rhi.getAdapters()[m_adapterIndex];
        }

        std::span<const Adapter::Features> getFeaturesToEnable() const
        {
            return m_featuresEnable;
        }

        T& getFeaturesToEnable(const std::span<const Adapter::Features>& features)
        {
            m_featuresEnable.insert(m_featuresEnable.end(), features.begin(), features.end());
            return reinterpret_cast<T&>(*this);
        }

    protected:
        size_t m_adapterIndex = 0;

        std::vector<Adapter::Features> m_featuresEnable;

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
#ifdef max
#undef max
#endif //
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
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_DEVICE_H
