#pragma once
#include <iostream>
#include <vector>

typedef struct GridIndex 
{
	int x;
	int y;
	int distFromStart;
	int parentIndex;
	bool visited;
	bool isWall;
};

//Forward declaration of other class
class Canvas;
class Tile;

class Maze 
{
public:
	
	void InitMaze(int nCellsWidth, int nCellsHeight);
	void GenerateMaze(Canvas* canvas);
	void FindPath(int x1, int y1, int x2, int y2);
	void GeneratePath(Canvas* canvas);

	// Methods to interact with other classes
	int GetMazeWidth() const;
	int GetMazeHeight() const;
	int GetCellWidth() const;
	int GetCellHeight() const;
	std::vector<GridIndex>* GetMaze(void);
	std::vector<GridIndex>* GetPath(void);
	std::vector<Tile>& GetTiles(void);

	static Maze* GetInstance() 
	{
		if (_mazePtr == nullptr) 
		{
			_mazePtr = new Maze();
		}

		return _mazePtr;
	}

protected:
	Maze();
	~Maze();

private:
	static Maze* _mazePtr;
	
	int _mazeSizeWidth;
	int _mazeSizeHeight;
	int _cellWidth;
	int _cellHeight;

	std::vector<GridIndex> _maze;
	std::vector<GridIndex> _path;
	
	std::vector<Tile> _tiles;
};