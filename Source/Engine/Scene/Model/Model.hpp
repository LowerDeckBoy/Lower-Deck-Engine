#pragma once
#include "Mesh.hpp"
#include "../Entity.hpp"
#include "RHI/D3D12/D3D12Buffer.hpp"


namespace lde
{
	class D3D12Context;
	
	class Model : public Entity
	{
	public:
		Model() {}
		Model(RHI::D3D12Context* pGfx, std::string_view Filepath, World* pWorld);
		~Model();
	
		void Create(RHI::D3D12Context* pGfx, World* pWorld);
	
		std::unique_ptr<Mesh> ModelMesh;
	
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
	
		std::string Filepath;
	
		//RHI::Buffer* VertexBuffer = nullptr;
		//RHI::Buffer* IndexBuffer = nullptr;
		RHI::D3D12Buffer* VertexBuffer = nullptr;
		RHI::D3D12Buffer* IndexBuffer = nullptr;
	
		RHI::D3D12ConstantBuffer* ConstBuffer = nullptr;
		RHI::cbPerObject cbData{};
	
	private:
		
	};

} // namespace lde
