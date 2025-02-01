#pragma once

#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <DirectXColors.h>
#include <DirectXMath.h>

#include <string>
#include <vector>

// Forward declaration of other classes
// 前のクラス表明
class Canvas;
class Maze;

class Tile 
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	using Position = DirectX::XMFLOAT3;
	using Color = DirectX::XMFLOAT3;

	// Forward declare struct
	// 前のstruct表明 （日本語わかんね）
	typedef struct VertexPosCol;

public:
	Tile(int x, int y, bool isWall, bool isPath, int scrnW, int scrnH, Maze* maze);
	~Tile(void);


	void SetIsWall(bool isWall);
	void SetIsPath(bool isPath);
	bool GetIsWall(void) const;
	bool GetIsPath(void) const;


	bool Initialize(Canvas* canvas);
	void Render(Canvas* canvas);

protected:
	

private:
	bool CompileShader(const std::wstring& filename, const std::string& entrypoint,
		const std::string& profile, ComPtr<ID3DBlob>& shaderBlob) const;
	bool LoadShaders(Canvas* canvas);
	[[nodiscard]] ComPtr<ID3D11VertexShader> CreateVertexShader(const std::wstring& filename, ComPtr<ID3DBlob>& vertexShaderBlob, Canvas* canvas) const;
	[[nodiscard]] ComPtr<ID3D11PixelShader>	CreatePixelShader(const std::wstring& filename, Canvas* canvas) const;

	ComPtr<ID3D11Buffer> _vertexBuffer = nullptr;
	ComPtr<ID3D11InputLayout> _vertexLayout = nullptr;
	ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
	ComPtr<ID3D11PixelShader> _pixelShader = nullptr;

	bool _isInit = false;

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