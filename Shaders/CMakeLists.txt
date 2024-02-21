
file(GLOB SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hlsl ${CMAKE_CURRENT_SOURCE_DIR}/*.hlsli)


if(${CMAKE_GENERATOR} MATCHES "Visual Studio 17 2022")
	file(COPY 
		${SHADERS}/
		DESTINATION 
		${CMAKE_BINARY_DIR}/Source/LowerDeck/Debug/Shaders)
elseif(${CMAKE_GENERATOR} MATCHES "Ninja")
file(COPY 
		${SHADERS}/
		DESTINATION 
		${CMAKE_BINARY_DIR}/Source/LowerDeck/Shaders)
endif()
