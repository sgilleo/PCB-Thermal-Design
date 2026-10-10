cmake_minimum_required(VERSION 3.12)

project("PCB_Thermal_Design")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)  


add_executable(${CMAKE_PROJECT_NAME} 
    src/main.cpp
    src/application.cpp
    src/pcb.cpp
    src/component.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    )

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    include
    imgui
    imgui/backends
    imgui/misc/cpp)


target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL)

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Generator is multi-config: ${CMAKE_GENERATOR}")
    message(STATUS "Available configs: ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "Generator is single-config: ${CMAKE_GENERATOR}")
    message(STATUS "Current build type: ${CMAKE_BUILD_TYPE}")

    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_sources(${CMAKE_PROJECT_NAME} PRIVATE imgui/imgui_demo.cpp)
        target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DEBUG_MODE)

        if(
            (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
            (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        )   

            target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4 /WX)

        elseif(
            (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
            (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        )

            target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)

        endif()

    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        if(
            (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
            (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        )   

            target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /O2 /Oi /Gy)

        elseif(
            (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
            (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        )

            target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -O3)

        endif()
    endif()


endif()

