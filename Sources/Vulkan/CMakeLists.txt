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

find_package(Vulkan REQUIRED)

add_library(VMA INTERFACE)
target_include_directories(
    VMA
    INTERFACE
    "${VMA_INCLUDE}"
)

target_link_libraries(
    TitaniumRHI
    PUBLIC
    Vulkan::Vulkan
    VMA
)
