cmake_minimum_required(VERSION 3.15)
project(soft_rasterizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

add_executable(soft_rasterizer main.cpp image.cpp "utils.cpp" "common_header.hpp" "display.hpp")

target_link_libraries(soft_rasterizer PRIVATE glfw GLEW::GLEW opengl32 glu32)