cmake_minimum_required(VERSION 3.11)

project(blueprint_hook)

add_library(blueprint_hook SHARED)

set_target_properties(blueprint_hook PROPERTIES PREFIX "")

target_include_directories(blueprint_hook PUBLIC
    ${blueprint_SOURCE_DIR}/include
    ${blueprint_SOURCE_DIR}/third-party/EasyHook/EasyHookDll
    PRIVATE src
)

target_sources(blueprint_hook PRIVATE
    ${blueprint_SOURCE_DIR}/include/blueprint_hook/blueprint_hook.h

    # src
    ${blueprint_hook_SOURCE_DIR}/src/dllmain.cpp
    ${blueprint_hook_SOURCE_DIR}/src/blueprint_hook.cpp
    ${blueprint_hook_SOURCE_DIR}/src/blueprint_hook.windows.cpp

    # src/libpe
    ${blueprint_hook_SOURCE_DIR}/src/libpe/htable.cpp
    ${blueprint_hook_SOURCE_DIR}/src/libpe/htable.h
    ${blueprint_hook_SOURCE_DIR}/src/libpe/libpe.cpp
    ${blueprint_hook_SOURCE_DIR}/src/libpe/libpe.h
    ${blueprint_hook_SOURCE_DIR}/src/libpe/llist.cpp
    ${blueprint_hook_SOURCE_DIR}/src/libpe/llist.h
)

add_dependencies(
    blueprint_hook
    blueprint_debugger
    EasyHook
    shared
)

target_link_libraries(
    blueprint_hook
    PRIVATE
    blueprint_debugger
    EasyHook
    shared
)

install(
    TARGETS blueprint_hook
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)