cmake_minimum_required(VERSION 3.11)

project(blueprint_tool)

add_executable(blueprint_tool)

add_dependencies(
    blueprint_tool
    shared
)

target_include_directories(blueprint_tool PUBLIC
    ${blueprint_SOURCE_DIR}/include
    ./
)

target_sources(blueprint_tool PRIVATE
    ${blueprint_SOURCE_DIR}/include/shared/shme.h

    ${blueprint_tool_SOURCE_DIR}/src/utility.h
    ${blueprint_tool_SOURCE_DIR}/src/blueprint_tool.h
    ${blueprint_tool_SOURCE_DIR}/src/command_line.h

    ${blueprint_tool_SOURCE_DIR}/src/utility.cpp
    ${blueprint_tool_SOURCE_DIR}/src/blueprint_tool.windows.cpp
    ${blueprint_tool_SOURCE_DIR}/src/command_line.cpp
    ${blueprint_tool_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(
    blueprint_tool
    imagehlp
    ws2_32
    shared
)

install(
    TARGETS blueprint_tool
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)