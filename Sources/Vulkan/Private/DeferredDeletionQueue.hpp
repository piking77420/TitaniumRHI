#ifndef TITANIUM_VULKAN_PRIVATE_DEFFERED_DELETATION_QUEUE_H
#define TITANIUM_VULKAN_PRIVATE_DEFFERED_DELETATION_QUEUE_H

#include <functional>
#include <queue>

namespace TiRHI::Vulkan
{
    class Device;
} // TiRHI::Vulkan

namespace TiRHI::Vulkan::Private
{
    struct PendingDeletion
    {
        uint64_t retireValue;
        std::move_only_function<void()> destroy;
    };

    class DeferredDeletionQueue
    {
    public:
        DeferredDeletionQueue() = default;
        ~DeferredDeletionQueue();

        void collect(uint64_t completedValue);

        void retire(uint64_t value, std::move_only_function<void()> destroy)
        {
            m_destroyQueue.push_back({value, std::move(destroy)});
        }

    private:
        std::vector<PendingDeletion> m_destroyQueue;
    };

} // TiRHI::Vulkan

#endif // TITANIUM_VULKAN_PRIVATE_DEFFERED_DELETATION_QUEUE_H
