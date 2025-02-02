#pragma once

#include "RHI/D3D12/D3D12Buffer.hpp"
#include "RHI/D3D12/D3D12Device.hpp"
#include <Core/CoreMinimal.hpp>
#include <DirectXMath.h>
#include <vector>

namespace lde
{
	class D3D12Texture;
	
	using namespace DirectX;
	
	struct Vertex
	{
		XMFLOAT3 Position	= XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT2 TexCoord	= XMFLOAT2(0.0f, 0.0f);
		XMFLOAT3 Normal		= XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 Tangent	= XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 Bitangent	= XMFLOAT3(0.0f, 0.0f, 0.0f);
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
	
		XMFLOAT4 BaseColorFactor	= XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		XMFLOAT4 EmissiveFactor		= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct BoundingBox
	{
		XMFLOAT3 Min = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 Max = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	struct Submesh
	{
		Material	Material;

		uint32		BaseIndex	= 0;
		uint32		BaseVertex	= 0;
		uint32		IndexCount	= 0;
		uint32		VertexCount	= 0;

		BoundingBox AABB;
	};
	
	struct Mesh
	{
		Mesh() = default;
		std::string				Name = "";
		std::vector<Submesh>	Submeshes;
		std::vector<Vertex>		Vertices;
		std::vector<uint32>		Indices;

		BufferHandle VertexBuffer = UINT32_MAX;
		BufferHandle IndexBuffer  = UINT32_MAX;
		BufferHandle ConstBuffer  = UINT32_MAX;

		cbPerObject cbData{};
		D3D12_INDEX_BUFFER_VIEW IndexView{};

		void Create(class D3D12Device* pDevice)
		{
			VertexBuffer = pDevice->CreateBuffer(
				BufferDesc{
					BufferUsage::eStructured,
					Vertices.data(),
					static_cast<uint32>(Vertices.size()),
					Vertices.size() * sizeof(Vertices.at(0)),
					static_cast<uint32>(sizeof(Vertices.at(0))),
					true
				});

			IndexBuffer = pDevice->CreateBuffer(
				BufferDesc{
					BufferUsage::eIndex,
					Indices.data(),
					static_cast<uint32>(Indices.size()),
					Indices.size() * sizeof(Indices.at(0)),
					static_cast<uint32>(sizeof(Indices.at(0)))
				});
		}
	};
	
	struct Node
	{
		Node* Parent = nullptr;
		std::vector<Node*> Children;

		Mesh* Mesh = nullptr;

		std::string Name = "";
	
		XMMATRIX Matrix		 = XMMatrixIdentity();
		XMFLOAT3 Translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT4 Rotation	 = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		XMFLOAT3 Scale		 = XMFLOAT3(1.0f, 1.0f, 1.0f);
	};

} // namespace lde
