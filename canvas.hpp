#pragma once

#ifndef _CANVAS_HPP_
#define _CANVAS_HPP_

#include <string>
#include <SDL.h>
#include <SDL_config_windows.h>
#include <SDL_syswm.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl.h>

class Canvas
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	Canvas(std::string& title);
	~Canvas(void);

	bool Init(void);
	void Loop(void);

	// Methods for other Classes to interact with the Canvas class
	ID3D11Device* GetDevice(void);
	ID3D11DeviceContext* GetDeviceContext(void);

protected:
	virtual void ProcessInput(void);
	virtual void UpdateVariables(void);
	virtual void RenderGraphics(void);

private:

	bool CreateSwapChainResources(void);
	void DestroySwapChainResources(void);
	void UpdateTime(void);

	// = SDL =
	SDL_Window* _window = nullptr;
	//SDL_Renderer* _renderer = nullptr;
	//SDL_Texture* _texture = nullptr;
	
	// = D3D11 =
	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain1> _swapChain = nullptr;
	ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
	ComPtr<ID3D11RasterizerState> _rasterizerState = nullptr;
	
	std::string _title;
};

#endif // !_CANVAS_HPP_