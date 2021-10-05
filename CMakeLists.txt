cmake_minimum_required(VERSION 3.15)
project(soft_rasterizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CImg CONFIG REQUIRED)
add_executable(soft_rasterizer main.cpp image.cpp)
target_link_libraries(soft_rasterizer PRIVATE CImg::CImg)

