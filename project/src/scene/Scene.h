#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>

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

		Camera& GetCamera() { return m_Camera; }
		void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
		[[nodiscard]] bool DoesHit(const Ray& ray) const;

		std::vector<Plane> const& GetPlaneGeometries() const { return m_PlaneGeometries; }
		std::vector<Sphere>const& GetSphereGeometries() const { return m_SphereGeometries; }
		std::vector<Light> const& GetLights() const { return m_Lights; }
		std::vector<Material*> const& GetMaterials() const { return m_Materials; }

	protected:
		std::string m_SceneName;

		std::vector<Plane> m_PlaneGeometries{};
		std::vector<Sphere> m_SphereGeometries{};
		std::vector<TriangleMesh> m_TriangleMeshGeometries{};
		std::vector<Light> m_Lights{};
		std::vector<Material*> m_Materials{};

		Camera m_Camera{};

		Sphere* AddSphere(Vector3 const& origin, float radius, unsigned char materialIndex = 0);
		Plane* AddPlane(Vector3 const& origin, Vector3 const& normal, unsigned char materialIndex = 0);
		TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

		Light* AddPointLight(Vector3 const& origin, float intensity, ColorRGB const& color);
		Light* AddAreaLight(Vector3 const& origin, float intensity, ColorRGB const& color, LightShape shape = LightShape::None, float radius = 0.f, std::vector<Vector3> const& vertices = {});
		Light* AddDirectionalLight(Vector3 const& direction, float intensity, ColorRGB const& color);
		unsigned char AddMaterial(Material* pMaterial);
	};

	class Scene_BasicGeometry final : public Scene
	{
	public:
		Scene_BasicGeometry() = default;
		~Scene_BasicGeometry() override = default;

		Scene_BasicGeometry(const Scene_BasicGeometry&) = delete;
		Scene_BasicGeometry(Scene_BasicGeometry&&) noexcept = delete;
		Scene_BasicGeometry& operator=(const Scene_BasicGeometry&) = delete;
		Scene_BasicGeometry& operator=(Scene_BasicGeometry&&) noexcept = delete;

		void Initialize() override;
	};

	class Scene_PointLights final : public Scene
	{
	public:
		Scene_PointLights() = default;
		~Scene_PointLights() override = default;

		Scene_PointLights(const Scene_PointLights&) = delete;
		Scene_PointLights(Scene_PointLights&&) noexcept = delete;
		Scene_PointLights& operator=(const Scene_PointLights&) = delete;
		Scene_PointLights& operator=(Scene_PointLights&&) noexcept = delete;

		void Initialize() override;
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

	class Scene_Triangle final : public Scene
	{
	public:
		Scene_Triangle() = default;
		~Scene_Triangle() override = default;

		Scene_Triangle(const Scene_Triangle&) = delete;
		Scene_Triangle(Scene_Triangle&&) noexcept = delete;
		Scene_Triangle& operator=(const Scene_Triangle&) = delete;
		Scene_Triangle& operator=(Scene_Triangle&&) noexcept = delete;

		void Initialize() override;
		};

	class Scene_MeshTest final : public Scene
	{
	public:
		Scene_MeshTest() = default;
		~Scene_MeshTest() override = default;

		Scene_MeshTest(const Scene_MeshTest&) = delete;
		Scene_MeshTest(Scene_MeshTest&&) noexcept = delete;
		Scene_MeshTest& operator=(const Scene_MeshTest&) = delete;
		Scene_MeshTest& operator=(Scene_MeshTest&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;
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
