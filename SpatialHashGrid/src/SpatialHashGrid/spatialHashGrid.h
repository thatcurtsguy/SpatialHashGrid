#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

#include "utilities.h"

/*
	SpatialHashGrid

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


class SpatialHashGrid {
private:
	float m_cellWidth{};
	float m_cellHeight{};

	int m_vertexReserve{};
	int m_totalCells{};

	int m_gridsX{};
	int m_gridsY{};

	Rectangle m_border{};
	std::vector<std::vector<Cell>> m_cellsArray{};



private:
	void initGrid() {
		/* this function initilises the grid cells in a 2d Array for the rows and columns
			it is a 2d array so we can optimise searches by fitering through the X and then Y coordinate
			rather than searching through every gridcell
		*/


		for (size_t i = 0; i < m_gridsX; i++) {
			std::vector<Cell> cellsRow;
			for (size_t j = 0; j < m_gridsY; j++) {
				Rectangle rect{ i * m_cellWidth, j * m_cellHeight, m_cellWidth, m_cellHeight };
				Cell cell{ rect };
				cell.reserve(m_vertexReserve);

				cellsRow.push_back(cell);
			}
			m_cellsArray.push_back(cellsRow);
		}
	}


	void init() {
		// calculating the cell dimensions
		m_cellWidth = (m_border.x + m_border.w) / m_gridsX;
		m_cellHeight = (m_border.y + m_border.h) / m_gridsY;
		m_totalCells = m_gridsX * m_gridsY;

		// initilising the grid
		initGrid();
	}


	bool checkValidIndex(sf::Vector2i index) {
		if (index.x < 0 || index.y < 0 || index.x >= m_gridsX || index.y >= m_gridsY)
			return false;
		return true;
	}

	void RaiseOutOfBoundsError(std::string functionName) {
		throw std::out_of_range(functionName);
	}

	bool SameCellAdd(Point& point) {
		// if the point index is valid
		if (checkValidIndex(point.gridIndex) == true) {

			// get the cell and check if it is within bounds
			Cell pointCell = cellAtIndex(point.gridIndex);
			if (pointCell.rect.checkPointCollision(point.position)) {

				// if so add the point to the Cell
				cellAtIndex(point.gridIndex).add(point);
				return true;
			}
		}
		return false;
	}

	Cell& cellAtIndex(sf::Vector2i index) {
		return m_cellsArray.at(index.x).at(index.y);
	}



public:
	// constructor
	SpatialHashGrid(Rectangle border, int gridsX, int gridsY, int vertexReserve = 8) {
		m_gridsX = gridsX;
		m_gridsY = gridsY;
		m_border = border;

		m_vertexReserve = vertexReserve;
		init();
	}


	void drawGrid(sf::RenderWindow& window) {
		for (std::vector<Cell>& column : m_cellsArray) {
			for (Cell& cell : column) {
				sf::RectangleShape drawRect;
				drawRect.setOutlineThickness(1);
				drawRect.setOutlineColor(sf::Color::White);
				drawRect.setFillColor(sf::Color(0, 0, 0, 0));

				drawRect.setPosition(sf::Vector2f(cell.rect.x, cell.rect.y));
				drawRect.setSize(sf::Vector2f(cell.rect.w, cell.rect.h));
				window.draw(drawRect);
			}
		}
	}


	sf::Vector2i findCellIndex(sf::Vector2f position) {
		/* findCell function finds the cell which contains the position parameter, it iterates through
		   cellsArray which is a 2D std::vector. once found it returns a std::tuple for the indexes to find said cell
           if no cell was found it returns -1, -1 for both indexes  
		*/
		sf::Vector2i index((int)(position.x / m_cellWidth), (int)(position.y / m_cellHeight));
		
		return index;
	}


	std::vector<Point> findNear(Point& point, float visual_radius) {
		// now we need to find which cell contains the point
		sf::Vector2i cell = findCellIndex(point.position);
		return cellAtIndex(cell).container;
	}

	
	void addPoint(Point& point) {
		// if the points previous grid index is valid then we check if it has moved out of its original cell
		//if (SameCellAdd(point) == true)
		//	return;

		sf::Vector2i cellIndex = findCellIndex(point.position);

		point.updateGridIndex(cellIndex);
		cellAtIndex(cellIndex).add(point);
	}


	void addPoints(std::vector<Point>& points) {
		// reseting cells by clearing all of their items
		for (std::vector<Cell>& column : m_cellsArray) {
			for (Cell& cell : column)
				cell.clear();
		}
		
		// we then can add all the Particles one by one
		for (Point& point : points) {
			addPoint(point);
		}
	}


	void reSize(std::vector<Point>& points, int gridsX, int gridsY) {
		// we need to clear all the cells' points indexes
		for (std::vector<Cell>& column : m_cellsArray) {
			for (Cell& cell : column) {
				cell.container.clear();
			}
		}

		for (Point& point : points)
			point.gridIndex = sf::Vector2i(-1, -1);

		m_cellsArray.clear();

		m_gridsX = gridsX;
		m_gridsY = gridsY;

		init();
	}
};