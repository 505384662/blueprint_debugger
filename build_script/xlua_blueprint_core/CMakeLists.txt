cmake_minimum_required(VERSION 3.11)

project(blueprint_core)

add_library(blueprint_core STATIC)

macro(source_group_by_dir proj_dir source_files)
    if(MSVC OR APPLE)
        get_filename_component(sgbd_cur_dir ${proj_dir} ABSOLUTE)

        foreach(sgbd_file ${${source_files}})
            get_filename_component(sgbd_abs_file ${sgbd_file} ABSOLUTE)
            file(RELATIVE_PATH sgbd_fpath ${sgbd_cur_dir} ${sgbd_abs_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})

            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)

            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC OR APPLE)
endmacro(source_group_by_dir)

set(BLUEPRINT_LUA_VERSION "54" CACHE STRING "Lua version: 51/52/53/54")

if(${BLUEPRINT_LUA_VERSION} STREQUAL "54")
    set(BLUEPRINT_LUA_DIR "lua-5.4.0")
    add_definitions(-DBLUEPRINT_LUA_54)
elseif(${BLUEPRINT_LUA_VERSION} STREQUAL "53")
    set(BLUEPRINT_LUA_DIR "lua-5.3.5")
    add_definitions(-DBLUEPRINT_LUA_53)
elseif(${BLUEPRINT_LUA_VERSION} STREQUAL "52")
    set(BLUEPRINT_LUA_DIR "lua-5.2.4")
    add_definitions(-DBLUEPRINT_LUA_52)
elseif(${BLUEPRINT_LUA_VERSION} STREQUAL "51")
    set(BLUEPRINT_LUA_DIR "lua-5.1.5")
    add_definitions(-DBLUEPRINT_LUA_51)
endif()

set(blueprint_SOURCE_DIR "${blueprint_core_SOURCE_DIR}/../../")
option(BLUEPRINT_COMPILE_AS_LIB "BluePrintLib" ON)
option(BLUEPRINT_USE_LUA_SOURCE "" OFF)

add_subdirectory("${blueprint_SOURCE_DIR}/third-party/libuv-1.29.0" libuv.out)
add_subdirectory("${blueprint_SOURCE_DIR}/blueprint_debugger" blueprint_debugger.out)

target_include_directories(blueprint_core PUBLIC "${blueprint_SOURCE_DIR}/include")

add_dependencies(blueprint_core blueprint_debugger)

target_link_libraries(blueprint_core PUBLIC blueprint_debugger)

target_sources(blueprint_core PUBLIC "${blueprint_SOURCE_DIR}/blueprint_core/src/blueprint_core.cpp")