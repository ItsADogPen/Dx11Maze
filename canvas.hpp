#pragma once


// = DirectX =
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl.h>


// = SDL =
#include <SDL.h>


// = Basics =
#include <string>

class Canvas 
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	Canvas(std::string& title, int scrnW, int scrnH, bool* isRunning);
	~Canvas(void);

	void Loop(void);

	
	ID3D11Device* GetDevice(void);
	ID3D11DeviceContext* GetDeviceContext(void);
	int GetScreenWidth(void) const;
	int GetScreenHeight(void) const;

protected:



private:

	void GenerateNewMazeSet(void);
	void ProcessInput(void);
	void UpdateVariables(void);
	void RenderGraphics(void);
	
	std::string _title;
	bool* _isRunning;
	bool _isWaitingForMaze;
	int _scrnW;
	int _scrnH;

	// ========== DirectX ==========
	bool CreateSwapChainResources(void);
	void DestroySwapChainResources(void);

	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain1> _swapChain = nullptr;
	ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
	ComPtr<ID3D11RasterizerState> _rasterizerState = nullptr;
	ComPtr<ID3D11RasterizerState> _rasterizerStateWireframe = nullptr;
	ComPtr<ID3D11RasterizerState> _rasterizerStateSolid = nullptr;
	// ================================

	// ========== SDL ==========
	SDL_Window* _window = nullptr;

	// =========================
};