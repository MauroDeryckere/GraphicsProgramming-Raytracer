//External includes
#ifdef ENABLE_VLD
#include "vld.h"
#endif
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>
#include <memory>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#include "SceneManager.h"

using namespace mau;

void PrintInfo();

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* args[])
{

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"RayTracer - Mauro Deryckere",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto const pTimer = std::make_unique<Timer>();
	auto const pRenderer = std::make_unique<Renderer>(pWindow);

	SceneManager sceneManager{};

	PrintInfo();

	sceneManager.LoadScene(2, pRenderer.get()); //Reference scene

	//Start loop
	pTimer->Start();

	float printTimer = 0.f;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_X)
					takeScreenshot = true;

				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ToggleShadows();
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pRenderer->CycleLighMode();
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					pRenderer->CycleSampleMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					pRenderer->DecreaseSamples();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)
				{
					pRenderer->IncreaseSamples();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)
				{
					pRenderer->CycleToneMapMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					pRenderer->ToggleProgressive();
				}

				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					sceneManager.GetActiveScene()->ToggleBVH();
					pRenderer->ResetAccumulation();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pRenderer->CycleBVHDebug();
				}

				//Scene selection (1-9)
				if (e.key.keysym.scancode >= SDL_SCANCODE_1 && e.key.keysym.scancode <= SDL_SCANCODE_9)
				{
					uint8_t const sceneIndex{ static_cast<uint8_t>(e.key.keysym.scancode - SDL_SCANCODE_1) };
					if (sceneIndex < sceneManager.GetSceneCount())
					{
						sceneManager.LoadScene(sceneIndex, pRenderer.get());
					}
				}

				break;
			}
		}

		//--------- Update ---------
		sceneManager.GetActiveScene()->Update(pTimer.get());
		//--------- Render ---------
		pRenderer->Render(sceneManager.GetActiveScene());

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << " | Accumulated: " << pRenderer->GetAccumulatedFrames() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	ShutDown(pWindow);
	return 0;
}

void PrintInfo()
{
	using namespace mau;

	std::cout << YELLOW << "Settings & Keybinds: \n";

	std::cout << "[X]: Screenshot\n";
	std::cout << "[F2]: Toggle Shadows\n";
	std::cout << "[F3]: Cycle Light Mode\n";
	std::cout << "[F4]: Cycle Sample Mode\n";
	std::cout << "[F5]: Decrease Samples Per Frame\n";
	std::cout << "[F6]: Increase Samples Per Frame\n";
	std::cout << "[F7]: Cycle Tone Mapping\n";
	std::cout << "[F8]: Toggle Progressive Rendering\n";
	std::cout << "[F9]: Toggle BVH Acceleration\n";
	std::cout << "[F10]: Cycle BVH Debug (off / leaf colors / AABB wireframe)\n\n";

	std::cout << "[1]: Cook-Torrance\n";
	std::cout << "[2]: Lambert-Phong\n";
	std::cout << "[3]: Reference\n";
	std::cout << "[4]: Bunny\n";
	std::cout << "[5]: Soft Shadows\n\n";

	std::cout << "[WASD]: Move Camera\n";
	std::cout << "[LMB + Drag]: Rotate Camera\n\n";
	std::cout << RESET;
}