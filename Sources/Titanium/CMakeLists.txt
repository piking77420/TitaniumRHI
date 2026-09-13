cmake_minimum_required(VERSION 3.26)

add_library(TitaniumRHIInterface INTERFACE)

target_sources(
    TitaniumRHIInterface
    INTERFACE
        Device.hpp
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