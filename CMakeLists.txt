cmake_minimum_required(VERSION 3.16)

project("PCB_Thermal_Design" C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# In single-config generators (Make/Ninja): Release by default.
# In multi-config (Visual Studio/Xcode) choosen with --config.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)
find_package(OpenGL REQUIRED)

# Avoid looking for Linux libraries if building for Windows
if(WIN32)
    set(glfw3_FOUND FALSE)
else()
    find_package(glfw3 3.3 QUIET)
endif()

if(NOT glfw3_FOUND)
    message(STATUS "Using FetchContent for GLFW...")
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(glfw)
endif()

add_executable(${CMAKE_PROJECT_NAME} 
    src/main.cpp
    src/application.cpp
    src/pcb.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    resources.rc #Windows app icon
    )

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    include
    imgui
    imgui/backends
    imgui/misc/cpp)


target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
    #nfd
    )
    
# Force GCC/MinGW libraries static link when building for Windows 
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()

# Generator expression: ($<CONFIG:...>) also work in Visual Studio / Xcode
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG_MODE>)

#Enable compiler warnings
if(MSVC) 
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)  
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# Copies resources to the executable folder
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/fonts)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/fonts
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts)
endif()

# Create symlink for compile_commands.json for Clangd extension
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()




