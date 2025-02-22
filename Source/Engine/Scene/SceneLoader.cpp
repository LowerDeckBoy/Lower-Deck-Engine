#include "Core/Logger.hpp"
#include "Graphics/AssetManager.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Scene.hpp"
#include "Scene/Components/NameComponent.hpp"
#include "SceneLoader.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace lde
{
	void SceneLoader::Load(D3D12RHI* pGfx, Scene* pScene, Filepath Path)
	{
		const auto fileExtension = Files::GetExtension(Path);

		std::ifstream f(Path.c_str());
		nlohmann::json json = nlohmann::json::parse(f);

		auto& importer = AssetManager::GetInstance();

		std::string sceneInfoLog = std::format("Loading scene: {0}\n", Path.filename().string());

		for (const auto& record : json["scene"]["models"])
		{
			const auto& name = std::string(record["name"]);
			const auto& path = std::string(record["path"]);

			Model model{};
			
			auto startTime = std::chrono::high_resolution_clock::now();
			
			importer.Import(pGfx, path, model.StaticMeshes);

			auto endTime = std::chrono::high_resolution_clock::now();

			sceneInfoLog.append(std::format("\t- {0}, load time: {1}\n", name, std::chrono::duration<double>(endTime - startTime)));
			model.Create(pGfx, pScene->World());
			model.AddComponent<NameComponent>(name);
			model.Filepath = path;

			pScene->Models.emplace_back(model);
		}

		LOG_INFO(sceneInfoLog.c_str());
		f.close();

	}
} // namespace lde
