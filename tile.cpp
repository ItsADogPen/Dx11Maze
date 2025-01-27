#include "main.hpp"
#include "tile.hpp"
#include "canvas.hpp"
#include "maze.hpp"

#include <SDL.h>
#include <SDL_config_windows.h>
#include <SDL_syswm.h>

#include <d3dcompiler.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")


Tile::Tile(int x, int y, bool isWall, bool isPath, Maze* maze)
{
	// Initial XY
	// 初度のXY
	_posX = x;
	_posY = y;

	// Cell width and height
	// タイルの幅と高さ
	_cellWidth = static_cast<float>(maze->GetCellWidth());
	_cellHeight = static_cast<float>(maze->GetCellHeight());
	
	// Change to world coordinates
	// ワールドコーディネートに変更
	_worldX = static_cast<float>(_posX) * _cellWidth;
	_worldY = static_cast<float>(_posY) * _cellHeight;
	
	// Change to Dx11 Normal Device Coordinates
	// Dx11 の ノーマルデバイスコーディネート (NDC)　に変更
	_normWorldX = (_worldX / (scrn_w / 2.0f)) - 1.0f;
	_normWorldY = 1.0f - ( (_worldY + _cellHeight) / (scrn_h / 2.0f));
	_normWidth = _cellWidth / (scrn_w / 2.0f);
	_normHeight = _cellHeight / (scrn_h / 2.0f);

	// Booleans
	// ブーリアン
	_isWall = isWall;
	_isPath = isPath;
}

Tile::~Tile() 
{
	_vertexBuffer.Reset();
	_vertexLayout.Reset();
	_vertexShader.Reset();
	_pixelShader.Reset();
}

void Tile::SetIsWall(bool isWall) 
{
	_isWall = isWall;
}

void Tile::SetIsPath(bool isPath) 
{
	_isPath = isPath;
}

bool Tile::GetIsWall() const 
{
	return _isWall;
}

bool Tile::GetIsPath() const 
{
	return _isPath;
}

void Tile::Render(Canvas* canvas) 
{
	ID3D11DeviceContext* deviceContext = canvas->GetDeviceContext();

	// Set shaders and input layout
	// シェーダーとインプットレイアウト
	deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
	deviceContext->IASetInputLayout(_vertexLayout.Get());

	// Set vertex buffers
	// vertexバッファ
	UINT stride = sizeof(VertexPosCol);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set primitive topology
	// primmitive トポロジー
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw
	// 画面に描く
	deviceContext->Draw(6, 0);
}

bool Tile::LoadShaders(Canvas* canvas)
{
	ID3D11Device* device = canvas->GetDevice();
	ID3D11DeviceContext* deviceContext = canvas->GetDeviceContext();

	ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
	_vertexShader = CreateVertexShader(L"assets\\shaders\\tile_vs.hlsl", vertexShaderBlob, canvas);
	if (_vertexShader == nullptr) 
	{
		return false;
	}

	_pixelShader = CreatePixelShader(L"assets\\shaders\\tile_ps.hlsl", canvas);
	if (_pixelShader == nullptr) 
	{
		return false;
	}

	constexpr D3D11_INPUT_ELEMENT_DESC vertexInputLayoutInfo[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexPosCol, pos),
			D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"COLOR",
			0,
			DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexPosCol, col),
			D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	if (FAILED(device->CreateInputLayout(
		vertexInputLayoutInfo, _countof(vertexInputLayoutInfo),
		vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
		&_vertexLayout))) {
		
		std::cerr << "D3D11: Failed to create default vertex input layout\n";
		return false;
	}

	// Determine tile color based on state
	// 状態によって色を変更
	DirectX::XMFLOAT3 tileCol;
	if		(_isWall)	tileCol = { 1.0f, 0.0f, 0.0f };
	else if (_isPath)	tileCol = { 0.0f, 1.0f, 0.0f };
	else            	tileCol = { 1.0f, 1.0f, 1.0f };

	// Define vertices for a quad representing the tile
	// 四角いの２つの三角形
	VertexPosCol vertices[] =
	{
		// First triangle (top-left, top-right, bottom-left)
		{{_normWorldX,			    _normWorldY,			   0.0f}, {tileCol}},
		{{_normWorldX + _normWidth, _normWorldY,			   0.0f}, {tileCol}},
		{{_normWorldX,			    _normWorldY + _normHeight, 0.0f}, {tileCol}},

		// Second triangle (bottom-left, top-right, bottom-right
		{{_normWorldX,			    _normWorldY + _normHeight, 0.0f}, {tileCol}},
		{{_normWorldX + _normWidth, _normWorldY,			   0.0f}, {tileCol}},
		{{_normWorldX + _normWidth, _normWorldY + _normHeight, 0.0f}, {tileCol}},
	};
	
	D3D11_BUFFER_DESC bufferInfo = {};
	bufferInfo.ByteWidth = sizeof(VertexPosCol) * 6;
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = vertices;

	if (FAILED(device->CreateBuffer(
		&bufferInfo,
		&resourceData,
		&_vertexBuffer
	))) {
		std::cerr << "D3D11: Failed to create triangle vertex buffer\n";
		return false;
	}
	return true;
}

bool Tile::CompileShader(const std::wstring& filename, const std::string& entryPoint, 
	const std::string& profile, ComPtr<ID3DBlob>& shaderBlob) const
{
	constexpr uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ComPtr<ID3DBlob> tempShaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(filename.data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.data(), profile.data(), compileFlags, 0, &tempShaderBlob, &errorBlob))) {
		
		std::cerr << "D3D11: Failed to read shader from file\n";
		if (errorBlob != nullptr) 
		{
			std::cerr << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
		}

		return false;
	}

	shaderBlob = std::move(tempShaderBlob);
	return true;
}

Tile::ComPtr<ID3D11VertexShader> Tile::CreateVertexShader(const std::wstring& fileName, ComPtr<ID3DBlob>& vertexShaderBlob, Canvas* canvas) const
{
	if (!CompileShader(fileName, "main", "vs_5_0", vertexShaderBlob)) 
	{
		return nullptr;
	}

	ComPtr<ID3D11VertexShader> vertexShader;
	if (FAILED(canvas->GetDevice()->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		nullptr,
		&vertexShader))) 
	{
		std::cerr << "D3D11: Failed to compile vertex shader\n";
		return nullptr;
	}

	return vertexShader;
}

Tile::ComPtr<ID3D11PixelShader> Tile::CreatePixelShader(const std::wstring& filename, Canvas* canvas) const
{
	ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
	if (!CompileShader(filename, "main", "ps_5_0", pixelShaderBlob)) 
	{
		return nullptr;
	}

	ComPtr<ID3D11PixelShader> pixelShader;
	if (FAILED(canvas->GetDevice()->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize(),
		nullptr,
		&pixelShader))) {
	
		std::cerr << "D3D11: Failed to compile pixel shader\n";
		return nullptr;
	}

	return pixelShader;
}