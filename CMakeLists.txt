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
set(TITANIUM_RHI_BACKEND "Vulkan" CACHE STRING "RHI backend")
set_property(CACHE TITANIUM_RHI_BACKEND PROPERTY STRINGS ${RHI_BACKENDS})
option(TITANIUM_USE_PIX "Enable PIX instrumentation for DirectX 12" OFF)

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

if(WIN32)
    target_compile_definitions(TitaniumRHI PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN)
endif()

## Choose wich backend
## TODO add guard for os
if(TITANIUM_RHI_BACKEND STREQUAL "Vulkan")
    target_compile_definitions(TitaniumRHI PUBLIC TITANIUM_VULKAN=1)
    message(STATUS "RHI Backend: Vulkan")

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)

    target_compile_definitions(TitaniumRHI PUBLIC VK_USE_PLATFORM_WIN32_KHR)

elseif(ANDROID)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_ANDROID_KHR)

    target_compile_definitions(TitaniumRHI PUBLIC VK_USE_PLATFORM_ANDROID_KHR)

elseif(APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_METAL_EXT)

    target_compile_definitions(TitaniumRHI PUBLIC VK_USE_PLATFORM_METAL_EXT)

elseif(UNIX)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR)

    target_compile_definitions(TitaniumRHI PUBLIC VK_USE_PLATFORM_WAYLAND_KHR)
endif()

    target_compile_definitions(TitaniumRHI PUBLIC VK_NO_PROTOTYPES)

    find_package(Vulkan REQUIRED COMPONENTS volk)
    target_link_libraries(TitaniumRHI
        PUBLIC
            Vulkan::volk)

    add_subdirectory(ThirdParty/VulkanMemoryAllocator)
    add_subdirectory(Sources/Vulkan)
elseif(TITANIUM_RHI_BACKEND STREQUAL "DirectX12")
    message(STATUS "RHI Backend: DirectX12")
    target_compile_definitions(TitaniumRHI PUBLIC TITANIUM_DIRECT_X12=1)

if(TITANIUM_USE_PIX)
    message(STATUS "Link PIX")

    include(FetchContent)
    FetchContent_GetProperties(
        WinPixEventRuntime
        SOURCE_DIR 
        PIX_SOURCE_DIR)

    set(PIX_VERSION "1.0.240308001")

    FetchContent_Declare(
        WinPixEventRuntime
        URL "https://www.nuget.org/api/v2/package/WinPixEventRuntime/${PIX_VERSION}"
    )

    FetchContent_MakeAvailable(WinPixEventRuntime)

    FetchContent_GetProperties(
        WinPixEventRuntime
        SOURCE_DIR PIX_SOURCE_DIR
    )

    add_library(PIX INTERFACE)

    target_include_directories(PIX INTERFACE
        "${PIX_SOURCE_DIR}/include/WinPixEventRuntime"
    )

    target_link_libraries(PIX INTERFACE
        "${PIX_SOURCE_DIR}/bin/x64/WinPixEventRuntime.lib"
    )

    target_compile_definitions(PIX INTERFACE
        USE_PIX
    )

    FetchContent_GetProperties(
    WinPixEventRuntime
    SOURCE_DIR PIX_SOURCE_DIR)

    set(
        TITANIUM_PIX_DLL
        "${PIX_SOURCE_DIR}/bin/x64/WinPixEventRuntime.dll"
        CACHE INTERNAL "Path to WinPixEventRuntime.dll"
        FORCE
    )

    message(STATUS "Titanium PIX DLL = ${TITANIUM_PIX_DLL}")

    target_link_libraries(TitaniumRHI PRIVATE PIX)

    target_compile_definitions(
        TitaniumRHI
        PRIVATE TITANIUM_USE_PIX
    )
endif()

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
