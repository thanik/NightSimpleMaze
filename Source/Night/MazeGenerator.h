#pragma once
#include "pch.h"

using namespace std;
class MazeGenerator
{
public:
	MazeGenerator() noexcept(false);
	vector<vector<int>> GenerateMap(int width, int height);
	void FreeMap();
private:
	bool m_isGenerated;
	int m_width;
	int m_height;
	vector<int> m_currentCell;
	int m_visited;
	vector<vector<int>> GetNeighborsWithWalls(int x, int y);
	vector<vector<int>> m_adjacentCells;
	vector<vector<int>> m_map;
	void PrintMap();
};

