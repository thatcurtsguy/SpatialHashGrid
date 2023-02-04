#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>

#include "utilities.h"

/*
	SpatialHashGrid

	PLEASE NOTE:
	> for any entity class you plan to add to this spatial hash grid, please add the following varaibles to it:
	const unsigned int id{};

	sf::Vector2f getPosition() const
	{
	    return m_position;
	}

	Improvements:
	- make a check visual range function
	- make a way to check nearby gridcells
	- make a way to return the cells within visual range
	- make the grid a 2d array so we can make more optimised searching by matching the x then y coord instead of checking through all
	- automatic resolution resizer
	- option for resolution resizer
	- better cell finding alg

	Notes:
	FINAL GOAL: 50k particles at 144fps
	03/10/2022 - 20k at 22fps
	26/10/2022 - 40k at 32fps

	REMEMBER:
	changing colors of the rects takes up about 20,000 microseconds
*/


template <typename T>
class SpatialHashGrid
{
	float m_cellWidth{};
	float m_cellHeight{};

	const unsigned int m_vertexReserve{};

	unsigned int m_totalCells{};
	unsigned int m_gridsX{};
	unsigned int m_gridsY{};

	sf::Rect<float> m_border{};
	std::vector<std::vector<Cell<T>>> m_cellsArray{};
	std::vector<T> m_points{};


public:
	sf::VertexArray m_renderGrid{};


private:

	void initGrid()
	{
		/* this function initilises the grid cells in a 2d Array for the rows and columns
			it is a 2d array so we can optimise searches by fitering through the X and then Y coordinate
			rather than searching through every gridcell
		*/

		m_cellsArray.reserve(m_gridsX);

		for (size_t i = 0; i < m_gridsX; i++)
		{
			std::vector<Cell<T>> cellsRow;
			cellsRow.reserve(m_gridsY);

			for (size_t j = 0; j < m_gridsY; j++)
			{
				const sf::Rect rect( i * m_cellWidth, j * m_cellHeight, m_cellWidth, m_cellHeight );
				Cell<T> cell(rect, m_vertexReserve);

				cellsRow.emplace_back(cell);
			}
			m_cellsArray.emplace_back(cellsRow);
		}
	}


	void init()
	{
		// calculating the cell dimensions
		m_cellWidth = (m_border.left + m_border.width) / static_cast<float>(m_gridsX);
		m_cellHeight = (m_border.top + m_border.height) / static_cast<float>(m_gridsY);
		m_totalCells = m_gridsX * m_gridsY;

		// initilising the grid
		initGrid();
		initDrawGrid();
	}


	void initDrawGrid()
	{
		sf::VertexArray grid(sf::Lines, m_gridsX * m_gridsY);

		size_t counter = 0;
		for (size_t i = 0; i < m_gridsX; i++)
		{
			grid[counter+0].position = { i * m_cellWidth, 0 };
			grid[counter+1].position = { i * m_cellWidth, m_border.top + m_border.height };
			counter += 2;
		}

		for (size_t i = 0; i < m_gridsY; i++)
		{
			grid[counter + 0].position = { 0, i * m_cellHeight };
			grid[counter + 1].position = { m_border.left + m_border.width, i * m_cellHeight };
			counter += 2;
		}

		m_renderGrid = grid;
	}


	bool checkValidIndex(const sf::Vector2i index) const
	{
		if (index.x < 0 || index.y < 0 || index.x >= m_gridsX || index.y >= m_gridsY)
			return false;
		return true;
	}


	static void RaiseOutOfBoundsError(const std::string& message)
	{
		throw std::out_of_range(message);
	}


	Cell<T>& cellAtIndex(sf::Vector2i index)
	{
		return m_cellsArray.at(index.x).at(index.y);
	}


	void resetGrid()
	{
		for (std::vector<Cell<T>>& column : m_cellsArray)
		{
			for (Cell<T>& cell : column)
				cell.clear();
		}
	}

	sf::Vector2i positionToIndex(const sf::Vector2f position, const bool preventOutOfRange = false, const bool wrap = true) const
	{
		/* findCell function finds the cell which contains the position parameter, it iterates through
		   cellsArray which is a 2D std::vector. once found it returns a std::tuple for the indexes to find said cell
		   if no cell was found it returns -1, -1 for both indexes
		*/
		sf::Vector2i index(position.x / m_cellWidth, position.y / m_cellHeight);

		// validation
		if (checkValidIndex(index))
		{
			return index;
		}

		if (preventOutOfRange)
		{
			index.x = (index.x < 0) ? 0 : (index.x >= m_gridsX) ? m_gridsX - 1 : index.x;
			index.y = (index.y < 0) ? 0 : (index.y >= m_gridsY) ? m_gridsY - 1 : index.y;
			return index;

		}

		if (wrap)
		{

		}

		const std::string result = concatenate("position: { ", position.x, " , ", position.y, " } ", " index: { ", index.x, ",", index.y, " }");
		RaiseOutOfBoundsError(result);
		return { -1, -1 };
	}


public:
	// constructor
	SpatialHashGrid(const sf::Rect<float> border, const unsigned int gridsX, const unsigned int gridsY, const unsigned int vertexReserve = 8)
		: m_vertexReserve(vertexReserve), m_gridsX(gridsX), m_gridsY(gridsY), m_border(border)
	{
		init();
	}
	~SpatialHashGrid() = default;


	void drawGrid(sf::RenderWindow& window) const
	{
		window.draw(m_renderGrid);
	}



	std::vector<T*>& findNear(T& point, const float visualRange)
	{
		// now we need to find which cell contains the point
		//return getCellsAroundRange(point.getPosition(), visualRange);
		const sf::Vector2i cell = positionToIndex(point.getPosition());
		return cellAtIndex(cell).container;
	}


	void addPoint(T* point)
	{
		const sf::Vector2i index = positionToIndex(point->getPosition());
		cellAtIndex(index).add(point);
	}


	void addPoints(std::vector<T>& points)
	{
		resetGrid();

		// we then can add all the Particles one by one
		for (T& point : points)
		{
			addPoint(&point);
		}
	}


	void reSize(const unsigned int gridsX, const unsigned int gridsY)
	{
		// we need to clear all the cells' points indexes
		resetGrid();

		m_cellsArray.clear();

		m_gridsX = gridsX;
		m_gridsY = gridsY;

		init();
		initDrawGrid();
	}
};