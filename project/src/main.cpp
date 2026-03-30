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
#include "Scene.h"

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

	//auto const pScene = std::make_unique<Scene_BasicGeometry>();
	//auto const pScene = std::make_unique<Scene_PointLights>();
	//auto const pScene = std::make_unique<Scene_CookTorrence>();
	//auto const pScene = std::make_unique<Scene_LambertPhong>();

	//auto const pScene = std::make_unique<Scene_Triangle>();
	//auto const pScene = std::make_unique<Scene_MeshTest>();

	auto const pScene = std::make_unique<Scene_Reference>();
	//auto const pScene = std::make_unique<Scene_Bunny>();

	//auto const pScene = std::make_unique<Scene_SoftShadows>();

	PrintInfo();

	pScene->Initialize();

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

				break;
			}
		}

		//--------- Update ---------
		pScene->Update(pTimer.get());
		//--------- Render ---------
		pRenderer->Render(pScene.get());

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
	std::cout << "Raytracer project Mauro Deryckere\n";
	std::cout << "Keybinds: \n";
	std::cout << "X: Screenshot\nF2: Shadows on/off\nF3: Cycle light mode\nF4: Cycle sample mode\nF5: Decrease samples\nF6: Increase samples\nF7: Cycle tone mapping\n\n";
	std::cout << "WASD: Move camera\nHold LMB and move: rotate camera\n\n";
}