#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "Scene.h"
#include "Renderer.h"

namespace mau
{
	class SceneManager final
	{
	public:
		SceneManager()
		{
			RegisterScene<Scene_BasicGeometry>("Basic Geometry");
			RegisterScene<Scene_PointLights>("Point Lights");
			RegisterScene<Scene_CookTorrence>("Cook-Torrance");
			RegisterScene<Scene_LambertPhong>("Lambert-Phong");
			RegisterScene<Scene_Triangle>("Triangle");
			RegisterScene<Scene_MeshTest>("Mesh Test");
			RegisterScene<Scene_Reference>("Reference");
			RegisterScene<Scene_Bunny>("Bunny");
			RegisterScene<Scene_SoftShadows>("Soft Shadows");
		}

		~SceneManager() = default;

		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) noexcept = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(SceneManager&&) noexcept = delete;

		void LoadScene(uint8_t index, Renderer* pRenderer)
		{
			if (index >= m_SceneEntries.size())
			{
				return;
			}

			if (index == m_ActiveSceneIndex && m_pActiveScene)
			{
				return;
			}

			m_ActiveSceneIndex = index;
			m_pActiveScene = m_SceneEntries[index].factory();
			m_pActiveScene->Initialize();
			pRenderer->ResetAccumulation();

			std::cout << "Scene -> " << GREEN << m_SceneEntries[index].name << "\n";
			std::cout << RESET;
		}

		[[nodiscard]] Scene* GetActiveScene() const noexcept { return m_pActiveScene.get(); }
		[[nodiscard]] uint8_t GetSceneCount() const noexcept { return static_cast<uint8_t>(m_SceneEntries.size()); }

	private:
		struct SceneEntry
		{
			std::string name;
			std::function<std::unique_ptr<Scene>()> factory;
		};

		template<typename T>
		void RegisterScene(std::string name)
		{
			m_SceneEntries.push_back({ std::move(name), [] { return std::make_unique<T>(); } });
		}

		std::vector<SceneEntry> m_SceneEntries{};
		std::unique_ptr<Scene> m_pActiveScene{};
		uint8_t m_ActiveSceneIndex{ 255 };
	};
}

#endif
