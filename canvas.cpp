#include "main.hpp"

#include "canvas.hpp"
#include "tile.hpp"
#include "maze.hpp"

#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

#include <iostream>
#include <vector>
#include <cassert>

Maze* nMaze;

Canvas::Canvas(std::string& title)
{
	_title = title;
}
Canvas::~Canvas() 
{
	_deviceContext->Flush();
	DestroySwapChainResources();
	_dxgiFactory.Reset();
	_swapChain.Reset();
	_deviceContext.Reset();
	_device.Reset();
}

bool Canvas::Init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "Error initializing SDL.\n";
		return false;
	}

	// = SDL =

	// SDL window context
	if (!(_window = SDL_CreateWindow(
		_title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		scrn_w,
		scrn_h,
		NULL)))
	{
		std::cerr << "Error initializing SDL window.\n";
		return false;
	}

	// = D3D11 =

	// init directX devices and swapchain
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)))) 
	{
		std::cerr << "DXGI: Failed to create factory\n";
		return false;
	}

	// feature level and create D3D11 device
	constexpr D3D_FEATURE_LEVEL devFeatureLv = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

	if (FAILED(D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr,
		0, &devFeatureLv, 1, D3D11_SDK_VERSION, &_device, nullptr, &_deviceContext
	))) 
	{
		std::cerr << "D3D11: Failed to create Device and Device Context\n";
		return false;
	}

	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.ScissorEnable = FALSE;
	rastDesc.MultisampleEnable = FALSE;
	rastDesc.AntialiasedLineEnable = FALSE;

	if (FAILED(_device->CreateRasterizerState(&rastDesc, &_rasterizerState))) 
	{
		std::cerr << "D3D11: Failed to create rasterizer state\n";
		return false;
	}

	// swap chain description
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = scrn_w;
	scDesc.Height = scrn_h;
	scDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	scDesc.Flags = {};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC scFullscreenDesc = {};
	scFullscreenDesc.Windowed = true;

	// Gets the HWND from SDL
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(_window, &info);
	SDL_assert(info.subsystem == SDL_SYSWM_WINDOWS);
	assert(IsWindow(info.info.win.window));

	if (FAILED(_dxgiFactory->CreateSwapChainForHwnd(
		_device.Get(), info.info.win.window, &scDesc,
		nullptr, nullptr, &_swapChain))) 
	{
		std::cerr << "DXGI: Failed to create Swapchain\n";
		return false;
	}

	CreateSwapChainResources();


	// Generate Maze
	nMaze = Maze::GetInstance();
	nMaze->InitMaze(20, 20);
	nMaze->GenerateMaze(this);
	nMaze->FindPath(1, 1, 400, 400);
	nMaze->GeneratePath(this);

	return true;
}

void Canvas::Loop(void) 
{
	ProcessInput();
	UpdateVariables();
	RenderGraphics();
}

ID3D11Device* Canvas::GetDevice(void) 
{
	return _device.Get();
}

ID3D11DeviceContext* Canvas::GetDeviceContext(void) 
{
	return _deviceContext.Get();
}

void Canvas::ProcessInput(void)
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
		case SDL_QUIT:
		{
			isRunning = false;
		}break;

		case SDL_KEYDOWN:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				isRunning = false;
			}
		}break;
	}
}

void Canvas::UpdateVariables(void) 
{
	// NO CODE
	// ÉRÅ[ÉhÇ»Çµ
}


void Canvas::RenderGraphics(void) 
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(scrn_w);
	viewport.Height = static_cast<float>(scrn_h);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	_deviceContext->RSSetViewports(1, &viewport);

	// Rasterizer state
	_deviceContext->RSSetState(_rasterizerState.Get());

	// = Device Context =
	constexpr float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
	
	// Set the render target where we'll draw
	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);
	
	// Get Maze instance and its tiles
	Maze* maze = Maze::GetInstance();
	if (maze) 
	{
		std::vector<Tile>& tiles = maze->GetTiles();
		for (auto& tile : tiles)
		{
			tile.LoadShaders(this);
			tile.Render(this);
		}
			
	}

	_swapChain->Present(1, 0);
}

bool Canvas::CreateSwapChainResources(void) 
{
	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	if (FAILED(_swapChain->GetBuffer(
		0, IID_PPV_ARGS(&backBuffer)))) 
	{
		std::cerr << "D3D11: Failed to get Back Buffer from the SwapChain\n";
		return false;
	}

	if (FAILED(_device->CreateRenderTargetView(
		backBuffer.Get(), nullptr, &_renderTarget))) 
	{
		std::cerr << "D3D11: Failed to create RTV from Back Buffer\n";
		return false;
	}

	return true;
}

void Canvas::DestroySwapChainResources(void) 
{
	_renderTarget.Reset();
}

void Canvas::UpdateTime(void) 
{

}