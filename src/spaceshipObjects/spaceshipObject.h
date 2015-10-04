#ifndef SPACESHIP_OBJECT_H
#define SPACESHIP_OBJECT_H

#include "engine.h"

#include "wiringConnection.h"

static constexpr float GridSize = 16.0f;
/**
    Spaceship objects are objects placed somewhere on the spaceship floorplan.
    Only a single object can occupy a space at a time.
    
    Objects are things like:
    * Walls
    * Lights
    * Sensors
    * Computers
    * Doors
    * Engines
    * Batteries
    * Solar panels
    * Power switches
    * Transformers (change high voltage to low voltage power)
    
    The following features are supported by SpaceshipObjects:
    * Varing sizes
    * Solid or not (solid objects block airflow, and are generally walls and doors)
    * Wiring connections (power, input, output)
*/
class SpaceshipObject : public Renderable
{
private:
    sf::Vector2i position;
    sf::Vector2i size;
    PVector<WiringConnection> wiring_connections;
protected:
    sf::Sprite sprite;
    
    SpaceshipObject(sf::Vector2i size, string texture);
    
    void addWiringConnection(P<WiringConnection> connection);
public:
    virtual ~SpaceshipObject();
    
    virtual void destroy();

    sf::Vector2i getSize();
    sf::Vector2i getPosition();
    void setPosition(sf::Vector2i position);
    
    P<WiringConnection> getWiringConnection(int index);

    //Solid objects block airflow, fluids, and the vacuum of space and everything else. Wires can still be put trough it tough.
    //Is used to calculate which areas are seperate rooms and thus have their own internal contents (pressure, fluid level, content mixture)
    virtual bool isSolid();

    virtual void render(sf::RenderTarget& window);
};

#endif//SPACESHIP_OBJECT_H
