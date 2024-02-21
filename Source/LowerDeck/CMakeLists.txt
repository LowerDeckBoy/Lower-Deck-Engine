
project("LowerDeck" VERSION 0.0.1 
	DESCRIPTION "Project application entry-point."
	LANGUAGES CXX)

add_executable(${PROJECT_NAME} "Application.cpp" "Application.hpp" "main.cpp")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# MSVC is a preferable compiler
if (MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC /fp:fast /GA /MP /TP /Ot)
	# Can't set /W4 directly without getting a heck lot of warnings
	string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REGEX REPLACE "[/|-]RTC(su|[1su])" "/O2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif (CLANG) # Add Clang specific compiler flag
	target_compile_options(${PROJECT_NAME} PUBLIC -W4 -fp:fast -GA -MP -TP -fexceptions)
endif()

# Native Win32 window
target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Source)
target_include_directories(${PROJECT_NAME} PUBLIC Editor)
target_include_directories(${PROJECT_NAME} PUBLIC Engine)
target_link_libraries(${PROJECT_NAME} PUBLIC Editor)
target_link_libraries(${PROJECT_NAME} PUBLIC Engine)
