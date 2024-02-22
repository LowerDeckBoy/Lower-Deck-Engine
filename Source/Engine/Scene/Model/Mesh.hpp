#pragma once

/*

*/

#include <Core/CoreMinimal.hpp>
#include <DirectXMath.h>
#include <vector>

namespace lde
{

	class D3D12Texture;
	
	using namespace DirectX;
	
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;
		XMFLOAT3 Normal;
		XMFLOAT3 Tangent;
		XMFLOAT3 BiTangent;
	};
	
	struct Material
	{
		int32 BaseColorIndex		= -1;
		int32 NormalIndex			= -1;
		int32 MetalRoughnessIndex	= -1;
		int32 EmissiveIndex			= -1;
	
		float MetallicFactor		= 0.5f;
		float RoughnessFactor		= 0.5f;
		float AlphaCutoff			= 0.5f;
		int32 bDoubleSided			= -1;
	
		XMFLOAT4 BaseColorFactor	= XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		XMFLOAT4 EmissiveFactor		= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct SubMesh
	{
		uint32 MeshID = UINT_MAX;
		Material Mat;
		XMMATRIX Matrix		= XMMatrixIdentity();
		uint32 BaseIndex	= 0;
		uint32 BaseVertex	= 0;
		uint32 IndexCount	= 0;
		uint32 VertexCount	= 0;
	};
	
	struct BoundingBox
	{
		XMFLOAT3 Min;
		XMFLOAT3 Max;
	};
	
	// Treat it like a component
	struct Mesh
	{
		Mesh() {}
		std::vector<SubMesh> SubMeshes;
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
	};
	
	class M
	{
	public:
		
		void ProcessMesh();
		void ProcessMaterials();

	private:
		std::vector<SubMesh> SubMeshes;
		std::vector<Material> Materials;

	};

	struct Node
	{
		Node* Parent = nullptr;
		std::vector<Node*> Children;
		std::string Name;
	
		XMMATRIX Matrix		 = XMMatrixIdentity();
		XMFLOAT3 Translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT4 Rotation	 = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		XMFLOAT3 Scale		 = XMFLOAT3(1.0f, 1.0f, 1.0f);
	};

} // namespace lde
