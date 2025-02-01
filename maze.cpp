#include "maze.hpp"
#include "tile.hpp"
#include "canvas.hpp"

#include <stdlib.h>
#include <queue>


// ======= Public ==========
Maze::~Maze()
{}

void Maze::InitMaze(int nCellsWidth, int nCellsHeight, int scrnW, int scrnH)
{
	_cellWidth = scrnW / nCellsWidth;
	_cellHeight = scrnH / nCellsHeight;
	_mazeSizeWidth = _cellWidth * nCellsWidth;
	_mazeSizeHeight = _cellHeight * nCellsHeight;
}

void Maze::GenerateMaze(Canvas* canvas)
{
	_tiles.clear();
	_maze.clear();

	for (int j = 0; j < (_mazeSizeHeight / _cellHeight); j++) {
		for (int i = 0; i < (_mazeSizeWidth / _cellWidth); i++)
		{
			GridIndex nGrid = {};
			int nRan = rand() % 10;

			if ((i == 0 && j == 0) || (nRan < 9))  nGrid.isWall = false;
			else									nGrid.isWall = true;

			nGrid.x = i;
			nGrid.y = j;
			nGrid.visited = false;
			nGrid.distFromStart = 0;
			_maze.push_back(nGrid);
		}
	}
}

void Maze::FindPath(int startX, int startY, int endX, int endY)
{
	// Convert screen coords -> grid coords
	// 画面座標 -> グリッド座標に変更する
	int startGridX = startX / _cellWidth;
	int startGridY = startY / _cellHeight;
	int endGridX = endX / _cellWidth;
	int endGridY = endY / _cellHeight;

	// Get grid dimensions
	// グリッドサイズを取って
	int gridWidth = _mazeSizeWidth / _cellWidth;
	int gridHeight = _mazeSizeHeight / _cellHeight;

	// Bounds check using lambda function
	// lamba functionで画面の限定確認
	auto isInBounds = [gridWidth, gridHeight](int x, int y) 
	{
		return x >= 0 && x < gridWidth && y >= 0 && y < gridHeight;
	};

	if (!isInBounds(startGridX, startGridY) || !isInBounds(endGridX, endGridY)) 
	{
		std::cout << "Start or end position out of bounds\n";
		return;
	}

	int endIndex = endGridY * gridWidth + endGridX;
	if (endIndex >= _maze.size()) 
	{
		std::cout << "End index out of bounds: " << endIndex << " >= " << _maze.size() << "\n";
		return;
	}

	// Check Wall
	// 壁場合を確認
	if (_maze.at(endIndex).isWall) 
	{
		std::cout << "Cannot pathfind to a wall tile at grid position: " << endGridX << ", " << endGridY << "\n";
		std::cout << "Index: " << endIndex << "\n";
		return;
	}

	// Convert start position to 1D array index
	// 初めてのポジション -> 1D アレイ index
	int startIndex = startGridY * gridWidth + startGridX;

	// Initialize start position
	// 初めてのポジションをイニシャライズ
	std::queue<int> pathQ;
	pathQ.push(startIndex);
	_maze.at(startIndex).visited = true;
	_maze.at(startIndex).isWall = false;
	_maze.at(startIndex).parentIndex = -1;

	// BFS loop
	// BFS ループ
	while (!pathQ.empty())
	{
		int currIndex = pathQ.front();
		pathQ.pop();

		GridIndex& currGrid = _maze.at(currIndex);

		// Check if we reached the end
		// 果てのポイントを確認
		if (currGrid.x == endGridX && currGrid.y == endGridY)
		{
			_path.clear();

			// Backtrack from end to start
			// 果てから初めてのポジションへ後戻る
			int backtrackIndex = currIndex;
			while (backtrackIndex != -1)
			{
				_path.push_back(_maze.at(backtrackIndex));
				backtrackIndex = _maze.at(backtrackIndex).parentIndex;
			}
			std::reverse(_path.begin(), _path.end());
			return;
		}

		// Check neighbours
		// グリッド隣人を確認
		std::vector<std::pair<int, int>> directions = 
		{ 
			{-1, 0}, 
			{1, 0}, 
			{0, -1}, 
			{0, 1} 
		};

		// ranged-based for-loop using structured binding
		// 日本語わかんね
		for (const auto& [dx, dy] : directions)
		{
			int newX = currGrid.x + dx;
			int newY = currGrid.y + dy;

			if (isInBounds(newX, newY))
			{
				int neighborIndex = newY * gridWidth + newX;
				if (!_maze.at(neighborIndex).visited && !_maze.at(neighborIndex).isWall)
				{
					_maze.at(neighborIndex).visited = true;
					_maze.at(neighborIndex).distFromStart = currGrid.distFromStart + 1;
					_maze.at(neighborIndex).parentIndex = currIndex;
					pathQ.push(neighborIndex);
				}
			}
		}
	}
}

void Maze::GeneratePath(Canvas* canvas) 
{
	// And also Maze
	// メイズと
	std::cout << _maze.size() << "\n";
	for (int i = 0; i < _maze.size(); i++) 
	{
		if (i % (_mazeSizeWidth / _cellWidth) == 0) 
		{
			std::cout << "\n";
		}

		bool isStart = false;
		bool isEnd = false;
		bool isPathCell = false;

		// Only check path-related stuff if we actually have a path
		// ルートがある場合だけで
		if (!_path.empty()) 
		{
			isStart = (_maze.at(i).x == _path.front().x && _maze.at(i).y == _path.front().y);
			isEnd = (_maze.at(i).x == _path.back().x && _maze.at(i).y == _path.back().y);
		
			for (const auto& pathCell : _path)
			{
				if (_maze.at(i).x == pathCell.x && _maze.at(i).y == pathCell.y)
				{
					isPathCell = true;
					break;
				}
			}
		}

		if (isStart) 
		{
			std::cout << " S ";
		}
		else if (isEnd) 
		{
			std::cout << " E ";
		}

		if (!_maze.at(i).isWall) 
		{
			if (isPathCell)
			{
					std::cout << "   ";
			}
			else	std::cout << " a ";
		}
		else 
		{
			std::cout << " I ";
		}
		
		Tile newTile(
			_maze.at(i).x, _maze.at(i).y,
			_maze.at(i).isWall, isPathCell,
			canvas->GetScreenWidth(), canvas->GetScreenHeight(), this);
		if (!newTile.Initialize(canvas)) 
		{
			std::cerr << "Failed to initialize tile\n";
			continue;
		}
		_tiles.push_back(newTile);
	}
}

int Maze::GetMazeWidth() const
{
	return _mazeSizeWidth;
}

int Maze::GetMazeHeight() const
{
	return _mazeSizeHeight;
}

int Maze::GetCellWidth() const
{
	return _cellWidth;
}

int Maze::GetCellHeight() const
{
	return _cellHeight;
}

std::vector<GridIndex>* Maze::GetMaze(void)
{
	return &_maze;
}

std::vector<GridIndex>* Maze::GetPath(void)
{
	return &_path;
}

std::vector<Tile>& Maze::GetTiles(void)
{
	return _tiles;
}
// =======================================

// ====== Private ======
Maze::Maze()
{}