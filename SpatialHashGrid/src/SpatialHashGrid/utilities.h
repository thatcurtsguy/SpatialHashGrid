#pragma once


struct Cell
{
	unsigned int m_vertexReserve;
	sf::Rect<float> rect;
    std::vector<unsigned int> container;

    explicit Cell(const sf::Rect<float>& rect = { 0, 0, 0, 0 }, const unsigned int vertexReserve = 0)
        : m_vertexReserve(vertexReserve), rect(rect)
    {
        reserve(vertexReserve);
    }

    void clear()
    {
        container.clear();
    }

    void reserve(const std::size_t amount)
    {
        container.reserve(amount);
    }

    void add(const unsigned int pointIndex)
    {
        container.emplace_back(pointIndex);
    }

    unsigned int get(const std::size_t vectorIndex) const
    {
        return container[vectorIndex];
    }
};


inline unsigned int calcCellsXY(const unsigned int pointsPerCell, const unsigned int points)
{
	return static_cast<unsigned int>(std::sqrt(points / pointsPerCell));
}