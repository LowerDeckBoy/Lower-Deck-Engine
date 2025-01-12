
# Copy .libs and .dlls into Build folder

# HLSL libraries
file(COPY 
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/dxc/ 
	DESTINATION 
	${BUILD_DIR}/Debug
)

# D3D12 dll
file(COPY
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/D3D12AgilitySDK/D3D12Core.dll
	${CMAKE_CURRENT_SOURCE_DIR}/Libs/D3D12AgilitySDK/d3d12SDKLayers.dll
	DESTINATION
	${BUILD_DIR}/Debug/D3D12/
)
