#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

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

	unsigned int m_cellsX{};
	unsigned int m_cellsY{};

	sf::Rect<float> m_border{};
	std::vector<std::vector<Cell<T>>> m_cellsArray{};

	sf::VertexArray m_renderGrid{};


private:
	void initGridCells()
	{
		/* this function initilises the grid cells in a 2d Array for the rows and columns
			it is a 2d array so we can optimise searches by fitering through the X and then Y coordinate
			rather than searching through every gridcell
		*/

		m_cellsArray.reserve(m_cellsX);
		std::vector<Cell<T>> cellsRow(m_cellsY);

		for (size_t i = 0; i < m_cellsX; i++)
		{
			for (size_t j = 0; j < m_cellsY; j++)
			{
				const sf::Rect rect( i * m_cellWidth, j * m_cellHeight, m_cellWidth, m_cellHeight );
				Cell<T> cell(rect, m_vertexReserve);

				cellsRow.emplace_back(cell);
			}
			m_cellsArray.emplace_back(cellsRow);
			cellsRow.clear();
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
		for (size_t i = 0; i < m_cellsX; i++)
		{
			grid[counter].position = { i * m_cellWidth, 0 };
			grid[counter + 1].position = { i * m_cellWidth, m_border.top + m_border.height };
			counter += 2;
		}

		for (size_t i = 0; i < m_cellsY; i++)
		{
			grid[counter].position = { 0, i * m_cellHeight };
			grid[counter + 1].position = { m_border.left + m_border.width, i * m_cellHeight };
			counter += 2;
		}

		m_renderGrid = grid;
	}



	bool checkValidIndex(const sf::Vector2i index) const
	{
		if (index.x < 0 || index.y < 0 || index.x >= m_cellsX || index.y >= m_cellsY)
			return false;
		return true;
	}


	static void RaiseOutOfBoundsError(const std::string& message)
	{
		throw std::out_of_range(message);
	}


	Cell<T>& get(sf::Vector2i index)
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

	sf::Vector2i positionToIndex(const sf::Vector2f position) const
	{
		return sf::Vector2i(std::floor(position.x / m_cellWidth), std::floor(position.y / m_cellHeight));
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

	sf::VertexArray* getRenderGrid()
	{
		return &m_renderGrid;
	}


	std::vector<T*>& findNear(const sf::Vector2f position, const float visualRange, const unsigned int unique_id)
	{
		// now we need to find which cell contains the point
		const sf::Vector2i index = positionToIndex(position);
		std::vector<T*>& entities = get(index).container;

		return entities;
		//return apply_visual_range(entities, position, visualRange, unique_id);
	}


	void addPoint(T* point)
	{
		if (const sf::Vector2i index = positionToIndex(point->getPosition()); checkValidIndex(index) == true)
			get(index).add(point);

		else
			RaiseOutOfBoundsError("[RUNTIME ERROR]: object passed in is out of bounds, check boundary collision");
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

		m_cellsX = gridsX;
		m_cellsY = gridsY;

		initSpatialHashGrid();
		initVertexArray();
	}
};