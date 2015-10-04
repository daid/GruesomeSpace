#include "wall.h"

Wall::Wall()
: SpaceshipObject(sf::Vector2i(1, 1), "wall.png")
{
}

bool Wall::isSolid()
{
    return true;
}
