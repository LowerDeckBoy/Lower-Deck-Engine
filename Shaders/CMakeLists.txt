
file(GLOB SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hlsl ${CMAKE_CURRENT_SOURCE_DIR}/*.hlsli)

set(DEST "${CMAKE_BINARY_DIR}/Source/LowerDeck/Debug/Shaders")
#if(${CMAKE_GENERATOR} MATCHES "Visual Studio 17 2022")
#	file(COPY 
#		${SHADERS}/
#		DESTINATION 
#		${CMAKE_BINARY_DIR}/Source/LowerDeck/Debug/Shaders)
#elseif(${CMAKE_GENERATOR} MATCHES "Ninja")
#file(COPY 
#		${SHADERS}/
#		DESTINATION 
#		${CMAKE_BINARY_DIR}/Source/LowerDeck/Shaders)
#endif()

#add_custom_command(TARGET LowerDeck POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR} ${DEST}
#	COMMENT "Copying shaders..."
#)
