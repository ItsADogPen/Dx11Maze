#pragma once

#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <DirectXColors.h>
#include <DirectXMath.h>

#include <string>
#include <vector>

// Forward declaration of other classes
class Canvas;
class Maze;

class Tile 
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using Position = DirectX::XMFLOAT3;
	using Color = DirectX::XMFLOAT3;

	// forward declare struct
	struct VertexPosCol;

public:
	Tile(int x, int y, bool isWall, bool isPath, Maze* maze);
	~Tile(void);

	// Methods to interact with Maze data
	void SetIsWall(bool isWall);
	void SetIsPath(bool isPath);
	bool GetIsWall() const;
	bool GetIsPath() const;

	// Methods to interact with Canvas
	bool LoadShaders(Canvas* canvas);
	void Render(Canvas* canvas);

protected:
	bool CompileShader(const std::wstring& filename, const std::string& entryPoint, 
		const std::string& profile, ComPtr<ID3DBlob>& shaderBlob) const;
	

private:
	[[nodiscard]] ComPtr<ID3D11VertexShader> CreateVertexShader(const std::wstring& filename, ComPtr<ID3DBlob>& vertexShaderBlob, Canvas* canvas) const;
	[[nodiscard]] ComPtr<ID3D11PixelShader> CreatePixelShader(const std::wstring& filename, Canvas* canvas) const;

	ComPtr<ID3D11Buffer> _vertexBuffer = nullptr;
	ComPtr<ID3D11InputLayout> _vertexLayout = nullptr;
	ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
	ComPtr<ID3D11PixelShader> _pixelShader = nullptr;

	int _posX;
	int _posY;
	float _cellWidth;
	float _cellHeight;
	float _worldX;
	float _worldY;
	float _normWorldX;
	float _normWorldY;
	float _normWidth;
	float _normHeight;

	bool _isWall;
	bool _isPath;

	struct VertexPosCol 
	{
		Position pos;
		Color col;
	};
};