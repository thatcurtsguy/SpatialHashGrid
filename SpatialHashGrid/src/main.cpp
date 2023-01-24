/*
    Makes a basic collision detection simulation Using a spatial Hash grid

    keys:
    > space - pause
    > Esc   - close
    > 1     - grid size +
    > 2     - grid size -
*/

#include <iostream>
#include <SFML/Graphics.hpp>

#include <vector>
#include <ctime>
#include <string>
#include <sstream>
#include "SpatialHashGrid/spatialHashGrid.h"
#include "circles/circles.hpp"
#include "entity.hpp"


unsigned int calcCellsXY(const unsigned int pointsPerCell, const unsigned int points)
{
    return static_cast<unsigned int>(std::sqrt(points / pointsPerCell));
}


int randint(const unsigned int start, const unsigned int end)
{
    return rand() % (end - start) + start;
}

float randfloat(const float start, const float end)
{
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (end - start)) + start;
}


void setCaption(sf::RenderWindow& window, sf::Clock& clock)
{
    // FPS management
    const float timePerFrame = clock.restart().asSeconds();

    std::ostringstream oss;
    oss << "Collision Detection | MS Per Frame:" << 1.0f / timePerFrame << "";
    const std::string var = oss.str();
    window.setTitle(var);
}


std::unique_ptr<sf::RenderWindow> generateWindow(const float screenWidth, const float screenHeight, const std::string& name)
{
    auto window = std::make_unique<sf::RenderWindow>(sf::VideoMode(static_cast<int>(screenWidth), static_cast<int>(screenHeight)), name);
    window->setFramerateLimit(144);
    window->setVerticalSyncEnabled(false);
    window->resetGLStates();

    return window;
}

std::vector<Entity> generateEntities(const float screenWidth, const float screenHeight, const unsigned int amount, const float radius, 
    const float maxSpeed, const sf::Color colorActive, const sf::Color colorInctive, const sf::Rect<float>& border)
{
    std::vector<Entity> entities;
    entities.reserve(amount);

    for (size_t i = 0; i < amount; i++)
    {
	    const sf::Vector2f position = { randfloat(radius, screenWidth - radius), randfloat(radius, screenHeight - radius) };
	    const sf::Vector2f velocity = { randfloat(-2.0f, 2.0f) , randfloat(-2.0f, 2.0f) };

        Entity entity(position, velocity, colorActive, colorInctive, radius, static_cast<unsigned int>(i), maxSpeed, border);
        entities.emplace_back(entity);
    }

    return entities;
}

int main()
{
    constexpr unsigned int particles = 50'000;
    constexpr float radius = 3;
    constexpr unsigned int vertexReserve = 10;
    constexpr unsigned int circleSides = 4;
    constexpr float maxSpeed = 1;

    // initilising random
    std::srand(static_cast<unsigned>(time(nullptr)));

    unsigned int CellsX = calcCellsXY(vertexReserve, particles);
    unsigned int CellsY = calcCellsXY(vertexReserve, particles);

    // setting up the screen
    constexpr float screenWidth = 1920.0f;
    constexpr float screenHeight = 1080.0f;

    auto clock = sf::Clock::Clock();

    ArrayOfCircles circles(particles, radius, circleSides);
    sf::Rect<float> border{ 0.0f, 0.0f, screenWidth, screenHeight };
    SpatialHashGrid<Entity> grid(border, CellsX, CellsY, vertexReserve);

    sf::Color colorActive = { 255, 0, 0 };
    sf::Color colorInctive = { 255, 255, 255 };

    std::unique_ptr<sf::RenderWindow> window = generateWindow(screenWidth, screenHeight, "Collision Detection!");

    std::vector<Entity> entities = generateEntities(screenWidth, screenHeight, particles, radius, maxSpeed, colorActive, colorInctive, border);

    bool paused = false;
    bool draw_grid = true;


    // main game loop
    while (window->isOpen()) 
    {
        // event handeler
        sf::Event event{};
        while (window->pollEvent(event)) 
        {
            if (event.type == sf::Event::Closed)
                window->close();

            else if (event.type == sf::Event::KeyPressed) 
            {
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        window->close();
                        break;

                    case sf::Keyboard::Space:
                        paused = not paused;
                        break;

                    case sf::Keyboard::G:
                        draw_grid = not draw_grid;
                        break;

                    case sf::Keyboard::Num1:
                        CellsX++;
                        CellsY++;
                        grid.reSize(CellsX, CellsY);
                        break;

                    case sf::Keyboard::Num2:
                        CellsX--;
                        CellsY--;
                        grid.reSize(CellsX, CellsY);
                        break;
                }
            }
        }

        window->clear();

        grid.addPoints(entities);

        if (!paused)
        {
	        for (Entity& entity : entities)
	        {
	        	entity.update(circles, grid.findNear(entity));
	        }
        }

        if (draw_grid)
        	grid.drawGrid(*window);

        setCaption(*window, clock);
        window->draw(circles.m_circleArray, sf::BlendAdd);
        window->display();
    }
}