#include "pch.h"
#include "MazeGenerator.h"

MazeGenerator::MazeGenerator() noexcept(false)
{
	m_isGenerated = false;
	m_width = 0;
	m_height = 0;
	m_visited = 0;
}

vector<vector<int>> MazeGenerator::GenerateMap(int width, int height)
{
	m_width = width;
	m_height = height;
	m_visited = 0;
	int width_blocks = width * 2 - 1;
	int height_blocks = height * 2 - 1;
	stack<vector<int>> cellIndexStack;
	m_map = vector<vector<int>>(height_blocks, vector<int>(width_blocks, 0));
	m_currentCell.push_back(rand() % m_width);
	m_currentCell.push_back(rand() % m_height);

	while (m_visited < m_width * m_height)
	{
		m_adjacentCells = GetNeighborsWithWalls(m_currentCell[0], m_currentCell[1]);
		if (m_adjacentCells.size() > 0)
		{
			int randomCell = rand() % m_adjacentCells.size();
			vector<int> cell = m_adjacentCells[randomCell];
			int cellX = cell[0] * 2;
			int cellY = cell[1] * 2;
			if (cell[0] - m_currentCell[0] > 0)
			{
				m_map[cellY][cellX - 1] = 1;
			}
			else if (cell[0] - m_currentCell[0] < 0)
			{
				m_map[cellY][cellX + 1] = 1;
			}

			if (cell[1] - m_currentCell[1] > 0)
			{
				m_map[cellY - 1][cellX] = 1;
			}
			else if (cell[1] - m_currentCell[1] < 0)
			{
				m_map[cellY + 1][cellX] = 1;
			}
			m_map[cellY][cellX] = 1;

			cellIndexStack.push(m_currentCell); // push the current cell onto the stack
			m_currentCell = cell; // make the random neighbor the new current cell
			m_visited++; // increment the # of cells visited
		}
		else
		{
			m_currentCell = cellIndexStack.top();
			cellIndexStack.pop();
		}
	}
	//PrintMap();
	return m_map;
}

void MazeGenerator::FreeMap()
{
	for (size_t y = 0; y < m_map.size(); y++)
	{
		for (size_t x = 0; x < m_map[y].size(); x++)
		{
			m_map[y][x] = 0;
		}
	}
}

void MazeGenerator::PrintMap()
{
	int width_blocks = m_width * 2 - 1;
	int height_blocks = m_height * 2 - 1;
	wstring mazeString = L"";
	for (int x = 0; x < width_blocks + 2; x++)
		mazeString.append(L"▓");
	mazeString += '\n';
	for (int y = 0; y < height_blocks; y++)
	{
		mazeString.append(L"▓");
		for (int x = 0; x < width_blocks; x++)
		{
			if (m_map[y][x] == 0)
			{
				mazeString.append(L"▓");
			}
			else
			{
				mazeString.append(L" ");
			}
		}
		mazeString.append(L"▓");
		mazeString += '\n';
	}
	for (int x = 0; x < width_blocks + 2; x++)
		mazeString.append(L"▓");
	mazeString += '\n';

	OutputDebugStringW(mazeString.c_str());
}

vector<vector<int>> MazeGenerator::GetNeighborsWithWalls(int x, int y)
{
	vector<vector<int>> neighbors;
	if (y > 0 && m_map[(y - 1) * 2][x * 2] == 0) // top
	{
		neighbors.push_back(vector<int>({ x, (y - 1) }));
	}

	if (x > 0 && m_map[y * 2][(x - 1) * 2] == 0) // left
	{
		neighbors.push_back(vector<int>({ (x - 1), y }));
	}

	if (x < m_width - 1 && m_map[y * 2][(x + 1) * 2] == 0) // right
	{
		neighbors.push_back(vector<int>({ (x + 1), y }));
	}

	if (y < m_height - 1 && m_map[(y + 1) * 2][x * 2] == 0) // bottom
	{
		neighbors.push_back(vector<int>({ x, (y + 1) }));
	}
	return neighbors;
}