#ifndef TITANIUM_BASE_DEVICE_H
#define TITANIUM_BASE_DEVICE_H

#include <algorithm>

#include <Titanium/RHI-Object.hpp>
#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseRHI.hpp>

namespace TiRHI
{
    class Adapter;

    template<typename T, typename TRHI>
    class BaseDevice : public Object<T, TRHI>
    {
    public:
        BaseDevice() = delete;
        ~BaseDevice() = default;
        BaseDevice(TRHI& rhi)
            : Object<T, TRHI>(rhi)
        {
        }

        template<typename TRHI>
        const Adapter& getSourceAdapter(const BaseRHI<TRHI>& rhi) const
        {
            static_assert(std::derived_from<T, BaseDevice<T, TRHI>>);
            return rhi.getAdapters()[m_adapterIndex];
        }

        std::span<const Adapter::Features> getFeaturesEnable() const
        {
            return m_featuresEnable;
        }

        T& setFeaturesEnable(const std::span<const Adapter::Features>& features)
        {
            m_featuresEnable.insert(m_featuresEnable.end(), features.begin(), features.end());
            return reinterpret_cast<T&>(*this);
        }

    protected:
        size_t m_adapterIndex = 0;

        std::vector<Adapter::Features> m_featuresEnable;

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
