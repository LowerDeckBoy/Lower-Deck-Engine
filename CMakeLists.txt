# CMakeList.txt : Top-level CMake project file

cmake_minimum_required (VERSION 3.5.0)
project("Lower Deck Engine")

add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)
add_definitions(-DRHI_D3D12)

message("Generator: " ${CMAKE_GENERATOR})

set(BUILD_DIR ${CMAKE_BINARY_DIR}/Bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}/Debug/Lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIR}/Debug)

add_subdirectory(Shaders)
add_subdirectory(Third-party)
add_subdirectory(Source)
