#pragma once

#include "Core/FileSystem.hpp"

namespace lde
{
	class Scene;
	class D3D12RHI;

	class SceneLoader
	{
	public:
		SceneLoader() = default;
		~SceneLoader() = default;

		static void Load(D3D12RHI* pGfx, Scene* pScene, Filepath Path);

	private:

	};
} // namespace lde
