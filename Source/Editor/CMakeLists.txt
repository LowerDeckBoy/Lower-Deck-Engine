
set(TARGET "Editor")

file(GLOB IMGUI ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ImGui/*.h ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/ImGui/*.cpp)

add_library(${TARGET} STATIC 
	Editor.cpp
	Editor.hpp
	EditorTheme.hpp
	Colors.hpp
	Components/Components.hpp
	Components/Components.cpp
	${IMGUI}
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)

# MSVC is a preferable compiler
if (MSVC)
	# Can't set /W4 directly without getting a heck lot of warnings
	string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# In order to properly set O2 and Ot flags RTC must be disabled
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	target_compile_options(${TARGET} PRIVATE /fp:fast /GA /MP /TP /O2 /Ot)
elseif (CLANG) # Add Clang specific compiler flag
	target_compile_options(${TARGET} PUBLIC -W4 -fp:fast -GA -MP -TP -O2 -Ot -fexceptions)
endif()

target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/Source)
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
target_include_directories(${TARGET} PUBLIC Engine)
target_link_libraries(${TARGET} PUBLIC Engine)
