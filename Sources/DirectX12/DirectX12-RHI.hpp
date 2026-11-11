#ifndef TITANIUM_DIRECTX12_RHI_H
#define TITANIUM_DIRECTX12_RHI_H

// thanks Maxime "mrouffet" ROUFFET - main developer (maximerouffet@gmail.com)
// for FromVulkanToDirectX12
// https://github.com/mrouffet/FromVulkanToDirectX12/blob/main/Sources/mainDX12.cpp

#include <Titanium/RHI-SpecialMembers.hpp>
#include <stdint.h>
#include <DirectX12-Header.hpp>
#include <DirectX12-Factory.hpp>
#include <DirectX12-Device.hpp>
#include <DirectX12-SwapChain.hpp>

#include <Titanium/RHI-BaseRHI.hpp>
#include <Titanium/RHITypes.hpp>

namespace TiRHI::DirectX12
{
    class RHI : public TiRHI::BaseRHI<RHI>
    {
    public:
        static constexpr TiRHI::RhiApi Api = RhiApi::DirectX12;

        RHI(const RhiCreate& rhiCreate);
        RHI_MOVE_ONLY(RHI);
        ~RHI() = default;

        [[nodiscard]] Device newDevice();

        [[nodiscard]] SwapChain newSwapChain();

        [[nodiscard]] IDXGIFactory6* getNativeFactory();

        [[nodiscard]] RhiApi getApi() const
        {
            return RhiApi::DirectX12;
        }

    private:
        Factory m_factory;

        void enumerateAvailableAdapter();
    };
}

#endif // TITANIUM_DIRECTX12_RHI_H
