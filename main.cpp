#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <stdlib.h>
#include <string>

// = Direct X =
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

// = SDL =
#define SDL_MAIN_HANDLED
#include <SDL.h>
// ============

#include "canvas.hpp"

int main(int argc, char* args[])
{
	SDL_SetMainReady();
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3D");

	srand(time(NULL));
	bool isRunning = true;

	std::string name = "Maze";

	Canvas app = {name, 800, 600, &isRunning};

	// = Main Program Loop =
	while (isRunning) 
	{
		app.Loop();
	}

	return 0;
}