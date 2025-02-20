#pragma once

#include "Core/CoreMinimal.hpp"
#include "RHI/D3D12/D3D12Buffer.hpp"
#include "RHI/D3D12/D3D12Device.hpp"
#include <DirectXMath.h>
#include <vector>

namespace lde
{
	class D3D12Texture;
	
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 Bitangent;
	};
	
	struct Material
	{
		uint32 BaseColorIndex		= (uint32)-1;
		uint32 NormalIndex			= (uint32)-1;
		uint32 MetalRoughnessIndex	= (uint32)-1;
		uint32 EmissiveIndex		= (uint32)-1;
	
		float MetallicFactor		= 0.0f;
		float RoughnessFactor		= 0.5f;
		float AlphaCutoff			= 0.5f;
		int32 bDoubleSided			= -1;
	
		DirectX::XMFLOAT4 BaseColorFactor	= DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		DirectX::XMFLOAT4 EmissiveFactor	= DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct BoundingBox
	{
		DirectX::XMFLOAT3 Min = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 Max = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	};
	
	struct StaticMesh
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView{};

		Material Material{};

		BoundingBox AABB;

		BufferHandle VertexBuffer = UINT32_MAX;
		BufferHandle IndexBuffer = UINT32_MAX;

		uint32 NumVertices;
		uint32 NumIndices;
	};

} // namespace lde
