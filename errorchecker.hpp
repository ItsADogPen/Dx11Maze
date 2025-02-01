#pragma once

#include <iostream>
#include <string>

#include <d3d11.h>
#include <dxgi1_6.h>

/*
	Custom error handling class
	カスタムエラー確認クラス
*/

typedef enum ErrorCheckerDX11Type 
{
	ErrorCheckFactory1,
	ErrorCheckDeviceAndDeviceContext,
	ErrorCheckRasterizerState,
	ErrorCheckSwapChain,
	ErrorCheckBackBuffer,
	ErrorCheckRenderTargetView,
	ErrorCheckInputLayout,
	ErrorCheckVertexBuffer,
	ErrorCheckShader
};

typedef enum ErrorCheckerSDLType 
{
	ErrorCheckSDLInitEverything,
	ErrorCheckSDLWindow
};

class ErrorChecker 
{
public:
	ErrorChecker(void);
	~ErrorChecker(void);

	bool CheckDX11HRESULTSUCCEEDED(HRESULT dx11Object, ErrorCheckerDX11Type type);
	bool CheckSDLInitResult(int result);
	bool CheckSDLResult(bool result, ErrorCheckerSDLType type);
	bool CheckSDLResult(int SDLResult, ErrorCheckerSDLType type);
};