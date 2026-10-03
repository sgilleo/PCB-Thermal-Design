cmake_minimum_required(VERSION 3.12)

project("PCB_Thermal_Design")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} 
    src/main.cpp
    src/application.cpp
    src/pcb.cpp
    src/component.cpp
    imgui/imgui_demo.cpp
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
