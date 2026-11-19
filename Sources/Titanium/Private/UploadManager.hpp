#ifndef TITANIUM_PRIVATE_UPLOAD_MANAGER_H
#define TITANIUM_PRIVATE_UPLOAD_MANAGER_H

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace TiRHI::Private
{
    template<typename StagingBuffer, typename Factory>
    class UploadManager
    {
    public:
        struct UploadBatch
        {
            std::vector<StagingBuffer> buffers;
        };

        UploadManager() = default;

        explicit UploadManager(Factory factory)
            : m_factory(std::move(factory))
        {
        }

        StagingBuffer acquire(uint64_t size)
        {
            auto minIt = m_available.end();
            uint64_t minCapacity = std::numeric_limits<uint64_t>::max();

            for (auto it = m_available.begin(); it != m_available.end(); ++it)
            {
                const uint64_t capacity = it->getCapacity();

                if (capacity >= size && capacity < minCapacity)
                {
                    minIt = it;
                    minCapacity = capacity;

                    if (capacity == size)
                        break;
                }
            }

            if (minIt != m_available.end())
            {
                StagingBuffer buffer = std::move(*minIt);
                m_available.erase(minIt);
                return buffer;
            }

            return m_factory(size);
        }

        void retire(UploadBatch batch, uint64_t submissionValue)
        {
            if (batch.buffers.empty())
                return;

            m_inFlight.push_back({submissionValue, std::move(batch)});
        }

        void collect(uint64_t completedValue)
        {
            auto it = m_inFlight.begin();

            while (it != m_inFlight.end())
            {
                if (it->retireValue > completedValue)
                {
                    ++it;
                    continue;
                }

                for (auto& buffer : it->batch.buffers)
                    m_available.push_back(std::move(buffer));

                it = m_inFlight.erase(it);
            }
        }

    private:
        struct InFlightBatch
        {
            uint64_t retireValue;
            UploadBatch batch;
        };

        Factory m_factory;

        std::vector<StagingBuffer> m_available;
        std::vector<InFlightBatch> m_inFlight;
    };
} // namespace TiRHI::Private

#endif // TITANIUM_PRIVATE_UPLOAD_MANAGER_H
