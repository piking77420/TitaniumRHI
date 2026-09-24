cmake_minimum_required(VERSION 3.26)

target_sources(
    TitaniumRHI
    PRIVATE
        DirectX12-Device.cpp
        DirectX12-Device.hpp
        DirectX12-Header.hpp
        DirectX12-Factory.cpp
        DirectX12-Factory.hpp
        DirectX12-RHI.cpp
        DirectX12-RHI.hpp
)

target_include_directories(
    TitaniumRHI
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(WIN32)
    target_link_libraries(
        TitaniumRHI
        PRIVATE
            d3d12.lib 
            dxgi.lib 
            dxguid.lib 
    )
endif()