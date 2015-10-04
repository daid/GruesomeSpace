#include "spaceshipObject.h"
#include "wiringConnection.h"
#include "main.h"

SpaceshipObject::SpaceshipObject(sf::Vector2i size, string texture)
: size(size)
{
    textureManager.setTexture(sprite, texture);
}

SpaceshipObject::~SpaceshipObject()
{
}

void SpaceshipObject::destroy()
{
    Renderable::destroy();
    foreach(WiringConnection, connection, wiring_connections)
    {
        connection->destroy();
    }
}

void SpaceshipObject::addWiringConnection(P<WiringConnection> connection)
{
    wiring_connections.push_back(connection);
}

sf::Vector2i SpaceshipObject::getSize()
{
    return size;
}

sf::Vector2i SpaceshipObject::getPosition()
{
    return position;
}

void SpaceshipObject::setPosition(sf::Vector2i position)
{
    this->position = position;
}

P<WiringConnection> SpaceshipObject::getWiringConnection(int index)
{
    if (index < 0 || index >= int(wiring_connections.size()))
        return nullptr;
    return wiring_connections[index];
}

bool SpaceshipObject::isSolid()
{
    return false;
}

void SpaceshipObject::render(sf::RenderTarget& window)
{
    sprite.setPosition(sf::Vector2f(position) * GridSize + sf::Vector2f(size) * GridSize * 0.5f - view_location);
    window.draw(sprite);
}
