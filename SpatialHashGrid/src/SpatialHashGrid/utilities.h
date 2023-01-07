#pragma once
#include <iostream>

struct Rectangle { // TODO: either use sf::rect or this
	float x{};
	float y{};
	float w{};
	float h{};

	bool checkPointCollision(sf::Vector2f position) {
		return position.x >= x and position.x < (x + w) and position.y >= y and position.y < (y + h);
	}
};


struct Point {
	using Id = int;

	Id id{};
	sf::Vector2f position{};
	sf::Vector2i gridIndex{-1, -1};

	void updateGridIndex(sf::Vector2i newIndex) {
		gridIndex = newIndex;
	}
};


struct Cell {
	Rectangle rect{}; 
	std::vector<Point> container{};

	void clear() {
		container.clear();
	}

	void reserve(int amount) {
		container.reserve(amount);
	}

	void add(Point& point) {
		container.emplace_back(point);
	}
};