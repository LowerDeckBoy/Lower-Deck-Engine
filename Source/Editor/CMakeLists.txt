
project(Editor)

file(GLOB IMGUI ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ImGui/*.h ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ImGui/*.cpp)
add_library(${PROJECT_NAME} STATIC "Editor.cpp" "Editor.hpp" "EditorTheme.hpp" "EditorColors.hpp" ${IMGUI})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# MSVC is a preferable compiler
if (MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC /fp:fast /GA /MP /TP /Ot)
	# Can't set /W4 directly without getting a heck lot of warnings
	string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REGEX REPLACE "[/|-]RTC(su|[1su])" "/O2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif (CLANG) # Add Clang specific compiler flag
	target_compile_options(${PROJECT_NAME} PUBLIC -W4 -fp:fast -GA -MP -TP -fexceptions)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Source)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
target_include_directories(${PROJECT_NAME} PUBLIC Engine)
target_link_libraries(${PROJECT_NAME} PUBLIC Engine)
