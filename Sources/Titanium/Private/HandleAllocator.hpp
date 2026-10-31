#ifndef TITANIUM_HANDLE_ALLOCATOR_H
#define TITANIUM_HANDLE_ALLOCATOR_H

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace TiRHI::Private
{
    template<typename Pool, typename NativeHandle, std::size_t Capacity>
    class HandleAllocator
    {
        static_assert(Capacity > 0);

    public:
        struct Handle
        {
            Handle(HandleAllocator& newAllocator, NativeHandle value, std::size_t newPoolIndex,
                   std::size_t newElementIndex, std::uint64_t newGeneration)
                : nativeHandle(std::move(value))
                , poolIndex(newPoolIndex)
                , elementIndex(newElementIndex)
                , generation(newGeneration)
                , allocator(&newAllocator)
            {
            }

            Handle(const Handle&) = delete;
            Handle& operator=(const Handle&) = delete;

            Handle(Handle&& other)
                : nativeHandle(std::move(other.nativeHandle))
                , poolIndex(other.poolIndex)
                , elementIndex(other.elementIndex)
                , generation(other.generation)
                , allocator(std::exchange(other.allocator, nullptr))
            {
            }

            Handle& operator=(Handle&& other)
            {
                if (this != &other)
                {
                    release();

                    nativeHandle = std::move(other.nativeHandle);
                    poolIndex = other.poolIndex;
                    elementIndex = other.elementIndex;
                    generation = other.generation;
                    allocator = std::exchange(other.allocator, nullptr);
                }

                return *this;
            }

            ~Handle()
            {
                release();
            }

            void release()
            {
                if (allocator)
                    allocator->release(*this);
            }

            NativeHandle nativeHandle;
            std::size_t poolIndex;
            std::size_t elementIndex;
            std::uint64_t generation;
            HandleAllocator* allocator = nullptr;
        };

        using CreatePool = std::function<std::optional<Pool>()>;
        using InitializeHandle = std::function<std::optional<NativeHandle>(Pool&, std::size_t)>;

        HandleAllocator() = default;
        ~HandleAllocator() = default;
        // Handles refer to this allocator's pool collection.
        HandleAllocator(const HandleAllocator&) = delete;
        HandleAllocator& operator=(const HandleAllocator&) = delete;
        HandleAllocator(HandleAllocator&&) = delete;
        HandleAllocator& operator=(HandleAllocator&&) = delete;

        bool build(CreatePool createPool, InitializeHandle initializeHandle)
        {
            m_createPool = createPool;
            m_initializeHandle = initializeHandle;

            return m_createPool && m_initializeHandle;
        }

        std::optional<Handle> create()
        {
            for (std::size_t i = 0; i < m_pools.size(); ++i)
            {
                if (m_pools[i]->availableMask.any())
                    return createFromPool(i);
            }

            auto nativePool = m_createPool();
            if (!nativePool)
                return std::nullopt;

            m_pools.push_back(std::make_unique<PoolImpl>(std::move(*nativePool)));

            return createFromPool(m_pools.size() - 1);
        }

        // Only pass handles issued by this allocator.
        bool release(Handle& handle)
        {
            if (handle.poolIndex >= m_pools.size() || handle.elementIndex >= Capacity)
            {
                return false;
            }

            PoolImpl& pool = *m_pools[handle.poolIndex];
            const std::size_t index = handle.elementIndex;

            if (pool.availableMask.test(index) || pool.generations[index] != handle.generation)
            {
                return false;
            }

            ++pool.generations[index];
            pool.availableMask.set(index);

            // Invalidate this instance.
            handle.poolIndex = std::numeric_limits<std::size_t>::max();
            handle.elementIndex = std::numeric_limits<std::size_t>::max();

            return true;
        }

    private:
        struct PoolImpl
        {
            explicit PoolImpl(Pool value)
                : pool(std::move(value))
            {
                availableMask.set();
            }

            Pool pool;
            std::bitset<Capacity> availableMask;
            std::array<std::uint64_t, Capacity> generations{};
        };

        std::optional<Handle> createFromPool(std::size_t poolIndex)
        {
            PoolImpl& pool = *m_pools[poolIndex];

            for (std::size_t index = 0; index < Capacity; ++index)
            {
                if (!pool.availableMask.test(index))
                    continue;

                auto nativeHandle = m_initializeHandle(pool.pool, index);
                if (!nativeHandle)
                    return std::nullopt;

                // Commit only after initialization succeeds.
                pool.availableMask.reset(index);

                return Handle{*this, std::move(*nativeHandle), poolIndex, index, pool.generations[index]};
            }

            return std::nullopt;
        }

        CreatePool m_createPool;
        InitializeHandle m_initializeHandle;
        std::vector<std::unique_ptr<PoolImpl>> m_pools;
    };
}

#endif
