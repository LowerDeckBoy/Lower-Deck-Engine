# CMakeList.txt : Top-level CMake project file, do global configuration

cmake_minimum_required (VERSION 3.5.0)
project("Lower Deck Engine")

add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)
# TEMPORAL; PLACEHOLDER
add_definitions(-DRHI_D3D12)

message("Generator: " ${CMAKE_GENERATOR})

# Include sub-projects.
add_subdirectory("Shaders")
add_subdirectory("Third-party")
add_subdirectory("Source")
