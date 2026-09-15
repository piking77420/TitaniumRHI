cmake_minimum_required(VERSION 3.26)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

target_sources(
    TitaniumRHI
    PRIVATE
        Metal-RHI.cpp
        Metal-RHI.hpp
)

target_include_directories(
    TitaniumRHI
    PUBLIC
        "${ABSOLUTE_METAL_CPP_DIR}"
)

target_include_directories(
    TitaniumRHI
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(APPLE)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALFX_FRAMEWORK MetalFX REQUIRED)

    target_link_libraries(
        TitaniumRHI
        PRIVATE
            ${FOUNDATION_FRAMEWORK}
            ${QUARTZCORE_FRAMEWORK}
            ${METAL_FRAMEWORK}
            ${METALFX_FRAMEWORK}
    )
endif()