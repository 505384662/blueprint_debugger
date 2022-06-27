cmake_minimum_required(VERSION 3.11)

project(blueprint_core)

add_library(blueprint_core SHARED)

add_dependencies(
    blueprint_core
    blueprint_debugger
)

option(BLUEPRINT_USE_LUA_SOURCE "Build with lua source" OFF)

set_target_properties(blueprint_core PROPERTIES PREFIX "")

target_include_directories(blueprint_core PUBLIC
    ${blueprint_SOURCE_DIR}/include
    PRIVATE src
)

target_sources(blueprint_core PUBLIC
    ${blueprint_SOURCE_DIR}/include/blueprint_core/blueprint_core.h
    ${blueprint_core_SOURCE_DIR}/src/blueprint_core.cpp
)

target_link_libraries(
    blueprint_core
    PUBLIC blueprint_debugger
)

install(
    TARGETS blueprint_core
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)