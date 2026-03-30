#ifndef BRDFS_H
#define BRDFS_H

#include "Maths.h"

namespace mau
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			return cd * kd / PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return cd * kd / PI;
		}

		/**
		 * \brief
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			auto const phong{ ks * std::pow(Vector3::Dot(Vector3::Reflect(n, l), v), exp) };
			return {phong, phong, phong};
		}	

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			float const oneMinusDot{ 1.f - Vector3::Dot(h, v) };
			float const omd2{ oneMinusDot * oneMinusDot };
			float const omd5{ omd2 * omd2 * oneMinusDot };
			return f0 + (1.f - f0) * omd5;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			float const a2{ roughness * roughness * roughness * roughness }; //a^4 = (roughness^2)^2
			float const nDotH{ Vector3::Dot(n, h) };
			float const denom{ nDotH * nDotH * (a2 - 1.f) + 1.f };
			return a2 / (PI * denom * denom);
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(float nDotV, float kDirect)
		{
			return nDotV / (nDotV * (1.f - kDirect) + kDirect);
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			float const a{ roughness * roughness };
			float const kDirect{ ((a + 1.f) * (a + 1.f)) / 8.f };
			return GeometryFunction_SchlickGGX(Vector3::Dot(n, v), kDirect)
				 * GeometryFunction_SchlickGGX(Vector3::Dot(n, l), kDirect);
		}

		static float GeometryFunction_Smith(float nDotV, float nDotL, float roughness)
		{
			float const a{ roughness * roughness };
			float const kDirect{ ((a + 1.f) * (a + 1.f)) / 8.f };
			return GeometryFunction_SchlickGGX(nDotV, kDirect) * GeometryFunction_SchlickGGX(nDotL, kDirect);
		}

	}
}

#endif