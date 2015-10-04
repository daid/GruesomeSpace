#include "main.h"
#include "spaceship.h"
#include "spaceshipObjects/spaceshipObject.h"

P<Spaceship> spaceship;

Spaceship::Spaceship()
: Renderable(floor_layer)
{
    spaceship = this;
    size = sf::Vector2i(0, 0);
}

void Spaceship::resizeTileInfo(sf::Vector2i request_size)
{
    if (request_size.x <= size.x && request_size.y <= size.y)
        return;
    
    sf::Vector2i new_size = sf::Vector2i(std::max(size.x, request_size.x), std::max(size.y, request_size.y));
    std::vector<TileInfo> old_tiles;
    old_tiles = std::move(tiles);
    tiles.clear();
    tiles.resize(new_size.x * new_size.y);
    for(int y=0; y<size.y; y++)
    {
        for(int x=0; x<size.x; x++)
        {
            tiles[x + y * new_size.x] = old_tiles[x + y * size.x];
        }
    }
    
    size = new_size;
}

void Spaceship::setFloor(sf::Vector2i position)
{
    if (position.x < 0 || position.y < 0)
        return;
    resizeTileInfo(position + sf::Vector2i(1, 1));
    tiles[position.x + position.y * size.x].has_floor = true;
}

void Spaceship::removeFloor(sf::Vector2i position)
{
    if (position.x < 0 || position.x >= size.x)
        return;
    if (position.y < 0 || position.y >= size.y)
        return;
    if (getObject(position))
        return;
    tiles[position.x + position.y * size.x].has_floor = false;
}

bool Spaceship::isFloor(sf::Vector2i position)
{
    if (position.x < 0 || position.x >= size.x)
        return false;
    if (position.y < 0 || position.y >= size.y)
        return false;
    return tiles[position.x + position.y * size.x].has_floor;
}

P<SpaceshipObject> Spaceship::getObject(sf::Vector2i position)
{
    if (position.x < 0 || position.x >= size.x)
        return nullptr;
    if (position.y < 0 || position.y >= size.y)
        return nullptr;
    return tiles[position.x + position.y * size.x].object;
}

void Spaceship::addObject(SpaceshipObject* object, sf::Vector2i position)
{
    for(int sx=0; sx<object->getSize().x; sx++)
    {
        for(int sy=0; sy<object->getSize().y; sy++)
        {
            if (getObject(sf::Vector2i(position.x + sx, position.y + sy)) || !isFloor(sf::Vector2i(position.x + sx, position.y + sy)))
            {
                object->destroy();
                return;
            }
        }
    }
    for(int sx=0; sx<object->getSize().x; sx++)
    {
        for(int sy=0; sy<object->getSize().y; sy++)
        {
            tiles[(position.x + sx) + (position.y + sy) * size.x].object = object;
        }
    }
    object->setPosition(position);
}

void Spaceship::render(sf::RenderTarget& window)
{
    sf::RectangleShape floor(sf::Vector2f(GridSize, GridSize));
    floor.setFillColor(sf::Color(32, 32, 32));
    
    for(int y=0; y<size.y; y++)
    {
        for(int x=0; x<size.x; x++)
        {
            if (tiles[x + y * size.x].has_floor)
            {
                floor.setPosition(sf::Vector2f(x * GridSize, y * GridSize) - view_location);
                window.draw(floor);
            }
        }
    }
}
