cmake_minimum_required(VERSION 3.26)

target_sources(
    TitaniumRHI
    PRIVATE
        Private/DeviceQueueFamily.cpp
        Private/DeviceQueueFamily.hpp
        Private/RHIToVulkan.cpp
        Private/RHIToVulkan.hpp
        Private/Transition.hpp
        Private/VulkanToRHI.cpp
        Private/VulkanToRHI.hpp
        Vulkan-AcquiredFrame.cpp
        Vulkan-AcquiredFrame.hpp
        Vulkan-CommandList.cpp
        Vulkan-CommandList.hpp
        Vulkan-Device.cpp
        Vulkan-Device.hpp
        Vulkan-Instance.cpp
        Vulkan-Instance.hpp
        Vulkan-RenderPassDescriptor.cpp
        Vulkan-RenderPassDescriptor.hpp
        Vulkan-RenderTargets.cpp
        Vulkan-RenderTargets.hpp
        Vulkan-RHI.cpp
        Vulkan-RHI.hpp
        Vulkan-Surface.cpp
        Vulkan-Surface.hpp
        Vulkan-SwapChain.cpp
        Vulkan-SwapChain.hpp
        Vulkan-Texture.cpp
        Vulkan-Texture.hpp
)

target_include_directories(
    TitaniumRHI
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(VMA INTERFACE)

target_include_directories(
    VMA
    INTERFACE
        "${VMA_INCLUDE}"
)

if(ANDROID)

    find_library(
        VULKAN_LIB
        vulkan
        REQUIRED
    )

    target_compile_definitions(
        TitaniumRHI
        PRIVATE
            VK_USE_PLATFORM_ANDROID_KHR
    )

    target_link_libraries(
        TitaniumRHI
        PUBLIC
            Vulkan::Headers
            ${VULKAN_LIB}
            VMA
    )

else()

    find_package(Vulkan REQUIRED)

    target_link_libraries(
        TitaniumRHI
        PUBLIC
            Vulkan::Headers
            Vulkan::Vulkan
            VMA
    )

endif()