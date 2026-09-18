cmake_minimum_required(VERSION 3.26)

target_sources(
    TitaniumRHI
    PRIVATE
        Vulkan-Device.cpp
        Vulkan-Device.hpp
        Vulkan-Functions.hpp
        Vulkan-Instance.cpp
        Vulkan-Instance.hpp
        Vulkan-RHI.cpp
        Vulkan-RHI.hpp
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