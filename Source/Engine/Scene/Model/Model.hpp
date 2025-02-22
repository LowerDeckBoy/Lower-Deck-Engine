#pragma once

#include "../Entity.hpp"
#include "Mesh.hpp"
#include "RHI/D3D12/D3D12Buffer.hpp"

namespace lde
{
	class D3D12RHI;
	class Buffer;
	
	class Model : public Entity
	{
	public:
		Model() = default;
		~Model() = default;
	
		void Create(D3D12RHI* pGfx, World* pWorld);
	
		BufferHandle ConstBuffer = UINT32_MAX;
		cbPerObject cbData{};

		std::vector<StaticMesh> StaticMeshes;

		std::string Filepath;

	private:

	};
} // namespace lde
