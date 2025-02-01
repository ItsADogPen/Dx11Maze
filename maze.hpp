#pragma once
#include <iostream>
#include <vector>
#include <memory>

typedef struct GridIndex 
{
	int x;
	int y;
	int distFromStart;
	int parentIndex;
	bool visited;
	bool isWall;
};

// Forward declaration of other classes
// 前のクラス表明
class Canvas;
class Tile;

class Maze 
{
public:
	Maze(const Maze&) = delete;
	Maze& operator=(const Maze&) = delete;

	// Meyers' Singleton
	static Maze& GetInstance(void)
	{
		static Maze instance;
		return instance;
	}
	~Maze(void);
	
	void InitMaze(int nCellsWidth, int nCellsHeight, int scrnW, int scrnH);
	void GenerateMaze(Canvas* canvas);
	void FindPath(int startX, int startY, int endX, int endY);
	void GeneratePath(Canvas* canvas);

	
	int GetMazeWidth(void) const;
	int GetMazeHeight(void) const;
	int GetCellWidth(void) const;
	int GetCellHeight(void) const;
	std::vector<GridIndex>* GetMaze(void);
	std::vector<GridIndex>* GetPath(void);
	std::vector<Tile>& GetTiles(void);

private:
	Maze(void);

	static Maze* _mazePtr;

	int _mazeSizeWidth;
	int _mazeSizeHeight;
	int _cellWidth;
	int _cellHeight;

	std::vector<GridIndex> _maze;
	std::vector<GridIndex> _path;
	std::vector<Tile> _tiles;
};