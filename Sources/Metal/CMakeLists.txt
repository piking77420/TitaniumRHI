cmake_minimum_required(VERSION 3.26)

target_sources(
    TitaniumRHI
    PRIVATE
        Metal-AcquiredFrame.cpp
        Metal-AcquiredFrame.hpp
        Metal-CommandList.cpp
        Metal-CommandList.hpp
        Metal-Device.cpp
        Metal-Device.hpp
        Metal-GraphicPipeline.cpp
        Metal-GraphicPipeline.hpp
        Metal-RenderPassDescriptor.cpp
        Metal-RenderPassDescriptor.hpp
        Metal-RenderTargets.cpp
        Metal-RenderTargets.hpp
        Metal-RHI.cpp
        Metal-RHI.hpp
        Metal-Surface.cpp
        Metal-Surface.hpp
        Metal-SwapChain.cpp
        Metal-SwapChain.hpp
        Metal-Texture.cpp
        Metal-Texture.hpp
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