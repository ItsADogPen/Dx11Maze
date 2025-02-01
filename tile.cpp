#include "tile.hpp"
#include "canvas.hpp"
#include "maze.hpp"
#include "errorchecker.hpp"

#include <d3dcompiler.h>

// = SDL =
#include <SDL.h>
#include <SDL_config_windows.h>
#include <SDL_syswm.h>
//=========

#include <iostream>


// ======= Public ================
Tile::Tile(int x, int y, bool isWall, bool isPath, int scrnW, int scrnH, Maze* maze)
{
	// Initial XY
	// イニシャルXY
	_posX = x;
	_posY = y;

	// Cell width and height
	// グリッド W & H
	_cellWidth = static_cast<float>(maze->GetCellWidth());
	_cellHeight = static_cast<float>(maze->GetCellHeight());

	// Change to world coordinates
	// ワールド座標に変更する
	_worldX = static_cast<float>(_posX) * _cellWidth;
	_worldY = static_cast<float>(_posY) * _cellHeight;

	// Change to Dx11 Normal Device Coordinates
	// Dx11 の NDC座標に変更する
	_normWorldX = (_worldX / (scrnW / 2.0f)) - 1.0f;
	_normWorldY = 1.0f - ((_worldY + _cellHeight) / (scrnH / 2.0f));
	_normWidth = _cellWidth / (scrnW / 2.0f);
	_normHeight = _cellHeight / (scrnH / 2.0f);

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

bool Tile::Initialize(Canvas* canvas) 
{
	if (_isInit) return true;
	if (!LoadShaders(canvas)) return false;

	_isInit = true;
	return true;
}

void Tile::Render(Canvas* canvas)
{
	ID3D11DeviceContext* deviceContext = canvas->GetDeviceContext();

	// Set shaders and input layout
	// シェーダーを設定する、インプットレイアウトを設定する
	deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
	deviceContext->IASetInputLayout(_vertexLayout.Get());

	// Set vertex buffers
	// バーテックスシェーダーを設定する
	UINT stride = sizeof(VertexPosCol);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set primitive topology
	// プリミティブトポロジーを設定する
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw
	// 確認
	deviceContext->Draw(6, 0);
}
// =============================================


// ======== Private =======
bool Tile::CompileShader(const std::wstring& filename, const std::string& entryPoint,
	const std::string& profile, ComPtr<ID3DBlob>& shaderBlob) const
{
	constexpr uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ComPtr<ID3DBlob> tempShaderBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	ErrorChecker errChecker = {};

	if (!errChecker.CheckDX11HRESULTSUCCEEDED(
		D3DCompileFromFile(filename.data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.data(), profile.data(), compileFlags, 0, &tempShaderBlob, &errorBlob),
		ErrorCheckShader)) {
		
		if (errorBlob != nullptr)
		{
			std::cerr << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
		}
		return false;
	}

	shaderBlob = std::move(tempShaderBlob);
	return true;
}

bool Tile::LoadShaders(Canvas* canvas)
{
	ID3D11Device* device = canvas->GetDevice();
	ID3D11DeviceContext* deviceContext = canvas->GetDeviceContext();

	ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
	_vertexShader = CreateVertexShader(L"assets\\shaders\\tile_vs.hlsl", vertexShaderBlob, canvas);
	if (_vertexShader == nullptr) return false;

	_pixelShader = CreatePixelShader(L"assets\\shaders\\tile_ps.hlsl", canvas);
	if (_pixelShader == nullptr) return false;

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

	ErrorChecker errChecker = {};

	if (!errChecker.CheckDX11HRESULTSUCCEEDED(device->CreateInputLayout(
		vertexInputLayoutInfo, _countof(vertexInputLayoutInfo),
		vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &_vertexLayout
	),
		ErrorCheckInputLayout)) return false;


	// Determine tile data based on Maze
	// タイルデータをメイズのデータによって定める

	DirectX::XMFLOAT3 tileCol;
	if (_isWall)	tileCol = { 1.0f, 0.0f, 0.0f };
	else if (_isPath)	tileCol = { 0.0f, 1.0f, 0.0f };
	else            	tileCol = { 1.0f, 1.0f, 1.0f };

	// Define vertices for a quad representing the tile
	// 2つの三角形で四角形を定義する
	VertexPosCol vertices[] =
	{
		// First triangle (top-left, top-right, bottom-left)
		// 1目様
		{{_normWorldX,			    _normWorldY,			   0.0f}, {tileCol}},
		{{_normWorldX + _normWidth, _normWorldY,			   0.0f}, {tileCol}},
		{{_normWorldX,			    _normWorldY + _normHeight, 0.0f}, {tileCol}},

		// Second triangle (bottom-left, top-right, bottom-right)
		// 2目様
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

	if (!errChecker.CheckDX11HRESULTSUCCEEDED(device->CreateBuffer(
		&bufferInfo, &resourceData, &_vertexBuffer
	), ErrorCheckVertexBuffer)) return false;

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
// =======================================