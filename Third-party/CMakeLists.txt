
# Copy .libs and .dlls into Build folder

# Determine which generator is used
set(COPY_DESTINATION "")

if (${CMAKE_GENERATOR} MATCHES "Visual Studio 17 2022")
	list(APPEND COPY_DESTINATION "${CMAKE_BINARY_DIR}/Source/LowerDeck/Debug/")
elseif(${CMAKE_GENERATOR} MATCHES "Ninja")
	list(APPEND COPY_DESTINATION "${CMAKE_BINARY_DIR}/Source/LowerDeck/")
endif()

# HLSL libraries
file(COPY 
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/dxc/ 
	DESTINATION 
	${COPY_DESTINATION}
)

# meshoptimizer libraries
file(COPY 
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/meshoptimizer/ 
	DESTINATION 
	${COPY_DESTINATION}
)

# D3D12 dll
file(COPY
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/D3D12AgilitySDK/D3D12Core.dll
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/D3D12AgilitySDK/d3d12SDKLayers.dll
	DESTINATION
	${COPY_DESTINATION}/D3D12/
)
