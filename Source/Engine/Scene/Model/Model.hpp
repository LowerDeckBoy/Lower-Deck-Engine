#pragma once
#include "../Entity.hpp"
#include "Mesh.hpp"
#include "Meshlet.hpp"
#include "RHI/D3D12/D3D12Buffer.hpp"

namespace lde
{
	namespace RHI
	{
		class D3D12RHI;
		class RHI;
		class Buffer;
	}
	
	class Model : public Entity
	{
	public:
		Model() {}
		//Model(RHI::D3D12RHI* pGfx, std::string_view Filepath, World* pWorld);
		Model(RHI::RHI* pRHI, std::string_view Filepath, World* pWorld);
		~Model();
	
		void Create(RHI::D3D12RHI* pGfx, World* pWorld);
	
		Mesh* GetMesh();

		std::string Filepath;

	private:
		std::unique_ptr<Mesh> m_Mesh;

	};
} // namespace lde
