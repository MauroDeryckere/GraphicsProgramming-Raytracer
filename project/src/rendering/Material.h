#ifndef MATERIAL_H
#define MATERIAL_H

#include "Maths.h"
#include "DataTypes.h"
#include "BRDFs.h"

#include <cassert>

namespace mau
{
#pragma region Material BASE
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		/**
		 * \brief Function used to calculate the correct color for the specific material and its parameters
		 * \param hitRecord current hitrecord
		 * \param l light direction
		 * \param v view direction
		 * \return color
		 */
		virtual ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) = 0;
	};
#pragma endregion

#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(const ColorRGB& color) : m_Color(color){ }

		ColorRGB Shade(const HitRecord& hitRecord, const Vector3& l, const Vector3& v) override
		{
			return m_Color;
		}

	private:
		ColorRGB m_Color{ colors::White };
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(const ColorRGB& diffuseColor, float diffuseReflectance) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(diffuseReflectance)
		{
			assert(diffuseReflectance <= 1.f && diffuseReflectance >= 0.f);
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor);
		}

	private:
		ColorRGB m_DiffuseColor{ colors::White };
		float m_DiffuseReflectance{ 1.f }; //kd
	};
#pragma endregion

#pragma region Material LAMBERT PHONG
	//LAMBERT-PHONG
	//=============
	class Material_LambertPhong final : public Material
	{
	public:
		Material_LambertPhong(const ColorRGB& diffuseColor, float kd, float ks, float phongExponent) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(kd), m_SpecularReflectance(ks),
			m_PhongExponent(phongExponent)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor)
				 + BRDF::Phong(m_SpecularReflectance, m_PhongExponent, l, v, hitRecord.normal);
		}

	private:
		ColorRGB m_DiffuseColor{ colors::White };
		float m_DiffuseReflectance{ 0.5f }; //kd
		float m_SpecularReflectance{ 0.5f }; //ks
		float m_PhongExponent{ 1.f }; //Phong Exponent
	};
#pragma endregion

#pragma region Material COOK TORRENCE
	//COOK TORRENCE
	class Material_CookTorrence final : public Material
	{
	public:
		Material_CookTorrence(const ColorRGB& albedo, float metalness, float roughness) :
			m_Albedo(albedo), m_Metalness(metalness), m_Roughness(roughness),
			m_F0{ (metalness == 0.f) ? ColorRGB{ 0.04f, 0.04f, 0.04f } : albedo }
		{
			assert(m_Metalness == 1.f || m_Metalness == 0.f);
			assert(m_Roughness != 0.f);
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			Vector3 const h{ (v + l).Normalized() };

			float const nDotV{ Vector3::Dot(hitRecord.normal, v) };
			float const nDotL{ Vector3::Dot(hitRecord.normal, l) };

			auto const F{ BRDF::FresnelFunction_Schlick(h, v, m_F0) };
			auto const D{ BRDF::NormalDistribution_GGX(hitRecord.normal, h, m_Roughness) };
			auto const G{ BRDF::GeometryFunction_Smith(nDotV, nDotL, m_Roughness) };

			auto const specular{ (D * F * G) / (4.f * nDotV * nDotL) };
			auto const diffuse{ (m_Metalness == 0.f) ? BRDF::Lambert(ColorRGB{1.f,1.f,1.f} - F, m_Albedo)
															 : BRDF::Lambert(0.f, m_Albedo) };

			return diffuse + specular;
		}

	private:
		ColorRGB m_Albedo{ 0.955f, 0.637f, 0.538f }; //Copper
		float m_Metalness{ 1.0f };
		float m_Roughness{ 0.1f }; // [1.0 > 0.0] >> [ROUGH > SMOOTH]
		ColorRGB m_F0{ 0.04f, 0.04f, 0.04f }; //Precomputed base reflectivity
	};
#pragma endregion
}

#endif
