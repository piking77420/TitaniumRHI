cmake_minimum_required(VERSION 3.26)

target_sources(
    TitaniumRHI
    PRIVATE
        DirectX12-Device.cpp
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