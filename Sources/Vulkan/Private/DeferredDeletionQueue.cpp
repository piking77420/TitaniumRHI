#include <Vulkan/Private/DeferredDeletionQueue.hpp>

namespace TiRHI::Vulkan::Private
{
    DeferredDeletionQueue::~DeferredDeletionQueue()
    {
        // device wait shoudl have be called
        for (size_t i = 0; i < m_destroyQueue.size(); i++)
        {
            m_destroyQueue[i].destroy();
        }
        m_destroyQueue.clear();
    }

    void DeferredDeletionQueue::collect(uint64_t completedValue)
    {
        std::erase_if(m_destroyQueue,
                      [completedValue](PendingDeletion& entry)
                      {
                          if (entry.retireValue > completedValue)
                              return false;

                          entry.destroy();
                          return true;
                      });
    }

} // TiRHI::Vulkan::Private
