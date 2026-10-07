cmake_minimum_required(VERSION 3.26)

add_library(TitaniumRHIInterface INTERFACE)

target_sources(
    TitaniumRHIInterface
    INTERFACE
        Log.hpp
        RHI-BaseAcquiredFrame.cpp
        RHI-BaseAcquiredFrame.hpp
        RHI-Adapter.cpp
        RHI-Adapter.hpp
        RHI-BaseDevice.cpp
        RHI-BaseDevice.hpp
        RHI-BaseCommandList.cpp
        RHI-BaseCommandList.hpp
        RHI-BaseRHI.hpp
        RHI-BaseSwapChain.cpp
        RHI-BaseSwapChain.hpp
        RHI-DebugScope.cpp
        RHI-DebugScope.hpp
        RHI-EnumToString.hpp
        RHI-Object.hpp
        RHI-Resource.cpp
        RHI-Resource.hpp
        RHI-Surface.cpp
        RHI-Surface.hpp
        RHITypes.hpp
        TitaniumHeader.hpp
        Version.hpp
)

get_filename_component(
    PARENT_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}"
    DIRECTORY
)
target_include_directories(
    TitaniumRHIInterface
    INTERFACE
        ${PARENT_DIR}
)