#include <algorithm>

#include <Titanium/RHI-Adapter.hpp>
#include <Titanium/RHI-BaseDevice.hpp>

namespace TiRHI
{
    int64_t BaseDevice::getAdapterScore(const Adapter& adapter)
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
        score += properties.deviceType == Adapter::Properties::Type::DiscreteGpu ? scoreDiscretGpu : -scoreDiscretGpu;

        return std::max(static_cast<decltype(score)>(-1), score);
    }

    size_t BaseDevice::getBestAdapter(const std::span<const Adapter>& adapters)
    {
        std::vector<int64_t> scores;
        scores.reserve(adapters.size());
        for (const auto& adapter : adapters)
            scores.emplace_back(getAdapterScore(adapter));

        auto it = std::ranges::max_element(scores);
        const size_t index = std::distance(scores.begin(), it);

        return index;
    }

} // namespace TiRHI
