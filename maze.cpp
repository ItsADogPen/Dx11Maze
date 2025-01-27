#include "main.hpp"
#include "maze.hpp"
#include "canvas.hpp"
#include "tile.hpp"

#include <stdlib.h>
#include <queue>

Maze* Maze::_mazePtr = nullptr;

Maze::Maze() 
{}

Maze::~Maze()
{}

void Maze::InitMaze(int nCellsWidth, int nCellsHeight)
{
	_cellWidth = scrn_w / nCellsWidth;
	_cellHeight = scrn_h / nCellsHeight;
	_mazeSizeWidth = _cellWidth * nCellsWidth;
	_mazeSizeHeight = _cellHeight * nCellsHeight;
}

void Maze::GenerateMaze(Canvas* canvas)
{
	_tiles.clear();
	_maze.clear();

	for (int j = 0; j < (_mazeSizeHeight / _cellHeight); j++) {
		for (int i = 0; i < (_mazeSizeWidth/_cellWidth); i++)
		{
			GridIndex nGrid = {};
			int nRan = rand() % 10;
			
			if ( (i == 0 && j == 0) || (nRan < 9))  nGrid.isWall = false;
			else									nGrid.isWall = true;
		
			nGrid.x = i;
			nGrid.y = j;
			nGrid.visited = false;
			nGrid.distFromStart = 0;
			_maze.push_back(nGrid);
		}
	}
}

void Maze::FindPath(int x1, int y1, int x2, int y2)
{
	int xConvert = x1 / _cellWidth;
	int yConvert = y1 / _cellHeight;
	// convert 2D grid to 1D array index, get cell
	// ID アレイ -> 2D グリッドに変更
	int index = _mazeSizeWidth >= _mazeSizeHeight ? 
		(yConvert * (_mazeSizeWidth / _cellWidth) + xConvert) : (xConvert * (_mazeSizeHeight / _cellHeight) + yConvert);

	std::vector<std::pair<int, int>> dir =
	{
		{-1, 0},
		{1, 0},
		{0, -1},
		{0, 1}
	};

	std::queue<int> pathQ;
	pathQ.push(index);				// A
	_maze.at(index).visited = true;
	_maze.at(index).isWall = false;
	_maze.at(index).parentIndex = -1; // using -1 to determine the start

	while (!pathQ.empty()) 
	{
		int currIndex = pathQ.front(); // A
		pathQ.pop();

		GridIndex currGrid = _maze.at(currIndex);
		int currX = currGrid.x;
		int currY = currGrid.y;
		int distP = currGrid.distFromStart;

		for (const auto& nDir : dir) 
		{
			int newX = currX + nDir.first;
			int newY = currY + nDir.second;

			if ((newX >= 0) && (newX < (_mazeSizeWidth / _cellWidth)) &&
				(newY >= 0) && (newY < (_mazeSizeHeight / _cellHeight))) {
			
				// Haven't yet set up for column-major mazes.  So for now.., just row-major mazes
				// 列優先はまだ整っていません。

				// convert grid coordinates back to 1D index
				// 2D -> 1Dに変更
				int neighbour = newY * (_mazeSizeWidth / _cellWidth) + newX;
				
				if (neighbour < _maze.size()) // Additional bounds check
				{
					if (!_maze.at(neighbour).visited && !_maze.at(neighbour).isWall)
					{
						_maze.at(neighbour).visited = true;
						_maze.at(neighbour).distFromStart = distP + 1;
						_maze.at(neighbour).parentIndex = currIndex;

						// Reached the END
						// 果てを達成
						if (_maze.at(neighbour).x == (x2 / _cellWidth) && _maze.at(neighbour).y == (y2 / _cellHeight))
						{
							_path.clear();

							// Backtrack from the end to start
							// 果てから初めてのXYへ行こう
							currIndex = neighbour;
							while (currIndex != -1) 
							{
								_path.push_back(_maze.at(currIndex));
								currIndex = _maze.at(currIndex).parentIndex;
							}

							std::reverse(_path.begin(), _path.end());

							// Clear queue to stop FindPath() BFS
							pathQ = std::queue<int>();
							std::cout << "REACHED THE END\n";
							break;
						}
						else
						{
							pathQ.push(neighbour);
						}
					}
				}
				else
				{
					std::cout << "Warning: Attempted to access out of bounds index: " << neighbour << "\n";
				}
			}
		}
	}

}

void Maze::GeneratePath(Canvas* canvas)
{
	// And also Maze
	// また、メイズ全体
	std::cout << _maze.size() << "\n";
	for (int i = 0; i < _maze.size(); i++) 
	{
		if (i % (_mazeSizeWidth / _cellWidth) == 0) 
		{
			std::cout << "\n";
		}

		bool isStart = (_maze.at(i).x == _path.front().x && _maze.at(i).y == _path.front().y);
		bool isEnd = (_maze.at(i).x == _path.back().x && _maze.at(i).y == _path.back().y);
		bool isPathCell = false;
		for (const auto& pathCell : _path) 
		{
			if (_maze.at(i).x == pathCell.x && _maze.at(i).y == pathCell.y) 
			{
				isPathCell = true;
				break;
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
		Tile newTile(_maze.at(i).x, _maze.at(i).y, _maze.at(i).isWall, isPathCell, this);
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