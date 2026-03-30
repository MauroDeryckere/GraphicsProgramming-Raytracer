#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <vector>
#include <iostream>

#include "ColorRGB.h"

struct SDL_Window;
struct SDL_Surface;

namespace mau
{
	// Console colors
	inline constexpr char const* RESET  = "\033[0m";
	inline constexpr char const* RED    = "\033[31m";
	inline constexpr char const* GREEN  = "\033[32m";
	inline constexpr char const* YELLOW = "\033[33m";

	class Vector3;
	class Scene;
	struct Light;
	struct HitRecord;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene);
		bool SaveBufferToImage() const;

		uint32_t GetAccumulatedFrames() const noexcept { return m_AccumulatedFrames; }
		void ResetAccumulation() noexcept;

		void CycleLighMode() noexcept
		{
			auto curr{ static_cast<uint8_t>(m_CurrLightMode) };
			++curr %= static_cast<uint8_t>(LightMode::COUNT);

			m_CurrLightMode = static_cast<LightMode>(curr);
			ResetAccumulation();

			std::cout << "Light mode -> " << GREEN;
			switch (m_CurrLightMode)
			{
			case LightMode::ObservedArea: std::cout << "ObservedArea\n"; break;
			case LightMode::Radiance:     std::cout << "Radiance\n"; break;
			case LightMode::BRDF:         std::cout << "BRDF\n"; break;
			case LightMode::Combined:     std::cout << "Combined\n"; break;
			default: break;
			}
			std::cout << RESET;
		}

		void ToggleShadows() noexcept
		{
			m_ShadowsEnabled = !m_ShadowsEnabled;
			ResetAccumulation();
			std::cout << "Shadows -> " << (m_ShadowsEnabled ? GREEN : RED) << (m_ShadowsEnabled ? "Enabled" : "Disabled") << "\n";
			std::cout << RESET;
		}

		void CycleSampleMode() noexcept
		{
			auto curr{ static_cast<uint8_t>(m_CurrSampleMode) };
			++curr %= static_cast<uint8_t>(SampleMode::COUNT);

			m_CurrSampleMode = static_cast<SampleMode>(curr);
			ResetAccumulation();

			std::cout << "Sample mode -> " << GREEN;
			switch (m_CurrSampleMode)
			{
			case SampleMode::RandomSquare:  std::cout << "Random\n"; break;
			case SampleMode::UniformSquare: std::cout << "Uniform\n"; break;
			default: break;
			}
			std::cout << RESET;
		}

		void CycleToneMapMode() noexcept
		{
			auto curr{ static_cast<uint8_t>(m_CurrToneMapMode) };
			++curr %= static_cast<uint8_t>(ToneMapMode::COUNT);

			m_CurrToneMapMode = static_cast<ToneMapMode>(curr);
			ResetAccumulation();

			std::cout << "Tone mapping -> " << GREEN;
			switch (m_CurrToneMapMode)
			{
			case ToneMapMode::None:          std::cout << "None\n"; break;
			case ToneMapMode::ReinhardJodie: std::cout << "Reinhard Jodie\n"; break;
			case ToneMapMode::ACES:          std::cout << "ACES\n"; break;
			default: break;
			}
			std::cout << RESET;
		}

		void ToggleProgressive() noexcept
		{
			m_ProgressiveEnabled = !m_ProgressiveEnabled;
			m_SampleCount = 1;
			ResetAccumulation();
			std::cout << "Progressive rendering -> " << (m_ProgressiveEnabled ? GREEN : RED) << (m_ProgressiveEnabled ? "Enabled" : "Disabled") << "\n";
			std::cout << RESET;
		}

		void IncreaseSamples() noexcept
		{
			m_SampleCount *= 2;
			ResetAccumulation();
			std::cout << "Samples per " << (m_ProgressiveEnabled ? "frame (accumulating)" : "pixel") << " -> " << GREEN << m_SampleCount << "\n";
			std::cout << RESET;
		}
		void DecreaseSamples() noexcept
		{
			m_SampleCount = std::max<uint32_t>(m_SampleCount / 2, 1);
			ResetAccumulation();
			std::cout << "Samples per " << (m_ProgressiveEnabled ? "frame (accumulating)" : "pixel") << " -> " << GREEN << m_SampleCount << "\n";
			std::cout << RESET;
		}

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		//Has to be updated when window is resized (not possible at the moment)
		std::vector<uint32_t> m_Pixels{ };

		enum class LightMode : uint8_t
		{
			ObservedArea, //Lambert cosine law
			Radiance, //Indicent Radiance
			BRDF, //Scattering of the light
			Combined, //ObservedArea * Radiance * BRDF
			COUNT
		}; 
		LightMode m_CurrLightMode{ LightMode::Combined }; //Cycle through with F3
		bool m_ShadowsEnabled{ true }; //Switched on/off with F2

		enum class SampleMode : uint8_t
		{
			RandomSquare,
			UniformSquare,
			COUNT
		};
		SampleMode m_CurrSampleMode{ SampleMode::UniformSquare }; //Cycle through with F4
		uint32_t m_SampleCount{ 1 }; //Samples per pixel; Decrease with F5, Increase with F6
		uint32_t m_LightSamples{ 10 }; //Samples per light (if applicable)

		enum class ToneMapMode : uint8_t
		{
			None,
			ReinhardJodie,
			ACES,
			COUNT
		};
		ToneMapMode m_CurrToneMapMode{ ToneMapMode::None }; //Cycle through with F7

		bool m_ProgressiveEnabled{ true };
		std::vector<ColorRGB> m_AccumulationBuffer{};
		uint32_t m_AccumulatedFrames{ 0 };

		[[nodiscard]] ColorRGB CalculateIllumination(Scene* pScene, const Light& light, const HitRecord& closestHit, const Vector3& viewDir) const noexcept;

		Vector3 SampleRay(uint32_t currSample) const noexcept;

		Vector3 SampleRandomSquare() const noexcept;
		Vector3 SampleUniformSquare(uint32_t currSample) const noexcept;

		void BoxFilter(ColorRGB& c) const noexcept;
	};
}

#endif
