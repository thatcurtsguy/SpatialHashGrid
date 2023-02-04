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
	std::vector<T*> container{};

	void clear()
	{
		container.clear();
	}

	void reserve(const unsigned int amount)
	{
		container.reserve(amount);
	}

	void add(T* point)
	{
		container.emplace_back(point);
	}

	T* get(const unsigned int index)
	{
		return container.at(index);
	}
};

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


inline unsigned int calcCellsXY(const unsigned int pointsPerCell, const unsigned int points)
{
	return static_cast<unsigned int>(std::sqrt(points / pointsPerCell));
}