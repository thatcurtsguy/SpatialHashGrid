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
#include "utilities/zoomableVertexArray.hpp"
#include "utilities/generalFunctions.hpp"



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

        Entity entity(position, velocity, colorActive, colorInctive, radius,i, maxSpeed, border);
        entities.emplace_back(entity);
    }

    return entities;
}


int main()
{
    constexpr unsigned int particles = 50'000;
    constexpr unsigned int vertexReserve = 10;
    constexpr unsigned int circleSides = 5;
    constexpr unsigned int deltaGridRate = 10;

    constexpr float maxSpeed     = 0.04f;
    constexpr float entityRadius = 1.7f;
    constexpr float zoomStrength = 0.25f;

    constexpr float screenWidth  = 1920.0f;
    constexpr float screenHeight = 1080.0f;

    unsigned int CellsX = calcCellsXY(vertexReserve, particles);
    unsigned int CellsY = calcCellsXY(vertexReserve, particles);

    // initilising random
    std::srand(static_cast<unsigned>(time(nullptr)));

    // setting up the screen
    std::unique_ptr<sf::RenderWindow> window = generateWindow(screenWidth, screenHeight, "Spatial Hash Grid");
    auto clock = sf::Clock::Clock();

    ArrayOfCircles circles(particles, entityRadius, circleSides);
    sf::Rect border{ 0.0f, 0.0f, screenWidth, screenHeight };
    SpatialHashGrid<Entity> grid(border, CellsX, CellsY, vertexReserve);

    std::vector<Entity> entities = generateEntities(
        screenWidth, screenHeight, particles, entityRadius, maxSpeed, 
        { 255, 0, 0 }, { 255, 255, 255 }, border);

    // variables
    bool paused = false;
    bool draw_grid = false;
    bool mousePressed = false;
    unsigned long long frameCount = 0;

    sf::Vector2f mousePosition = getMousePositionFloat(*window);

    // Zooming
    ZoomableVertexArray zoomedCircles(&circles.m_circleArray, zoomStrength, screenWidth, screenHeight);
    ZoomableVertexArray zoomedGrid(grid.getRenderGrid(), zoomStrength, screenWidth, screenHeight);


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
                        CellsX+= deltaGridRate;
                        CellsY+= deltaGridRate;
                        grid.reSize(CellsX, CellsY);
                        break;

                    case sf::Keyboard::Num2:
                        CellsX-= deltaGridRate;
                        CellsY-= deltaGridRate;
                        grid.reSize(CellsX, CellsY);
                        break;
                }
            }

            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                auto mousePos = sf::Vector2f(sf::Mouse::getPosition(*window));
                zoomedCircles.updateMousePosition(mousePos, event.mouseWheelScroll.delta);
                zoomedGrid.updateMousePosition(mousePos, event.mouseWheelScroll.delta);
            }

            else if (event.type == sf::Event::MouseButtonPressed)
            {
                mousePressed = true;
            }

            else if (event.type == sf::Event::MouseButtonReleased)
            {
                mousePressed = false;
            }
        }

        if (mousePressed)
        {
            sf::Vector2f newPosition = getMousePositionFloat(*window);
            sf::Vector2f deltaPosition = newPosition - mousePosition;

            zoomedCircles.translate(deltaPosition);
            zoomedGrid.translate(deltaPosition);

            mousePosition = newPosition;
        }

        window->clear();

        grid.addPoints(entities);

        if (!paused)
        {
	        for (Entity& entity : entities)
	        {
	        	entity.update(circles, grid.findNear(entity.p_position, entityRadius, entity.id));
	        }
        }


        setCaption(*window, clock);
        zoomedCircles.drawVertexArray(*window, &circles.m_circleArray);
        if (draw_grid)
			zoomedGrid.drawVertexArray(*window, grid.getRenderGrid());
        window->display();

        frameCount++;
    }
}