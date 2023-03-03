#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "utilities.h"

/*
	SpatialHashGrid

	Improvements:
	- make a check visual range function
	- make a way to return the cells within visual range
	- automatic resolution resizer

	Notes:
	FINAL GOAL: 50k particles at 144fps
	03/10/2022 - 20k at 22fps
	26/10/2022 - 40k at 32fps

	REMEMBER:
	changing colors of the rects takes up about 20,000 microseconds
*/


class SpatialHashGrid
{
	float m_cellWidth{};
	float m_cellHeight{};

	const unsigned int m_vertexReserve{};

	unsigned int m_cellsX{};
	unsigned int m_cellsY{};

	sf::Rect<float> m_border{};
	std::vector<std::vector<Cell>> m_cellsArray{};

	sf::VertexArray m_renderGrid{};


private:
	void initGridCells()
	{
		/* this function initilises the grid cells in a 2d Array for the rows and columns
			it is a 2d array so we can optimise searches by fitering through the X and then Y coordinate
			rather than searching through every gridcell
		*/

		m_cellsArray.resize(m_cellsX, std::vector<Cell>(m_cellsY));
		sf::Rect<float> rect(0, 0, m_cellWidth, m_cellHeight);
		for (int i = 0; i < m_cellsX; i++) 
		{
			rect.left = i * m_cellWidth;
			for (int j = 0; j < m_cellsY; j++) 
			{
				rect.top = j * m_cellHeight;
				m_cellsArray[i][j].rect = rect;
				m_cellsArray[i][j].m_vertexReserve = m_vertexReserve;
			}
		}
	}


	void initSpatialHashGrid()
	{
		// calculating the cell dimensions
		m_cellWidth = (m_border.left + m_border.width) / static_cast<float>(m_cellsX);
		m_cellHeight = (m_border.top + m_border.height) / static_cast<float>(m_cellsY);

		// initilising the grid
		initGridCells();
		initVertexArray();
	}


	void initVertexArray()
	{
		sf::VertexArray grid(sf::Lines, (m_cellsX + m_cellsY) * 2);

		size_t counter = 0;
		for (float i = 0; i < static_cast<float>(m_cellsX); i++)
		{
			grid[counter].position = { i * m_cellWidth, 0 };
			grid[counter + 1].position = { i * m_cellWidth, m_border.top + m_border.height };
			counter += 2;
		}

		for (float i = 0; i < static_cast<float>(m_cellsY); i++)
		{
			grid[counter].position = { 0, i * m_cellHeight };
			grid[counter + 1].position = { m_border.left + m_border.width, i * m_cellHeight };
			counter += 2;
		}

		m_renderGrid = grid;
	}



	bool checkValidIndex(const sf::Vector2i cellIndex) const
	{
		if (cellIndex.x < 0 || cellIndex.y < 0 || cellIndex.x >= m_cellsX || cellIndex.y >= m_cellsY)
			return false;
		return true;
	}


	static void RaiseOutOfBoundsError(const std::string& message)
	{
		throw std::out_of_range(message);
	}


	Cell& get(const sf::Vector2i cellIndex)
	{
		return m_cellsArray.at(cellIndex.x).at(cellIndex.y);
	}


	sf::Vector2i positionToCellIndex(const sf::Vector2f position) const
	{
		return sf::Vector2i(std::floor(position.x / m_cellWidth), std::floor(position.y / m_cellHeight));
	}


	std::vector<unsigned int> getLocalObjectIndexes(const sf::Vector2i cellIndex)
	{
		std::vector<unsigned int> selectedObjectIndexes;
		selectedObjectIndexes.reserve(get({ 0, 0 }).m_vertexReserve * 9);

		for (int x = cellIndex.x - 1; x <= cellIndex.x + 1; x++)
		{
			for (int y = cellIndex.y - 1; y <= cellIndex.y + 1; y++)
			{
				if (!checkValidIndex({ x, y }))
					continue;

				for (const unsigned int object : get({ x, y }).container)
				{
					selectedObjectIndexes.push_back(object);
				}
			}
		}

		return selectedObjectIndexes;
	}


public:
	// constructor
	SpatialHashGrid(const sf::Rect<float> border, const unsigned int gridsX, const unsigned int gridsY, const unsigned int vertexReserve = 8)
		: m_vertexReserve(vertexReserve), m_cellsX(gridsX), m_cellsY(gridsY), m_border(border)
	{
		initSpatialHashGrid();
	}
	~SpatialHashGrid() = default;


	void drawGrid(sf::RenderWindow& window) const
	{
		window.draw(m_renderGrid);
	}


	void resetGrid()
	{
		for (std::vector<Cell>& column : m_cellsArray)
		{
			for (Cell& cell : column)
				cell.clear();
		}
	}


	sf::VertexArray* getRenderGrid()
	{
		return &m_renderGrid;
	}


	std::vector<unsigned int> findNear(const sf::Vector2f position)
	{
		// now we need to find which cell contains the point
		const sf::Vector2i index = positionToCellIndex(position);

		return getLocalObjectIndexes(index);
	}


	void addPoint(const unsigned int pointIndex, const sf::Vector2f position)
	{
		if (const sf::Vector2i cellIndex = positionToCellIndex(position); checkValidIndex(cellIndex) == true)
			get(cellIndex).add(pointIndex);

		else
			RaiseOutOfBoundsError("[RUNTIME ERROR]: object passed in is out of bounds, check boundary collision");
	}



	void reSize(const unsigned int gridsX, const unsigned int gridsY)
	{
		// we need to clear all the cells' points indexes
		resetGrid();

		m_cellsArray.clear();

		m_cellsX = gridsX;
		m_cellsY = gridsY;

		initSpatialHashGrid();
		initVertexArray();
	}
};