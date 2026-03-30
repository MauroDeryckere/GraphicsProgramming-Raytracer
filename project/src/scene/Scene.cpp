#include "Scene.h"
#include "Utils.h"
#include "Material.h"
#include "Light.h"
#include "BVH.h"
#include "DataTypes.h"

#include <ranges>
#include <algorithm>

namespace mau
{
	Scene::Scene()
	{
		m_Materials.emplace_back(std::make_unique<Material_SolidColor>(ColorRGB{ 1, 0, 0 }));

		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	void mau::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		constexpr bool useBVH{ false };

		HitRecord closestHitRecord{ };

		for(auto const& sphere : m_SphereGeometries)
		{
			HitRecord temp{ };
			if (GeometryUtils::HitTest_Sphere(sphere, ray, temp))
			{
				if (temp.t < closestHitRecord.t)
				{
					closestHitRecord = temp;
				}
			}
		}

		for (auto const& plane : m_PlaneGeometries)
		{
			HitRecord temp{ };
			if (GeometryUtils::HitTest_Plane(plane, ray, temp))
			{
				if (temp.t < closestHitRecord.t)
				{
					closestHitRecord = temp;
				}
			}
		}

		if (useBVH)
		{
			for (const auto& mesh : m_TriangleMeshGeometries)
			{
				HitRecord temp{ };
				if (GeometryUtils::HitTest_BVH(ray, mesh, mesh.bvh, 0, temp))
				{
					if (temp.t < closestHitRecord.t)
					{
						closestHitRecord = temp;
					}
				}
			}
		}
		else
		{
			for (auto const& mesh : m_TriangleMeshGeometries)
			{
				HitRecord temp{ };
				if (GeometryUtils::HitTest_TriangleMesh(mesh, ray, temp))
				{
					if (temp.t < closestHitRecord.t)
					{
						closestHitRecord = temp;
					}
				}
			}
		}

		closestHit = closestHitRecord;
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		constexpr bool useBVH{ false };

		for (auto const& sphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(sphere, ray))
			{
				return true;
			}
		}

		for (auto const& plane : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(plane, ray))
			{
				return true;
			}
		}


		if (useBVH)
		{
			for (const auto& mesh : m_TriangleMeshGeometries)
			{
				if (GeometryUtils::HitTest_BVH(ray, mesh, mesh.bvh, 0))
				{
					return true;
				}
			}
		}
		else
		{
			for (auto const& mesh : m_TriangleMeshGeometries)
			{
				if (GeometryUtils::HitTest_TriangleMesh(mesh, ray))
				{
					return true;
				}
			}
		}


		return false;
	}

	Sphere* Scene::AddSphere(const Vector3& origin, float radius, uint8_t materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, uint8_t materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, uint8_t materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddAreaLight(Vector3 const& origin, float intensity, ColorRGB const& color, LightShape shape, float radius, std::vector<Vector3> const& vertices)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Area;

		l.radius = radius;
		l.vertices = vertices;

		l.shape = shape;

		switch (l.shape)
		{
		case LightShape::None:
			assert(l.vertices.size() == 0);
			break;
		case LightShape::Triangular:
			assert(l.vertices.size() == 3);
			break;
		}
		l.vertices.shrink_to_fit();
		auto const a{ l.vertices[1] - l.vertices[0] };
		auto const b{ l.vertices[2] - l.vertices[0] };

		auto const normal{ Vector3::Cross(a, b).Normalized() };
		l.direction = -normal;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		l.vertices.shrink_to_fit();

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	uint8_t Scene::AddMaterial(std::unique_ptr<Material> pMaterial)
	{
		m_Materials.emplace_back(std::move(pMaterial));
		return static_cast<uint8_t>(m_Materials.size() - 1);
	}

	void Scene_BasicGeometry::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		uint8_t constexpr matId_Solid_Red{ 0 };
		uint8_t const matId_Solid_Blue{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Blue)) };

		uint8_t const matId_Solid_Yellow{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Yellow)) };
		uint8_t const matId_Solid_Green { AddMaterial(std::make_unique<Material_SolidColor>(colors::Green)) };
		uint8_t const matId_Solid_Magenta{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Magenta)) };

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Planes
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}

	void Scene_PointLights::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		uint8_t constexpr matId_Solid_Red{ 0 };
		uint8_t const matId_Solid_Blue{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Blue)) };

		uint8_t const matId_Solid_Yellow{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Yellow)) };
		uint8_t const matId_Solid_Green{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Green)) };
		uint8_t const matId_Solid_Magenta{ AddMaterial(std::make_unique<Material_SolidColor>(colors::Magenta)) };

		//Planes
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//Light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}

	void Scene_CookTorrence::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		auto const matCT_GrayRoughMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, 1.f)) };
		auto const matCT_GrayMediumMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, .6f)) };
		auto const matCT_GraySmoothMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, .1f)) };
		auto const matCT_GrayRoughPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, 1.f)) };
		auto const matCT_GrayMediumPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, .6f)) };
		auto const matCT_GraySmoothPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, .1f)) };

		auto const matLambertGrayBlue{ AddMaterial(std::make_unique<Material_Lambert>(ColorRGB{.49f, .57f, .57f}, 1.f)) };

		//Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matLambertGrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambertGrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matLambertGrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matLambertGrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matLambertGrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//Light
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f }); //Backlight
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .80f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { .34f, .47f, .68f }); 
	}

	void Scene_LambertPhong::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		uint8_t const matId_Red{ AddMaterial(std::make_unique<Material_Lambert>(colors::Red, 1.f)) };
		uint8_t const matId_Blue{ AddMaterial(std::make_unique<Material_LambertPhong>(colors::Blue, 1.f, 1.f, 60.f)) };
		uint8_t const matId_Yellow{ AddMaterial(std::make_unique<Material_Lambert>(colors::Yellow, 1.f)) };

		//Spheres
		AddSphere({ -.75f, 1.f, 0.f }, 1.f, matId_Red);
		AddSphere({ .75f, 1.f, 0.f }, 1.f, matId_Blue);

		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matId_Yellow);

		AddPointLight({ 0.f, 5.f, 5.f }, 25.f, colors::White);
		AddPointLight({ 0.f, 2.5f, -5.f }, 25.f, colors::White);
	}

	void Scene_Triangle::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		auto const matLambert_GrayBlue{ AddMaterial(std::make_unique<Material_Lambert>(ColorRGB{.49f, .57f, .57f }, 1.f)) };
		auto const matLambert_White{ AddMaterial(std::make_unique<Material_Lambert>(colors::White, 1.f)) };

		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);

		Triangle const baseTriangle{ {-.75f, 1.5f, 0.f}, {.75f, 0.f, 0.f }, {-.75f, 0.f, 0.f} };
		auto m{ AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White) };
		m->AppendTriangle(baseTriangle, true);
		m->Translate({ 0.f, .5f, 0.f });
		m->UpdateTransforms();

		//Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .80f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { .34f, .47f, .68f });
	}

	void Scene_MeshTest::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		auto const matLambert_GrayBlue{ AddMaterial(std::make_unique<Material_Lambert>(ColorRGB{.49f, .57f, .57f }, 1.f)) };
		auto const matLambert_White{ AddMaterial(std::make_unique<Material_Lambert>(colors::White, 1.f)) };

		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);

		auto pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("resources/simple_object.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);

		pMesh->Scale({ .7f, .7f, .7f });
		pMesh->Translate({ 0.f, 1.f, 0.f });

		pMesh->UpdateTransforms();

		//Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .80f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { .34f, .47f, .68f });
	}

	void Scene_MeshTest::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);
	}

	void Scene_Reference::Initialize()
	{
		m_SceneName = "Reference Scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		auto const matCT_GrayRoughMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, 1.f)) };
		auto const matCT_GrayMediumMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, .6f)) };
		auto const matCT_GraySmoothMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, .1f)) };
		auto const matCT_GrayRoughPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, 1.f)) };
		auto const matCT_GrayMediumPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, .6f)) };
		auto const matCT_GraySmoothPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, .1f)) };

		auto const matLambert_GrayBlue{ AddMaterial(std::make_unique<Material_Lambert>(ColorRGB{.49f, .57f, .57f }, 1.f)) };
		auto const matLambert_White{ AddMaterial(std::make_unique<Material_Lambert>(colors::White, 1.f)) };

		//Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		Triangle const baseTriangle{ {-.75f, 1.5f, 0.f}, {.75f, 0.f, 0.f }, {-.75f, 0.f, 0.f} };
		auto m{ AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White) };
		m->AppendTriangle(baseTriangle, true);
		m->Translate({ -1.75f, 4.5f, 0.f });
		m->UpdateAABB();
		m->UpdateTransforms();

		m = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m->AppendTriangle(baseTriangle, true);
		m->Translate({ 0.f, 4.5f, 0.f });
		m->UpdateAABB();
		m->UpdateTransforms();

		m = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m->AppendTriangle(baseTriangle, true);
		m->Translate({ 1.75f, 4.5f, 0.f });
		m->UpdateAABB();
		m->UpdateTransforms();

		//Lights
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .80f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { .34f, .47f, .68f });
	}

	void Scene_Reference::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		auto const yawAngle{ (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2 };
		for(auto& m : m_TriangleMeshGeometries)
		{
			m.RotateY(yawAngle);
			m.UpdateTransforms();
		}
	}

	void Scene_Bunny::Initialize()
	{
		m_SceneName = "Reference Scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		auto const matLambert_GrayBlue{ AddMaterial(std::make_unique<Material_Lambert>(ColorRGB{.49f, .57f, .57f }, 1.f)) };
		auto const matLambert_White{ AddMaterial(std::make_unique<Material_Lambert>(colors::White, 1.f)) };

		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);

		//Smallest possible tests for the BVH
		//Triangle const baseTriangle{ {-.75f, 5.5f, 0.f}, {.75f, 4.5f, 0.f }, {-.75f, 4.5f, 0.f} };
		//auto m{ AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White) };
		//m->AppendTriangle(baseTriangle, true);
		//m->UpdateTransforms(true);
		//m->InitializeBVH();

		//Triangle const baseTriangle{ {-.75f, 5.5f, 0.f}, {.75f, 4.5f, 0.f }, {-.75f, 4.5f, 0.f} };
		//Triangle const baseTriangle2{ {-.75f, 3.5f, 0.f}, {.75f, 2.5f, 0.f }, {-.75f, 2.5f, 0.f} };
		//Triangle const baseTriangle3{ {-.75f, 1.5f, 0.f}, {.75f, .5f, 0.f }, {-.75f, .5f, 0.f} };
		//auto m{ AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White) };
		//m->AppendTriangle(baseTriangle, true);
		//m->AppendTriangle(baseTriangle2, true);
		//m->AppendTriangle(baseTriangle3, true);
		//m->UpdateTransforms(true);
		//m->InitializeBVH();

		auto pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("resources/lowpoly_bunny.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);

		pMesh->Scale({ 2.f, 2.f, 2.f });
		pMesh->RotateY(TO_RADIANS * 180.f);

		pMesh->UpdateAABB();
		pMesh->UpdateTransforms(true);
		pMesh->InitializeBVH();


		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, { 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, { 1.f, .80f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, { .34f, .47f, .68f });
	}

	void Scene_SoftShadows::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		auto const matCT_GrayRoughMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, 1.f)) };
		auto const matCT_GrayMediumMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, .6f)) };
		auto const matCT_GraySmoothMetal{ AddMaterial(std::make_unique<Material_CookTorrence>({.972f, .960f, .915f}, 1.f, .1f)) };
		auto const matCT_GrayRoughPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, 1.f)) };
		auto const matCT_GrayMediumPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, .6f)) };
		auto const matCT_GraySmoothPlastic{ AddMaterial(std::make_unique<Material_CookTorrence>({.75f, .75f, .75f}, 0.f, .1f)) };

		auto const matLambert_GrayBlue{ AddMaterial(std::make_unique<Material_Lambert>(ColorRGB{.49f, .57f, .57f }, 1.f)) };

		//Planes
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		AddAreaLight({}, 1000.f , { .45f, 1.f, .45f }, LightShape::Triangular, 0.f, { {-1.75, 0.5f, -6.f}, { 0.f, 5.f, -5.f}, {1.75f, 0.5f, -5.f} });
	}

}
