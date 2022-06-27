cmake_minimum_required(VERSION 3.11)

project(blueprint_debugger)

add_library(blueprint_debugger STATIC)

add_dependencies(
    blueprint_debugger
    uv
)

target_include_directories(blueprint_debugger PUBLIC
    ${blueprint_SOURCE_DIR}/include
    ${blueprint_SOURCE_DIR}/third-party/rapidjson-1.1.0/include
    ${blueprint_SOURCE_DIR}/third-party/libuv-1.29.0/include
    PRIVATE src
)

target_sources(blueprint_debugger PUBLIC

    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/blueprint_debugger.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/blueprint_debugger_manager.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/blueprint_facade.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/hook_state.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/lua_version.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/transporter.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/types.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/blueprint_helper.h

    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/api/lua_api.h

    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/proto/pipeline_client_transporter.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/proto/pipeline_server_transporter.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/proto/socket_client_transporter.h
    ${blueprint_SOURCE_DIR}/include/blueprint_debugger/proto/socket_server_transporter.h

    # src/api
    ${blueprint_debugger_SOURCE_DIR}/src/api/lua_api.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state.cpp

    # src/proto
    ${blueprint_debugger_SOURCE_DIR}/src/proto/pipeline_client_transporter.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/proto/pipeline_server_transporter.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/proto/socket_client_transporter.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/proto/socket_server_transporter.cpp

    # src
    ${blueprint_debugger_SOURCE_DIR}/src/blueprint_debugger.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/blueprint_debugger_manager.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/blueprint_facade.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/blueprint_helper.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/hook_state.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/stack.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/transporter.cpp
    ${blueprint_debugger_SOURCE_DIR}/src/lua_version.cpp
)

target_compile_definitions(blueprint_debugger PUBLIC -DRAPIDJSON_HAS_STDSTRING)

if(NOT WIN32)
    target_compile_options(blueprint_debugger PUBLIC -fPIC)
endif()

if(BLUEPRINT_USE_LUA_SOURCE)
    target_compile_definitions(blueprint_debugger PUBLIC -DBLUEPRINT_USE_LUA_SOURCE)
    target_include_directories(blueprint_debugger PUBLIC
        ${blueprint_SOURCE_DIR}/third-party/${BLUEPRINT_LUA_DIR}/src
    )

    if(${BLUEPRINT_LUA_VERSION} STREQUAL "jit")
        target_link_directories(blueprint_debugger PUBLIC
            ${blueprint_SOURCE_DIR}/third-party/${BLUEPRINT_LUA_DIR}/src
        )
        target_link_libraries(
            blueprint_debugger
            PRIVATE lua51.lib
        )
        target_sources(blueprint_debugger
            PRIVATE
            ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_jit.cpp
        )
    else()
        target_link_libraries(
            blueprint_debugger
            PRIVATE lua${BLUEPRINT_LUA_VERSION}
        )
        target_sources(blueprint_debugger
            PRIVATE
            ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_${BLUEPRINT_LUA_VERSION}.cpp
        )
    endif()

elseif(BLUEPRINT_COMPILE_AS_LIB)
    message("blueprint build as lib")
    target_compile_definitions(blueprint_debugger PUBLIC -DBLUEPRINT_USE_LUA_SOURCE)
    target_sources(blueprint_debugger
        PRIVATE
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_${BLUEPRINT_LUA_VERSION}.cpp
    )
else()
    target_include_directories(blueprint_debugger PRIVATE
        ${blueprint_SOURCE_DIR}/third-party
    )

    target_sources(blueprint_debugger
        PUBLIC
        ${blueprint_SOURCE_DIR}/include/blueprint_debugger/api/lua_api_loader.h
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_api_loader.cpp

        PRIVATE
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_54.cpp
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_53.cpp
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_52.cpp
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_51.cpp
        ${blueprint_debugger_SOURCE_DIR}/src/api/lua_state/lua_state_jit.cpp
    )
endif(BLUEPRINT_USE_LUA_SOURCE)

target_link_libraries(
    blueprint_debugger
    PUBLIC uv_a
)
