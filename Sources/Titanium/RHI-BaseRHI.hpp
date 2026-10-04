#ifndef TITANIUM_RHI_BASE_RHI_H
#define TITANIUM_RHI_BASE_RHI_H

#include <concepts>
#include <span>

#include <Titanium/RHITypes.hpp>
#include <Titanium/RHI-Adapter.hpp>

namespace TiRHI
{
    struct RHIGlobalState
    {
        static inline LogCallBackSignature logCallBack{nullptr};
    };

    template<typename Derived>
    class BaseRHI : public RHIGlobalState
    {
    public:
        using _Derived = Derived;

        BaseRHI() = delete;
        ~BaseRHI() = default;

        BaseRHI(BaseRHI&& otherBaseRHI) noexcept = default;
        BaseRHI(const BaseRHI& otherBaseRHI) = default;

        BaseRHI& operator=(BaseRHI&& otherBaseRHI) noexcept = default;
        BaseRHI& operator=(const BaseRHI& otherBaseRHI) = default;

        BaseRHI(const RhiCreate& rhiCreate);

        size_t getFrameInFlight() const
        {
            // TODO for setter collect ibject create call back with rhi
            // and call onChangeframesInFlight
            return m_framesInFlight;
        }

        const std::span<const Adapter> getAdapters() const noexcept
        {
            return std::span<const Adapter>(m_adapters);
        }

        size_t getCurrentFrame() const;

        void nextFrame();

    protected:
        // Number of frames that may be processed concurrently.
        //
        // Explicit APIs such as Vulkan and DirectX12 may use 2 or 3 frames in flight.
        // Backends that do not currently support multiple frames in flight, such as
        // the OpenGL backend, may force this value to 1 during device creation.
        size_t m_framesInFlight = 2;

        std::vector<Adapter> m_adapters;

        size_t m_currentFrame = 0;
    };
#ifdef max
#undef max
#endif
    template<typename Derived>
    inline BaseRHI<Derived>::BaseRHI(const RhiCreate& rhiCreate)
        : m_framesInFlight(std::max(3uz, rhiCreate.frameInFlight))
    {
        static_assert(std::derived_from<Derived, BaseRHI<Derived>>, "Derived must inherit from RHI<Derived>");

        logCallBack = rhiCreate.logCallback;
    }

    template<typename Derived>
    inline size_t BaseRHI<Derived>::getCurrentFrame() const
    {
        return m_currentFrame;
    }

    template<typename Derived>
    inline void BaseRHI<Derived>::nextFrame()
    {
        m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
    }

}

#endif // TITANIUM_RHI_BASE_RHI_H
