#pragma once


template <typename T>
struct Cell
{
	explicit Cell(const sf::Rect<float>& Rect, const unsigned int vertexReserve) : rect(Rect)
	{
		reserve(vertexReserve);
	}
	~Cell() = default;

	sf::Rect<float> rect{}; 
	std::vector<T> container;

	void clear()
	{
		container.clear();
	}

	void reserve(const int amount)
	{
		container.reserve(amount);
	}

	void add(T& point)
	{
		container.emplace_back(point);
	}
};