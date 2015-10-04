#ifndef WALL_H
#define WALL_H

#include "spaceshipObject.h"

class Wall : public SpaceshipObject
{
public:
    Wall();
    
    virtual bool isSolid();
};

#endif//WALL_H
