cmake_minimum_required(VERSION 3.26)

project(TitaniumRHI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RHI_BACKENDS
    Vulkan
    DirectX12
    Metal
)

set(TITANIUM_RHI_BACKEND "Vulkan" CACHE STRING "RHI backend")
set_property(CACHE TITANIUM_RHI_BACKEND PROPERTY STRINGS ${RHI_BACKENDS})

if(NOT TITANIUM_RHI_BACKEND IN_LIST RHI_BACKENDS)
    message(FATAL_ERROR
        "TITANIUM_RHI_BACKEND must be one of: ${RHI_BACKENDS}"
    )
endif()

add_library(TitaniumRHI STATIC)

if (NOT APPLE)
    add_subdirectory(ThirdParty/VulkanMemoryAllocator)
    set(VMA_INCLUDE "${CMAKE_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/include")
endif()

add_subdirectory(Sources/Titanium)

if(TITANIUM_RHI_BACKEND STREQUAL "Vulkan")
    message(STATUS "RHI Backend: Vulkan")
    add_subdirectory(Sources/Vulkan)

elseif(TITANIUM_RHI_BACKEND STREQUAL "DirectX12")
    message(STATUS "RHI Backend: DirectX12")
    add_subdirectory(Sources/DirectX12)

elseif(TITANIUM_RHI_BACKEND STREQUAL "Metal")
    message(STATUS "RHI Backend: Metal")
    add_subdirectory(Sources/Metal)
endif()

target_link_libraries(
    TitaniumRHI
    PUBLIC
        TitaniumRHIInterface
)

if (BUILD_UNIT_TEST OR TRUE)
    include(CTest)  
    enable_testing()
    add_subdirectory(Sources/UnitTest)
endif()
