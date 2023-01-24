#pragma once

#include <SFML/Graphics.hpp>

#include "circles/circles.hpp"

class Entity
{
	sf::Vector2f m_velocity{};

	sf::Color m_colorActive{};
	sf::Color m_colorInactive{};

	const float  m_radius{};
	const float  m_radiusSquared{};
	const float  m_maxSpeed{};

	const sf::Rect<float>& m_border{};

public:
	sf::Vector2f p_position{};

	const unsigned int id{};
	sf::Vector2i gridIndex{ -1, -1 };


private:
	void borderCollision()
	{
		const float buffer = m_radius;

		if (p_position.x < m_border.left + buffer || p_position.x > m_border.left + m_border.width - buffer)
		{
			m_velocity.x *= -1;
		}

		if (p_position.y < m_border.top + buffer || p_position.y > m_border.top + m_border.height - buffer)
		{
			m_velocity.y *= -1;
		}

		if (p_position.x < m_border.left + buffer) 
		{
			p_position.x = m_border.left + buffer;
		}
		else if (p_position.x > m_border.left + m_border.width - buffer) 
		{
			p_position.x = m_border.left + m_border.width - buffer;
		}

		if (p_position.y < m_border.top + buffer) 
		{
			p_position.y = m_border.top + buffer;
		}
		else if (p_position.y > m_border.top + m_border.height - buffer) 
		{
			p_position.y = m_border.top + m_border.height - buffer;
		}
	}


	void updatePosition(ArrayOfCircles& allCircles, const Circle& renderCircle)
	{
		p_position += m_velocity;
		renderCircle.setPosition(allCircles.m_circleArray, p_position);
	}


	void speed_limit()
	{
		if (const float speed = sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y); speed > m_maxSpeed) 
		{
			m_velocity.x = (m_velocity.x / speed) * m_maxSpeed;
			m_velocity.y = (m_velocity.y / speed) * m_maxSpeed;
		}
	}


	bool checkNearbyCollision(const std::vector<Entity>& nearbyEntities) const
	{
		for (const Entity& entity : nearbyEntities)
		{
			// dist = sqrt(dx**2 + dy**2)
			const float dx = entity.p_position.x - p_position.x;
			const float dy = entity.p_position.y - p_position.y;

			if (const float dSquared = dx * dx + dy * dy; dSquared < (m_radiusSquared*2) and (entity.id != id))
				return true;
		}
		return false;
	}


	void interactWithNearby(const Circle& renderCircle, ArrayOfCircles& allCircles, const std::vector<Entity>& nearbyEntities) const
	{
		const bool collision = checkNearbyCollision(nearbyEntities);
		const sf::Color currentColor = renderCircle.getColor(allCircles.m_circleArray);

		if (collision == true)
		{
			if (currentColor != m_colorActive)
			{
				renderCircle.setColor(allCircles.m_circleArray, m_colorActive);
			}
		}
		else
		{
			if (currentColor != m_colorInactive) 
			{
				renderCircle.setColor(allCircles.m_circleArray, m_colorInactive);
			}
		}
	}


public:
	// constructor and destructor
	Entity(const sf::Vector2f position, const sf::Vector2f velocity, const sf::Color colorActive, const sf::Color colorInactive,
	       const float interactionRadius, const unsigned int _id, const float maxSpeed, const sf::Rect<float>& border)

	: m_velocity(velocity), m_colorActive(colorActive), m_colorInactive(colorInactive), m_radius(interactionRadius),
	  m_radiusSquared(m_radius * m_radius), m_maxSpeed(maxSpeed), m_border(border), p_position(position), id(_id) {}

	~Entity() = default;


	void update(ArrayOfCircles& allCircles, const std::vector<Entity>& nearbyEntities)
	{
		const Circle& renderCircle = allCircles.m_circles[id];

		interactWithNearby(renderCircle, allCircles, nearbyEntities);
		speed_limit();
		updatePosition(allCircles, renderCircle);
		borderCollision();
	}

	sf::Vector2f getPosition() const
	{
		return p_position;
	}
};