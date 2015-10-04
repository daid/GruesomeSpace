#ifndef SPACESHIP_H
#define SPACESHIP_H

#include "engine.h"
#include "spaceshipObjects/spaceshipObject.h"

class Spaceship;

extern P<Spaceship> spaceship;

class Spaceship : public Renderable
{
private:
    struct TileInfo
    {
        TileInfo() : has_floor(false) {}
    
        bool has_floor;
        P<SpaceshipObject> object;
    };
    
    sf::Vector2i size;
    std::vector<TileInfo> tiles;
    
    void resizeTileInfo(sf::Vector2i new_size);
public:
    Spaceship();
    
    void setFloor(sf::Vector2i position);
    void removeFloor(sf::Vector2i position);
    bool isFloor(sf::Vector2i position);
    
    P<SpaceshipObject> getObject(sf::Vector2i position);
    
    void addObject(SpaceshipObject* object, sf::Vector2i position);
    
    virtual void render(sf::RenderTarget& window);
};

#endif//SPACESHIP_H
