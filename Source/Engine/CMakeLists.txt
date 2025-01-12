
set(TARGET "Engine")

add_library(${TARGET} STATIC)

set(CORE
	Core/CoreDefines.hpp
	Core/CoreMinimal.hpp
	Core/CoreTypes.hpp
	Core/Logger.cpp
	Core/Logger.hpp
	Core/Math.hpp
	Core/RefPtr.hpp
	Core/Singleton.hpp
	Core/String.hpp
)

set(GRAPHICS 
	Graphics/AssetManager.cpp
	Graphics/AssetManager.hpp
	Graphics/ImageBasedLighting.cpp
	Graphics/ImageBasedLighting.hpp
	Graphics/ShaderCompiler.cpp
	Graphics/ShaderCompiler.hpp
	Graphics/Skybox.cpp
	Graphics/Skybox.hpp
	Graphics/ShadowMap.cpp
	Graphics/ShadowMap.hpp
	Graphics/TextureManager.cpp
	Graphics/TextureManager.hpp
)

set(RENDER 
	Render/Renderer.cpp
	Render/Renderer.hpp

	Render/RenderPass/DepthPrepass.hpp
	Render/RenderPass/GBufferPass.cpp
	Render/RenderPass/GBufferPass.hpp
	Render/RenderPass/LightPass.cpp
	Render/RenderPass/LightPass.hpp
	Render/RenderPass/RenderPass.hpp
	Render/RenderPass/SkyPass.cpp
	Render/RenderPass/SkyPass.hpp
)

set(RHI 
	RHI/D3D12/D3D12Adapter.cpp
	RHI/D3D12/D3D12Buffer.cpp
	RHI/D3D12/D3D12Buffer.hpp
	RHI/D3D12/D3D12CommandList.cpp
	RHI/D3D12/D3D12CommandList.hpp
	RHI/D3D12/D3D12Descriptor.hpp
	RHI/D3D12/D3D12DescriptorHeap.cpp
	RHI/D3D12/D3D12DescriptorHeap.hpp
	RHI/D3D12/D3D12Device.cpp
	RHI/D3D12/D3D12Device.hpp
	RHI/D3D12/D3D12Fence.cpp
	RHI/D3D12/D3D12Fence.hpp
	RHI/D3D12/D3D12Memory.cpp
	RHI/D3D12/D3D12Memory.hpp
	RHI/D3D12/D3D12PipelineState.cpp
	RHI/D3D12/D3D12PipelineState.hpp
	RHI/D3D12/D3D12Queue.cpp
	RHI/D3D12/D3D12Queue.hpp
	RHI/D3D12/D3D12Raytracing.cpp
	RHI/D3D12/D3D12Raytracing.hpp
	RHI/D3D12/D3D12Resource.cpp
	RHI/D3D12/D3D12Resource.hpp
	RHI/D3D12/D3D12RHI.cpp
	RHI/D3D12/D3D12RHI.hpp
	RHI/D3D12/D3D12RootSignature.cpp
	RHI/D3D12/D3D12RootSignature.hpp
	RHI/D3D12/D3D12SwapChain.cpp
	RHI/D3D12/D3D12SwapChain.hpp
	RHI/D3D12/D3D12Texture.cpp
	RHI/D3D12/D3D12Texture.hpp
	RHI/D3D12/D3D12Utility.cpp
	RHI/D3D12/D3D12Utility.hpp
	RHI/D3D12/D3D12Viewport.hpp

	RHI/Buffer.hpp
	RHI/BufferConstants.hpp
	RHI/CommandList.hpp
	RHI/Device.hpp
	RHI/PipelineState.hpp
	RHI/Resource.hpp
	RHI/RHI.hpp
	RHI/RHICommon.cpp
	RHI/RHICommon.hpp
	RHI/Shader.hpp
	RHI/SwapChain.hpp
	RHI/Texture.hpp
	RHI/Types.cpp
	RHI/Types.hpp
)

set(SCENE 
	Scene/Entity.hpp
	Scene/Light.hpp
	Scene/Scene.cpp
	Scene/Scene.hpp
	Scene/SceneCamera.cpp
	Scene/SceneCamera.hpp
	Scene/World.cpp
	Scene/World.hpp

	Scene/Components/CameraComponent.hpp
	Scene/Components/Components.hpp
	Scene/Components/LightComponent.hpp

	Scene/Model/Mesh.hpp
	Scene/Model/Model.cpp
	Scene/Model/Model.hpp
)

set(UTILITY 
	Utility/FileSystem.cpp
	Utility/FileSystem.hpp
	Utility/Utility.hpp
)

set(PLATFORM
	Platform/Platform.hpp
	Platform/Timer.hpp
	Platform/Window.cpp
	Platform/Window.hpp
)

target_sources(${TARGET}
	PRIVATE
	${CORE}
	${GRAPHICS}
	${RENDER}
	${RHI}
	${SCENE}
	${UTILITY}
	${PLATFORM}
	"${CMAKE_SOURCE_DIR}/Third-party/D3D12MA/D3D12MemAlloc.cpp"
	"${CMAKE_SOURCE_DIR}/Third-party/D3D12MA/D3D12MemAlloc.h"
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)
set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)

# MSVC is a preferable compiler
if (MSVC)
	# Can't set /W4 directly without getting a heck lot of warnings; 
	string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# In order to properly set O2 and Ot flags RTC must be disabled
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	target_compile_options(${TARGET} PRIVATE /fp:fast /GA /MP /TP /O2 /Ot)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "CLANG") # Add Clang specific compiler flag
	target_compile_options(${TARGET} PRIVATE -W4 -fp:fast -GA -MP -TP -O2 -Ot -fexceptions)
endif()

find_package(assimp CONFIG REQUIRED)
find_package(directxmesh CONFIG REQUIRED)

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PUBLIC ${CMAKE_SOURCE_DIR}/Third-party)

target_link_libraries(${TARGET} PUBLIC d3d12 dxgi dxguid dinput8)
target_link_libraries(${TARGET} PUBLIC assimp::assimp)
target_link_libraries(${TARGET} PUBLIC Microsoft::DirectXMesh)
