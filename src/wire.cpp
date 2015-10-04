#include "main.h"
#include "wire.h"
#include "spaceship.h"

Wire::Wire(sf::Color color)
: Renderable(wiring_layer), color(color)
{
}

void Wire::addWirePosition(sf::Vector2f position)
{
    wire_points.push_back(position);
}

void Wire::connect(P<WiringConnection> a, P<WiringConnection> b)
{
    if (this->a)
        return;
    if (!a || !b)
        return;
    if (a == b)
        return;
    
    this->a = a;
    this->b = b;
    
    this->a->wires.push_back(this);
    this->b->wires.push_back(this);
    
    this->a->updateGroup();
}

void Wire::disconnect()
{
    if (a)
        a->wires.remove(this);
    if (b)
        b->wires.remove(this);
    if (a)
        a->updateGroup();
    if (b)
        b->updateGroup();
    a = nullptr;
    b = nullptr;
}

void Wire::render(sf::RenderTarget& window)
{
    if (wire_points.size() < 2)
        return;
    sf::VertexArray a(sf::LinesStrip, wire_points.size());
    for(unsigned int n=0; n<wire_points.size(); n++)
    {
        a[n].position = wire_points[n] * GridSize - view_location;
        a[n].color = color;
    }
    window.draw(a);
}
