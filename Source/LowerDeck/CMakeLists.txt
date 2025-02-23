
set(TARGET "LowerDeck")

add_executable(${TARGET} 
	main.cpp
	Application.cpp 
	Application.hpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_23)

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

# Native Win32 window
target_link_options(${TARGET} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/Source)
target_include_directories(${TARGET} PUBLIC Editor)
target_include_directories(${TARGET} PUBLIC Engine)

target_link_libraries(${TARGET} PUBLIC Editor)
target_link_libraries(${TARGET} PUBLIC Engine)

add_custom_command(POST_BUILD
	TARGET ${TARGET}
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	
	${CMAKE_SOURCE_DIR}/Assets/Fonts
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/Assets/Fonts
)

add_custom_command(POST_BUILD
	TARGET ${TARGET}
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	
	${CMAKE_SOURCE_DIR}/Assets/Textures
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/Assets/Textures
)

add_custom_command(POST_BUILD
	TARGET ${TARGET}
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
	
	${CMAKE_SOURCE_DIR}/Shaders
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/Shaders
)
