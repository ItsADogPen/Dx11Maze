#include "errorchecker.hpp"

// = DirectX =
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

ErrorChecker::ErrorChecker() 
{

}

ErrorChecker::~ErrorChecker() 
{
	
}

bool ErrorChecker::CheckDX11HRESULTSUCCEEDED(HRESULT dx11Object, ErrorCheckerDX11Type type)
{
	if (FAILED(dx11Object))
	{
		switch (type)
		{
			case ErrorCheckFactory1:
			{
				std::cerr << "DXGI: Failed to create Factory1\n";
				return false;
			}

			case ErrorCheckDeviceAndDeviceContext:
			{
				std::cerr << "D3D11: Failed to create Device and Device Context\n";
				return false;
			}

			case ErrorCheckRasterizerState: 
			{
				std::cerr << "D3D11: Failed to create rasterizer state\n";
				return false;
			}

			case ErrorCheckSwapChain: 
			{
				std::cerr << "DXGI: Failed to create Swapchain\n";
				return false;
			}

			case ErrorCheckBackBuffer: 
			{
				std::cerr << "D3D11: Failed to get Back Buffer from the SwapChain\n";
				return false;
			}
			case ErrorCheckRenderTargetView: 
			{
				std::cerr << "D3D11: Failed to create RTV from Back Buffer\n";
				return false;
			}
			case ErrorCheckInputLayout:
			{
				std::cerr << "D3D11: Failed to create default vertex input layout\n";
				return false;
			}
			case ErrorCheckVertexBuffer: 
			{
				std::cerr << "D3D11: Failed to create triangle vertex buffer\n";
				return false;
			}
			case ErrorCheckShader: 
			{
				std::cerr << "D3D11: Failed to read shader from file\n";
				return false;
			}

			default: 
			{
				std::cerr << "DX11: Unknown error type\n";
				return false;
			}
		}
	}

	return true;
}

bool ErrorChecker::CheckSDLInitResult(int result) 
{
	if (result != 0)
	{
		std::cerr << "SDL: Failed to Initialize\n";
		return false;
	}
	return true;
}

bool ErrorChecker::CheckSDLResult(bool result, ErrorCheckerSDLType type) 
{
	if (!result) 
	{
		switch (type)
		{
			case ErrorCheckSDLWindow:
			{
				std::cerr << "SDL: Failed to create window context\n";
				return false;
			}

			default: 
			{
				std::cerr << "SDL2: Unknown error type\n";
				return false;
			}
		}
	}
	
	return true;
}