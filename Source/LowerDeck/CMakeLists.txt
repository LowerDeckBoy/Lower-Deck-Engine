
project("LowerDeck" VERSION 0.0.1 
	DESCRIPTION "Project application entry-point."
	LANGUAGES CXX)

add_executable(${PROJECT_NAME} "Application.cpp" "Application.hpp" "main.cpp")

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# MSVC is a preferable compiler
if (MSVC)
	# Can't set /W4 directly without getting a heck lot of warnings
	string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# In order to properly set O2 and Ot flags RTC must be disabled
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	target_compile_options(${PROJECT_NAME} PRIVATE /fp:fast /GA /MP /TP /O2 /Ot)
elseif (CLANG) # Add Clang specific compiler flag
	target_compile_options(${PROJECT_NAME} PUBLIC -W4 -fp:fast -GA -MP -TP -O2 -Ot -fexceptions)
endif()

# Native Win32 window
target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Source)
target_include_directories(${PROJECT_NAME} PUBLIC Editor)
target_include_directories(${PROJECT_NAME} PUBLIC Engine)
target_link_libraries(${PROJECT_NAME} PUBLIC Editor)
target_link_libraries(${PROJECT_NAME} PUBLIC Engine)


# Determine which generator is used
#set(COPY_DESTINATION "")

if (${CMAKE_GENERATOR} MATCHES "Visual Studio 17 2022")
	set(COPY_DESTINATION "${CMAKE_BINARY_DIR}/Source/LowerDeck/Debug")
elseif(${CMAKE_GENERATOR} MATCHES "Ninja")
	lset(COPY_DESTINATION "${CMAKE_BINARY_DIR}/Source/LowerDeck")
endif()

add_custom_command(TARGET LowerDeck PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Shaders "${COPY_DESTINATION}/Shaders"
	COMMENT "Copying shaders..."
)

#add_custom_command(TARGET LowerDeck POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/Assets ${COPY_DESTINATION}/Assets#
	#COMMENT "Copying assets..."
#)
