//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Light.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <algorithm>
#include <execution>


using namespace mau;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_Pixels.resize(m_Width * m_Height);
	std::iota(m_Pixels.begin(), m_Pixels.end(), 0);

	m_AccumulationBuffer.resize(m_Width * m_Height);
}

void Renderer::Render(Scene* pScene)
{
	Camera& camera{ pScene->GetCamera() };
	auto const& lights { pScene->GetLights() };

	if (m_ProgressiveEnabled && (camera.IsDirty() || pScene->IsDirty()))
	{
		ResetAccumulation();
	}

	if (m_ProgressiveEnabled)
	{
		++m_AccumulatedFrames;
	}

	float const aspectRatio{ m_Width / static_cast<float>(m_Height) };
	float const fov{ tan(camera.fovAngle * TO_RADIANS/2) };

	Matrix const cameraToWorld{ camera.CalculateCameraToWorld() };

	std::for_each(std::execution::par_unseq, m_Pixels.begin(), m_Pixels.end(), [&](int const idx)
	{
		ColorRGB frameColor{ };

		int const px{ idx % m_Width };
		int const py{ idx / m_Width };

		for (uint32_t currSample{ 0 }; currSample < m_SampleCount; ++currSample)
		{
			auto const offset{ SampleRay(currSample) };

			float const x{ ((2 * (px + .5f + offset.x) / static_cast<float>(m_Width) - 1) * aspectRatio * fov) };
			float const y{ ((1 - 2 * (py + .5f + offset.y) / static_cast<float>(m_Height)) * fov) };

			Vector3 const dirViewSpace{ x , y, 1.f };
			Vector3 const dirWorldSpace{ (cameraToWorld.TransformVector(dirViewSpace)).Normalized() };

			Ray const viewRay{ cameraToWorld.GetTranslation() , dirWorldSpace };

			uint32_t bvhLeafNodeIdx{ std::numeric_limits<uint32_t>::max() };
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit, bvhLeafNodeIdx);

			if (closestHit.didHit)
			{
				switch (m_CurrBVHDebugMode)
				{
				case BVHDebugMode::LeafColor:
				{
					if (bvhLeafNodeIdx != std::numeric_limits<uint32_t>::max())
					{
						float const hue{ std::fmod(bvhLeafNodeIdx * 0.618033988f, 1.f) };
						float const h6{ hue * 6.f };
						float const f{ h6 - std::floor(h6) };
						float const p{ 0.9f * 0.2f };
						float const q{ 0.9f * (1.f - 0.8f * f) };
						float const t{ 0.9f * (1.f - 0.8f * (1.f - f)) };

						int const hi{ static_cast<int>(h6) % 6 };
						switch (hi)
						{
						case 0: frameColor += ColorRGB{ 0.9f, t, p }; break;
						case 1: frameColor += ColorRGB{ q, 0.9f, p }; break;
						case 2: frameColor += ColorRGB{ p, 0.9f, t }; break;
						case 3: frameColor += ColorRGB{ p, q, 0.9f }; break;
						case 4: frameColor += ColorRGB{ t, p, 0.9f }; break;
						case 5: frameColor += ColorRGB{ 0.9f, p, q }; break;
						default: break;
						}
					}
					else
					{
						frameColor += ColorRGB{ 0.15f, 0.15f, 0.15f };
					}
					break;
				}
				case BVHDebugMode::Wireframe:
				case BVHDebugMode::Off:
				default:
					for (auto const& light : lights)
					{
						frameColor += CalculateIllumination(pScene, light, closestHit, viewRay.direction);
					}
					break;
				}
			}

			//AABB wireframe overlay: draw on top of scene
			if (m_CurrBVHDebugMode == BVHDebugMode::Wireframe)
			{
				for (auto const& mesh : pScene->GetTriangleMeshGeometries())
				{
					float wireT{};
					uint32_t wireDepth{};
					if (GeometryUtils::TraceAABBWireframes(viewRay, mesh, wireT, wireDepth))
					{
						//Green wireframe, brighter for shallower nodes
						float const brightness{ 1.f - wireDepth * 0.08f };
						frameColor = ColorRGB{ 0.f, std::max(brightness, 0.2f), 0.f };
					}
				}
			}
		}

		BoxFilter(frameColor);

		ColorRGB displayColor{};
		if (m_ProgressiveEnabled)
		{
			m_AccumulationBuffer[idx] += frameColor;
			displayColor = m_AccumulationBuffer[idx] / static_cast<float>(m_AccumulatedFrames);
		}
		else
		{
			displayColor = frameColor;
		}

		switch (m_CurrToneMapMode)
		{
		case ToneMapMode::ReinhardJodie:
			ReinhardJolieToneMap(displayColor);
			break;
		case ToneMapMode::ACES:
			ACESAproxToneMap(displayColor);
			break;
		case ToneMapMode::None:
		default:
			displayColor.MaxToOne();
			break;
		}

		m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
			static_cast<uint8_t>(displayColor.r * 255),
			static_cast<uint8_t>(displayColor.g * 255),
			static_cast<uint8_t>(displayColor.b * 255));

	});

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

ColorRGB mau::Renderer::CalculateIllumination(Scene* pScene, const Light& light, const HitRecord& closestHit, const Vector3& viewDir) const noexcept
{
	uint32_t hits{ 0 };

	float observedArea{ };
	ColorRGB radiance{ };

	ColorRGB shade{ };

	bool const hasNoSoftShadows{ !light.HasSoftShadows() };
	if (hasNoSoftShadows)
	{
		auto const dirToLight{ GetDirectionToLight(light, light.origin, closestHit.origin) };
		Ray const shadowRay{ closestHit.origin, dirToLight.first, 0.001f, dirToLight.second };

		if (!m_ShadowsEnabled || !pScene->DoesHit(shadowRay))
		{
			auto const o{ GetObservedArea(light, dirToLight.first, closestHit.normal) };
			if (o <= 0.f)
			{
				return {};
			}

			observedArea = o;
			radiance = GetRadiance(light, light.origin, closestHit);
			shade = pScene->GetMaterial(closestHit.materialIndex)->Shade(closestHit, dirToLight.first, -viewDir);
		}
	}
	else
	{
		for (uint32_t sample{ 0 }; sample < m_LightSamples; ++sample)
		{
			switch (light.shape)
			{
			case LightShape::None:
				continue;

			case LightShape::Triangular:
			{
				auto const pointOnTriangle{ GeometryUtils::GetRandomTriangleSample(light.vertices[0], light.vertices[1], light.vertices[2])};

				auto const dirToLight{ GetDirectionToLight(light, pointOnTriangle, closestHit.origin) };

				if (m_ShadowsEnabled)
				{
					Ray const shadowRay{ closestHit.origin, dirToLight.first, 0.001f, dirToLight.second };
					if (pScene->DoesHit(shadowRay))
					{
						++hits;
						continue;
					}
				}

				auto const o{ GetObservedArea(light, dirToLight.first, closestHit.normal) };
				if (o > 0.f)
				{
					observedArea += o;
					radiance += GetRadiance(light, pointOnTriangle, closestHit);
					shade += pScene->GetMaterial(closestHit.materialIndex)->Shade(closestHit, dirToLight.first, -viewDir);
				}

				break;
			}
			}
		}

		if (m_LightSamples > hits)
		{
			observedArea /= static_cast<float>(m_LightSamples);
			radiance /= static_cast<float>(m_LightSamples);
			shade /= static_cast<float>(m_LightSamples);
		}
	}

	float const illuminationFactor{ !m_ShadowsEnabled || hasNoSoftShadows ? 1.f : 1.f - (static_cast<float>(hits) / static_cast<float>(m_LightSamples)) };

	switch (m_CurrLightMode)
	{
	case LightMode::ObservedArea:
	{
		return { illuminationFactor * observedArea, illuminationFactor * observedArea, illuminationFactor * observedArea };
	}
	case LightMode::Radiance:
	{
		return illuminationFactor * radiance;
	}
	case LightMode::BRDF:
	{
		return illuminationFactor * shade;
	}
	case LightMode::Combined:
	{
		return illuminationFactor * radiance * shade * observedArea;
	}
	default:
		break;
	}

	return {};
}

Vector3 mau::Renderer::SampleRay(uint32_t currSample) const noexcept
{
	switch(m_CurrSampleMode)
	{
	case SampleMode::RandomSquare:
		return SampleRandomSquare();
	case SampleMode::UniformSquare:
		if (m_SampleCount == 1)
		{
			return {};
		}

		return SampleUniformSquare(currSample);
	default: 
		return {};
	}
}

Vector3 mau::Renderer::SampleRandomSquare() const noexcept
{
	return {Random(0.f, 1.f) - .5f, Random(0.f, 1.f) - .5f, 0.f};
}

Vector3 mau::Renderer::SampleUniformSquare(uint32_t currSample) const noexcept
{

	uint32_t gridSize{ static_cast<uint32_t>(std::sqrt(m_SampleCount)) };

	if (gridSize * gridSize < m_SampleCount)
	{
		++gridSize;
	}

	float const subpixelWidth{ 1.0f / gridSize };
	float const subpixelHeight{ 1.0f / gridSize };

	uint32_t const sampleX{ currSample % gridSize };
	uint32_t const sampleY { currSample / gridSize };

	if (m_SampleCount == 2) //When there are only 2 samples, just do the samples on the center line
	{
		return Vector3{ (currSample * subpixelWidth) + (0.5f * subpixelWidth) - .5f, 0.5f, 0.0f };
	}

	return Vector3
	{
		(sampleX * subpixelWidth) + (0.5f * subpixelWidth) - .5f,
		(sampleY * subpixelHeight) + (0.5f * subpixelHeight) - .5f,
		0.0f 
	};
}

void mau::Renderer::BoxFilter(ColorRGB& c) const noexcept
{
	c /= m_SampleCount;
}

void mau::Renderer::ResetAccumulation() noexcept
{
	std::fill(m_AccumulationBuffer.begin(), m_AccumulationBuffer.end(), ColorRGB{});
	m_AccumulatedFrames = 0;
}
