#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <memory>

#include "Maths.h"
#include "DataTypes.h"
#include "Light.h"
#include "Camera.h"

namespace mau
{
	//Forward Declarations
	class Timer;
	class Material;
	struct Plane;
	struct Sphere;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(mau::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		Camera& GetCamera() noexcept { return m_Camera; }
		const Camera& GetCamera() const noexcept { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		[[nodiscard]] bool DoesHit(const Ray& ray) const;

		[[nodiscard]] bool IsDirty() const noexcept { return m_IsDirty; }

		std::vector<Plane> const& GetPlaneGeometries() const { return m_PlaneGeometries; }
		std::vector<Sphere>const& GetSphereGeometries() const { return m_SphereGeometries; }
		std::vector<Light> const& GetLights() const { return m_Lights; }
		[[nodiscard]] Material* GetMaterial(uint8_t index) const { return m_Materials[index].get(); }

	protected:
		std::string m_SceneName;

		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Light> m_Lights{};
		std::vector<std::unique_ptr<Material>> m_Materials{};

		Camera m_Camera{};
		bool m_IsDirty{ false };

		Sphere* AddSphere(Vector3 const& origin, float radius, uint8_t materialIndex = 0);
		Plane* AddPlane(Vector3 const& origin, Vector3 const& normal, uint8_t materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, uint8_t materialIndex = 0);

		Light* AddPointLight(Vector3 const& origin, float intensity, ColorRGB const& color);
		Light* AddAreaLight(Vector3 const& origin, float intensity, ColorRGB const& color, LightShape shape = LightShape::None, float radius = 0.f, std::vector<Vector3> const& vertices = {});
		Light* AddDirectionalLight(Vector3 const& direction, float intensity, ColorRGB const& color);
		uint8_t AddMaterial(std::unique_ptr<Material> pMaterial);
	};

	class Scene_CookTorrence final : public Scene
	{
	public:
		Scene_CookTorrence() = default;
		~Scene_CookTorrence() override = default;

		Scene_CookTorrence(const Scene_CookTorrence&) = delete;
		Scene_CookTorrence(Scene_CookTorrence&&) noexcept = delete;
		Scene_CookTorrence& operator=(const Scene_CookTorrence&) = delete;
		Scene_CookTorrence& operator=(Scene_CookTorrence&&) noexcept = delete;

		void Initialize() override;
	};

	class Scene_LambertPhong final : public Scene
	{
	public:
		Scene_LambertPhong() = default;
		~Scene_LambertPhong() override = default;

		Scene_LambertPhong(const Scene_LambertPhong&) = delete;
		Scene_LambertPhong(Scene_LambertPhong&&) noexcept = delete;
		Scene_LambertPhong& operator=(const Scene_LambertPhong&) = delete;
		Scene_LambertPhong& operator=(Scene_LambertPhong&&) noexcept = delete;

		void Initialize() override;
	};

	class Scene_Reference final : public Scene
	{
	public:
		Scene_Reference() = default;
		~Scene_Reference() override = default;

		Scene_Reference(const Scene_Reference&) = delete;
		Scene_Reference(Scene_Reference&&) noexcept = delete;
		Scene_Reference& operator=(const Scene_Reference&) = delete;
		Scene_Reference& operator=(Scene_Reference&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;
	};

	class Scene_Bunny final : public Scene
	{
	public:
		Scene_Bunny() = default;
		~Scene_Bunny() override = default;

		Scene_Bunny(const Scene_Bunny&) = delete;
		Scene_Bunny(Scene_Bunny&&) noexcept = delete;
		Scene_Bunny& operator=(const Scene_Bunny&) = delete;
		Scene_Bunny& operator=(Scene_Bunny&&) noexcept = delete;

		void Initialize() override;
	};

	class Scene_SoftShadows final : public Scene
	{
	public:
		Scene_SoftShadows() = default;
		~Scene_SoftShadows() override = default;

		Scene_SoftShadows(const Scene_SoftShadows&) = delete;
		Scene_SoftShadows(Scene_SoftShadows&&) noexcept = delete;
		Scene_SoftShadows& operator=(const Scene_SoftShadows&) = delete;
		Scene_SoftShadows& operator=(Scene_SoftShadows&&) noexcept = delete;

		void Initialize() override;
	};

}

#endif
