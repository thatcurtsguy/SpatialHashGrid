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

#include <ctime>
#include <string>
#include <sstream>
#include "SpatialHashGrid/spatialHashGrid.h"

int calcCellsXY(int pointsPerCell, int points) {
    return std::sqrt(points / pointsPerCell);
}

// - - global variables - sue me - - //
bool paused = false;
bool draw_grid = true;
const int particles = 50000;
const float radius = 3.0f;
const float radiusSquared = radius * radius;
const int vertexReserve = 20;
int CellsX = calcCellsXY(vertexReserve, particles);
int CellsY = calcCellsXY(vertexReserve, particles);
// - - - - - - - - - - - - - - - - - //



int randint(int start, int end) {
    return rand() % (end - start) + start;
}

float randfloat(float start, float end) {
    return (float(rand()) / float((RAND_MAX)) * (end - start)) + start;
}


void borderPoint(sf::Vector2f &position, sf::Vector2f &velocity, float borderX, float borderY, float radius) {
    if (position.x <= radius) {
        position.x = radius;
        velocity.x *= -1;
    }
    
    else if (position.x > borderX - radius) {
        position.x = borderX - radius;
        velocity.x *= -1;
    }
    
    if (position.y <= radius) {
        position.y = radius;
        velocity.y *= -1;
    }

    else if (position.y > borderY - radius) {
        position.y = borderY - radius;
        velocity.y *= -1;
    }
}


void setCaption(sf::RenderWindow& window, sf::Clock& clock) {
    // FPS management
    float timePerFrame = clock.restart().asSeconds();

    std::ostringstream oss;
    oss << "Collision Detection | MS Per Frame:" << 1.0f / timePerFrame << "";
    std::string var = oss.str();
    window.setTitle(var);
}


sf::VertexArray createRectangles(int amount, float screenWidth, float screenHeight, float size) {
    sf::VertexArray rectangles(sf::Quads, amount*4);

    for (int i = 0; i < amount; i+=4) {
        sf::Vertex topLeft{ { randfloat(0.0f, screenWidth), randfloat(0.0f, screenWidth) } }; 
        rectangles[i + 0] = topLeft;                                                            // topleft
        rectangles[i + 1] = { { topLeft.position.x + size, topLeft.position.y} };               // topright
        rectangles[i + 2] = { { topLeft.position.x + size, topLeft.position.y + size } };       // bottom right
        rectangles[i + 3] = { { topLeft.position.x, topLeft.position.y + size } };              // bottom left

    }
    return rectangles;
}


void moveShape(int pointIndex, int verticies, sf::VertexArray* rectangles, sf::Vector2f moveBy) {
    int idx = pointIndex * verticies;

    (*rectangles)[idx + 0].position += moveBy;
    (*rectangles)[idx + 1].position += moveBy;
    (*rectangles)[idx + 2].position += moveBy;
    (*rectangles)[idx + 3].position += moveBy;
}


void colorShape(int pointIndex, int verticies, sf::VertexArray* rectangles, sf::Color col) {
    /* colors a shape in a VertexArray
       pointIndex - the index of the object, usually the index of its center position in another list
       verticies  - the amount of points there are per object. e.g. Rect = 4
       col        - the color you would like the shape to be changed to
    */
    int startIdx = pointIndex * verticies;

    (*rectangles)[startIdx].color = col;
    for (int i = 1; i < verticies; i++)
        (*rectangles)[startIdx + i].color = col;
}


void setRectPosition(int pointIndex, int verticies, sf::VertexArray* vertexArray, sf::Vector2f desiredCenter) {
    int startIdx = pointIndex * 4;

    // first we need to get the current center-position of the rectangles
    sf::Vector2f sumOfPositions;
    for (int i = 0; i < verticies; i++) {
        sumOfPositions += (*vertexArray)[startIdx + i].position;
    }
    sf::Vector2f currentCenter(sumOfPositions.x / verticies, sumOfPositions.y / verticies);

    // next we need to get the change in / resultent vector
    sf::Vector2f resultantVector = desiredCenter - currentCenter;

    // finally we move the rectangles
    moveShape(pointIndex, 4, vertexArray, resultantVector);
}


void update_position(int i, sf::VertexArray* vertexArray, sf::Vector2f& position, sf::Vector2f& velocity, bool collision) {
    if (paused == false)
        position += velocity;

    if (collision == true) // TODO: only color shape if the color isnt aready selected (reduces lag)
        colorShape(i, 4, vertexArray, sf::Color(200, 50, 50));

    else if ((*vertexArray)[i].color != sf::Color(120, 120, 120))
        colorShape(i, 4, vertexArray, sf::Color(120, 120, 120));

    setRectPosition(i, 4, vertexArray, position);
}


bool rectToRectCollision(sf::Vector2f pos1, sf::Vector2f pos2, sf::Vector2f size) {
    // Doesn't work
    sf::Rect<float> rect1(pos1, size);
    sf::Rect<float> rect2(pos2, size);
    return rect1.intersects(rect2);
}


bool interactWithNearby(std::vector<Point> near, sf::Vector2f& position, int id) {
    for (Point& point : near) {
        float dx = point.position.x - position.x;
        float dy = point.position.y - position.y;
        float dSquared = dx*dx + dy*dy;

        if (dSquared < (radiusSquared * 2) and (point.id != id))
            return true;
    }
    return false;
}


int main() {
    // initilising random
    std::srand(static_cast<unsigned>(time(NULL)));

    // setting up the screen
    const float screenWidth = 1400.0f;
    const float screenHeight = 750.0f;
    sf::RenderWindow window(sf::VideoMode((int)screenWidth, (int)screenHeight), "Collision Detection!");
    sf::Clock clock;

    // setting up the spatial has grid
    Rectangle border{ 0.0f, 0.0f, screenWidth, screenHeight };
    SpatialHashGrid grid(border, CellsX, CellsY, vertexReserve);

    std::vector<sf::Vector2f> velocities(particles);
    std::vector<Point> points;
    // shift + f1

    sf::VertexArray rectangles = createRectangles(particles, screenWidth, screenHeight, radius);

    points.reserve(particles);
    const unsigned int sb = 10; // side buffer

    for (int i = 0; i < particles; i++) {
        points.emplace_back(Point{ i,
            {randfloat(sb, screenWidth - sb), randfloat(sb, screenHeight - sb)}
            });
        velocities[i] = { randfloat(-2.0f, 2.0f) , randfloat(-2.0f, 2.0f) };
    }


    // main game loop
    while (window.isOpen()) {
        // event handeler
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        window.close();
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
                        grid.reSize(points, CellsX, CellsY);
                        break;

                    case sf::Keyboard::Num2:
                        CellsX--;
                        CellsY--;
                        grid.reSize(points, CellsX, CellsY);
                        break;
                }
            }
        }

        window.clear();

        grid.addPoints(points);

        for (int i = 0; i < points.size(); i++) {
            std::vector<Point> near = grid.findNear(points[i], radius);
            int collision = interactWithNearby(near, points[i].position, points[i].id);
            update_position(i, &rectangles, points[i].position, velocities[i], collision);

            borderPoint(points[i].position, velocities[i], screenWidth, screenHeight, radius);
        }

        if (draw_grid)
                grid.drawGrid(window);

        setCaption(window, clock);
        window.draw(rectangles, sf::BlendAdd);
        window.display();
    }
}