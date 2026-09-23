cmake_minimum_required(VERSION 3.26)

project(TitaniumRHI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CROSSCOMPILING)
    set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
endif()

set(RHI_BACKENDS
    Vulkan
    DirectX12
    Metal
)

if(CMAKE_GENERATOR STREQUAL "Xcode")
    set(TITANIUM_RHI_BACKEND "Metal" CACHE STRING "RHI backend")
else()
    set(TITANIUM_RHI_BACKEND "Vulkan" CACHE STRING "RHI backend")
endif()
set_property(CACHE TITANIUM_RHI_BACKEND PROPERTY STRINGS ${RHI_BACKENDS})

if(NOT TITANIUM_RHI_BACKEND IN_LIST RHI_BACKENDS)
    message(FATAL_ERROR
        "TITANIUM_RHI_BACKEND must be one of: ${RHI_BACKENDS}"
    )
endif()

add_library(TitaniumRHI STATIC)

## Enable Validation Layer
if (VALIDATION_LAYER)
target_compile_definitions(TitaniumRHI PRIVATE TITANIUM_VALIDATION_LAYER=1)
message(STATUS "VALIDATION LAYER ON")
else()
message(STATUS "VALIDATION LAYER OFF")
endif()

add_subdirectory(Sources/Titanium)

## Choose wich backend
## TODO add guard for os
if(TITANIUM_RHI_BACKEND STREQUAL "Vulkan")
    message(STATUS "RHI Backend: Vulkan")
    target_compile_definitions(TitaniumRHI PUBLIC TITANIUM_VULKAN=1)
    add_subdirectory(ThirdParty/VulkanMemoryAllocator)
    add_subdirectory(ThirdParty/Vulkan-Headers)

    set(VMA_INCLUDE "${CMAKE_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/include")
    add_subdirectory(Sources/Vulkan)

elseif(TITANIUM_RHI_BACKEND STREQUAL "DirectX12")
    message(STATUS "RHI Backend: DirectX12")
    target_compile_definitions(TitaniumRHI PUBLIC TITANIUM_DIRECT_X12=1)
    add_subdirectory(Sources/DirectX12)

elseif(TITANIUM_RHI_BACKEND STREQUAL "Metal")
    message(STATUS "RHI Backend: Metal")
        target_compile_definitions(TitaniumRHI PUBLIC TITANIUM_METAL=1)
    set(ABSOLUTE_METAL_CPP_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/metal-cpp/")
    add_subdirectory(Sources/Metal)
endif()

target_link_libraries(
    TitaniumRHI
    PUBLIC
        TitaniumRHIInterface
)

if (BUILD_UNIT_TEST OR True)
    include(CTest)  
    enable_testing()
    add_subdirectory(Sources/UnitTest)
endif()
