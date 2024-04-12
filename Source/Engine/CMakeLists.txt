
project(Engine)

# TEMPORAL

file(GLOB CORE ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp)
file(GLOB GRAPHICS ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp)
file(GLOB RENDER ${CMAKE_CURRENT_SOURCE_DIR}/Render/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Render/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Render/RenderPass/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Render/RenderPass/*.cpp)
file(GLOB RHI ${CMAKE_CURRENT_SOURCE_DIR}/RHI/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/RHI/*.cpp)
file(GLOB D3D12 ${CMAKE_CURRENT_SOURCE_DIR}/RHI/D3D12/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/RHI/D3D12/*.cpp)
file(GLOB SCENE ${CMAKE_CURRENT_SOURCE_DIR}/Scene/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Scene/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Components/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Model/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Model/*.hpp)
file(GLOB UTILITY ${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.cpp)
file(GLOB PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/Platform/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/Platform/*.cpp)

add_library(${PROJECT_NAME} STATIC
	${CORE}
	${GRAPHICS}
	${RENDER}
	${RHI}
	${D3D12}
	${SCENE}
	${UTILITY}
	${PLATFORM}
	"${CMAKE_SOURCE_DIR}/Third-party/D3D12MA/D3D12MemAlloc.cpp"
	"${CMAKE_SOURCE_DIR}/Third-party/D3D12MA/D3D12MemAlloc.h"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# MSVC is a preferable compiler
if (MSVC)
	# Can't set /W4 directly without getting a heck lot of warnings
	string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	# In order to properly set O2 and Ot flags RTC must be disabled
	string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	target_compile_options(${PROJECT_NAME} PRIVATE /fp:fast /GA /MP /TP /O2 /Ot)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "CLANG") # Add Clang specific compiler flag
	target_compile_options(${PROJECT_NAME} PRIVATE -W4 -fp:fast -GA -MP -TP -O2 -Ot -fexceptions)
endif()

message("Compile flags: " ${CMAKE_CXX_FLAGS})
message("Compile debug flags: " ${CMAKE_CXX_FLAGS_DEBUG})
find_package(assimp CONFIG REQUIRED)
find_package(DirectXTex CONFIG REQUIRED)
find_package(DirectXTK12 CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Third-party)

target_link_libraries(${PROJECT_NAME} PUBLIC d3d12 dxgi dxguid dinput8)
target_link_libraries(${PROJECT_NAME} PUBLIC assimp::assimp)
target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXTex)
target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXTK12)

