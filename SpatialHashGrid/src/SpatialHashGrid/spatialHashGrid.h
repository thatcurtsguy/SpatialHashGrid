#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "utilities.h"

/*
	SpatialHashGrid

	PLEASE NOTE:
	> for any entity class you plan to add to this spatial hash grid, please add the following varaibles to it:
	const unsigned int id{};
	sf::Vector2i gridIndex{ -1, -1 };

	sf::Vector2f getPosition() const
	{
	    return m_position;
	}

	Improvements:
	- replace screen size settings with a rect object
	- make a check visual range function
	- make a way to check nearby gridcells
	- make a way to return the cells within visual range
	- make the grid a 2d array so we can make more optimised searching by matching the x then y coord instead of checking through all
	- automatic resolution resizer
	- option for resolution resizer
	- ability to pass in any class type to be stored
	- better cell finding alg
	- efficient grid drawing with vertex list

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
	sf::VertexArray m_renderGrid{};



	template <typename T2>
	static std::string concatenate(const T2& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}

	template <typename T2, typename... Args>
	static std::string concatenate(const T2& t, const Args&... args)
	{
		std::stringstream ss;
		ss << t << concatenate(args...);
		return ss.str();
	}



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

		for (size_t i = 0; i < m_gridsX; i++)
		{
			grid[counter + 0].position = { 0, i * m_cellWidth };
			grid[counter + 1].position = { m_border.left + m_border.width, i * m_cellWidth };
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


public:
	// constructor
	SpatialHashGrid(const sf::Rect<float> border, const unsigned int gridsX, const unsigned int gridsY, const unsigned int vertexReserve = 8)
		: m_vertexReserve(vertexReserve), m_gridsX(gridsX), m_gridsY(gridsY), m_border(border)
	{
		init();
	}
	~SpatialHashGrid() = default;


	void drawGrid(sf::RenderWindow& window)
	{
		window.draw(m_renderGrid);
	}


	sf::Vector2i findCellIndex(const sf::Vector2f position) const
	{
		/* findCell function finds the cell which contains the position parameter, it iterates through
		   cellsArray which is a 2D std::vector. once found it returns a std::tuple for the indexes to find said cell
		   if no cell was found it returns -1, -1 for both indexes
		*/
		const sf::Vector2i index(static_cast<int>(position.x / m_cellWidth), static_cast<int>(position.y / m_cellHeight));

		// validation
		if (checkValidIndex(index))
		{
			return index;
		}
		
		std::string result = concatenate("position: { ", position.x, " , ", position.y, " } ", " index: { ", index.x, ",", index.y, " }");
		RaiseOutOfBoundsError(result);
		return { -1, -1 };
	}


	std::vector<T> findNear(T& point)
	{
		// now we need to find which cell contains the point
		const sf::Vector2i cell = findCellIndex(point.getPosition());
		return cellAtIndex(cell).container;
	}


	void addPoint(T& point)
	{
		const sf::Vector2i cellIndex = findCellIndex(point.getPosition());

		point.gridIndex = cellIndex;
		cellAtIndex(cellIndex).add(point);
	}


	void addPoints(std::vector<T>& points)
	{
		// reseting cells by clearing all of their items
		for (std::vector<Cell<T>>& column : m_cellsArray)
		{
			for (Cell<T>& cell : column)
				cell.clear();
		}

		// we then can add all the Particles one by one
		for (T& point : points)
		{
			addPoint(point);
		}
	}


	void reSize(const unsigned int gridsX, const unsigned int gridsY)
	{
		// we need to clear all the cells' points indexes
		for (std::vector<Cell<T>>& column : m_cellsArray)
		{
			for (Cell<T>& cell : column)
			{
				cell.container.clear();
			}
		}

		for (T& point : m_points)
			point.gridIndex = { -1, -1 };

		m_cellsArray.clear();

		m_gridsX = gridsX;
		m_gridsY = gridsY;

		init();
		initDrawGrid();
	}
};