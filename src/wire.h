#ifndef WIRE_H
#define WIRE_H

#include "engine.h"

class WiringConnection;

class Wire : public Renderable
{
private:
    sf::Color color;
    P<WiringConnection> a;
    P<WiringConnection> b;
    std::vector<sf::Vector2f> wire_points;
public:
    Wire(sf::Color color);
    
    void addWirePosition(sf::Vector2f position);
    
    void connect(P<WiringConnection> a, P<WiringConnection> b);
    void disconnect();
    
    virtual void render(sf::RenderTarget& window);
    
    friend class WiringConnection;
};

#include "wiringConnection.h"

#endif//WIRE_H
