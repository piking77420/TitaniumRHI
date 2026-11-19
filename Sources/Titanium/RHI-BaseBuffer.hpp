#ifndef TITANIUM_BASE_BUFFER_H
#define TITANIUM_BASE_BUFFER_H

#include <Titanium/RHI-Resource.hpp>

namespace TiRHI
{
    template<typename T, typename TDevice, typename TRHI>
    class BaseBuffer : public Resource<T, TRHI>
    {
    public:
        using _Derived = T;
        using _Device = TDevice;

        BaseBuffer() = delete;
        ~BaseBuffer() = default;
        BaseBuffer(BaseBuffer&&) noexcept = default;
        BaseBuffer& operator=(BaseBuffer&&) noexcept = default;
        BaseBuffer(TRHI& rhi)
            : Resource<T, TRHI>(rhi)
        {
            static_assert(std::derived_from<T, BaseBuffer<T, _Device, TRHI>>);
        }

        _Derived& setSize(uint64_t size)
        {
            m_size = size;
            return static_cast<T&>(*this);
        }

        uint64_t getSize() const
        {
            return m_size;
        }

        _Derived& setUsage(BufferUsage usage)
        {
            m_usage = usage;
            return static_cast<T&>(*this);
        }

        BufferUsage getUsage() const
        {
            return m_usage;
        }

        _Derived& setMemoryType(BufferMemoryType memoryType)
        {
            m_memoryType = memoryType;
            return static_cast<T&>(*this);
        }

        BufferMemoryType getMemoryType() const
        {
            return m_memoryType;
        }

        _Derived& setUpdateMode(BufferUpdateMode updateMode)
        {
            m_updateMode = updateMode;
            return static_cast<T&>(*this);
        }

        BufferUpdateMode getUpdateMode() const
        {
            return m_updateMode;
        }

        bool build(_Device& device)
        {
            return static_cast<_Derived&>(*this).build(device);
        }

        bool isValid() const
        {
            return static_cast<_Derived&>(*this).isValid();
        }

        bool isInvalid() const
        {
            return static_cast<_Derived&>(*this).isInvalid();
        }

        operator bool() const
        {
            return static_cast<_Derived&>(*this).operator bool();
        }

        std::byte* beginDynamicUpdate()
        {
            return static_cast<_Derived&>(*this).beginDynamicUpdate();
        }

        void endDynamicUpdate()
        {
            static_cast<_Derived&>(*this).endDynamicUpdate();
        }

        struct ScopedDynamicUpdate
        {
            explicit ScopedDynamicUpdate(_Derived& newBuffer)
                : buffer(newBuffer)
            {
                data = buffer.beginDynamicUpdate();
            }

            ~ScopedDynamicUpdate()
            {
                buffer.endDynamicUpdate();
                data = nullptr;
            }

            ScopedDynamicUpdate(const ScopedDynamicUpdate&) = delete;
            ScopedDynamicUpdate& operator=(const ScopedDynamicUpdate&) = delete;

            explicit operator bool() const noexcept
            {
                return data != nullptr;
            }

            _Derived& buffer;

            std::byte* data{nullptr};
        };

    protected:
        uint64_t m_size = {};

        BufferUsage m_usage{BufferUsage::None};

        BufferMemoryType m_memoryType{};

        BufferUpdateMode m_updateMode;
    };

} // namespace TiRHI

#endif // TITANIUM_BASE_BUFFER_H
