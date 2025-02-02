#pragma once
#include "../Entity.hpp"
#include "Mesh.hpp"
#include "RHI/D3D12/D3D12Buffer.hpp"

namespace lde
{
	class D3D12RHI;
	class RHI;
	class Buffer;
	
	class Model : public Entity
	{
	public:
		Model() {}
		//Model(D3D12RHI* pGfx, std::string_view Filepath, World* pWorld);
		Model(RHI* pRHI, std::string_view Filepath, World* pWorld);
		~Model();
	
		void Create(D3D12RHI* pGfx, World* pWorld);
	
		Mesh* GetMesh()
		{
			return m_Mesh.get();
		}

		std::string Filepath;

	private:
		std::unique_ptr<Mesh> m_Mesh;

		std::vector<Material> m_Materials;

	};
} // namespace lde
