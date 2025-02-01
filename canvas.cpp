#include "canvas.hpp"
#include "errorchecker.hpp"
#include "maze.hpp"
#include "tile.hpp"

// = DirectX =
#include <d3dcompiler.h>
// ===============

// = SDL =
#include <SDL_config_windows.h>
#include <SDL_syswm.h>
// ============

#include <iostream>
#include <vector>
#include <cassert>

// ========== Public ==========
Canvas::Canvas(std::string& title, int scrnW, int scrnH, bool* isRunning)
{
	ErrorChecker errChecker = {};
	
	// = Special SDL check =
	// = 特別なSDL確認 =
	if (!errChecker.CheckSDLInitResult(SDL_Init(SDL_INIT_EVERYTHING))) return;

	/*
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
	{
		std::cerr << "Error initializing SDL\n";
		return;
	}
	*/

	_title = title;
	_isRunning = isRunning;
	_scrnW = scrnW;
	_scrnH = scrnH;

	// ====== SDL ======

	// SDL window (画面）context
	// 
	if (!errChecker.CheckSDLResult(_window = SDL_CreateWindow(
		_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		scrnW, scrnH, NULL), 
		ErrorCheckSDLWindow)) return;

	/*
	if (!(_window = SDL_CreateWindow(
		_title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		scrnW,
		scrnH,
		NULL))) {
	
		std::cerr << "Error initializing SDL window\n";
		return;
	}
	*/
	// ===========================

	// ====== D3D11 ======

	// Init DirectX device and swap chain
	// DirectX COM Object device と swap chain を イニシャライズ
	if (!errChecker.CheckDX11HRESULTSUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)), ErrorCheckFactory1)) return;

	constexpr D3D_FEATURE_LEVEL d3dFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
	HRESULT HR_d3dFeatureLevel = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, 
		0, &d3dFeatureLevel, 1, D3D11_SDK_VERSION, &_device, nullptr, &_deviceContext);
	if (!errChecker.CheckDX11HRESULTSUCCEEDED(HR_d3dFeatureLevel, ErrorCheckDeviceAndDeviceContext)) return;

	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.ScissorEnable = FALSE;
	rastDesc.MultisampleEnable = FALSE;
	rastDesc.AntialiasedLineEnable = FALSE;
	if (!errChecker.CheckDX11HRESULTSUCCEEDED(_device->CreateRasterizerState(&rastDesc, &_rasterizerState), ErrorCheckRasterizerState)) return;

	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = scrnW;
	scDesc.Height = scrnH;
	scDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 2;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	scDesc.Flags = {};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC scFullscrnDesc = {};
	scFullscrnDesc.Windowed = true;

	// Gets the HWND from SDL
	// SDL から HWND タイプを取得する
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(_window, &info);
	SDL_assert(info.subsystem == SDL_SYSWM_WINDOWS);
	assert(IsWindow(info.info.win.window));
	if (!errChecker.CheckDX11HRESULTSUCCEEDED(_dxgiFactory->CreateSwapChainForHwnd(_device.Get(), info.info.win.window,
		&scDesc, nullptr, nullptr, &_swapChain), ErrorCheckSwapChain)) return;
	
	CreateSwapChainResources();

	// == Generate Maze ==
	// == メイズを生じる ==
	
	Maze& maze = Maze::GetInstance();
	maze.InitMaze(20, 20, _scrnW, _scrnH);
	maze.GenerateMaze(this);
	std::cout << "Maze created with " << maze.GetMaze()->size() << " cells\n";
	maze.FindPath(0, 0, 400, 400);
	maze.GeneratePath(this);
	//====================

	std::cout << "Canvas Initialization SUCCESS\n";
}

Canvas::~Canvas() 
{
	_deviceContext->Flush();
	DestroySwapChainResources();
	_dxgiFactory.Reset();
	_swapChain.Reset();
	_deviceContext.Reset();
	_device.Reset();
	*_isRunning = false;
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

int Canvas::GetScreenWidth(void) const 
{
	return _scrnW;
}

int Canvas::GetScreenHeight(void) const 
{
	return _scrnH;
}

// ==================================================


// ========== Private ==========
void Canvas::ProcessInput(void) 
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) 
	{
		case SDL_QUIT:
		{
			*_isRunning = false;
		}break;

		case SDL_KEYDOWN: 
		{
			if (event.key.keysym.sym == SDLK_ESCAPE) 
			{
				*_isRunning = false;
			}
		}break;
	}
}

void Canvas::UpdateVariables(void) 
{
	// No code
}

void Canvas::RenderGraphics(void) 
{
	if (*_isRunning) 
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(_scrnW);
		viewport.Height = static_cast<float>(_scrnH);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;


		// Device Context
		// Sets all relevant COM Objects
		// 関連する全てのCOM Objectを設定する
		constexpr float clearColor[] = { 0.5f, 0.5f, 0.5f, 1.0f };

		_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
		_deviceContext->RSSetState(_rasterizerState.Get());
		_deviceContext->RSSetViewports(1, &viewport);
		_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

		// Get Maze instance and its tiles
		// メイズのタイルごと
		Maze& maze = Maze::GetInstance();
		std::vector<Tile>& tiles = maze.GetTiles();
		for (auto& tile : tiles)
		{
			tile.Render(this);
		}

		// Present the swap chain
		// 表示
		_swapChain->Present(1, 0);
	}
}

// = DirectX =
bool Canvas::CreateSwapChainResources(void) 
{
	ErrorChecker errChecker = {};

	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	if (!errChecker.CheckDX11HRESULTSUCCEEDED(_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)), ErrorCheckBackBuffer)) return false;
	if (!errChecker.CheckDX11HRESULTSUCCEEDED(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &_renderTarget),
		ErrorCheckRenderTargetView)) return false;

	return true;
}

void Canvas::DestroySwapChainResources(void) 
{
	_renderTarget.Reset();
}
// =============================================