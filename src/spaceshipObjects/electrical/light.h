#ifndef LIGHT_H
#define LIGHT_H

#include "../spaceshipObject.h"

class Light : public SpaceshipObject, public Updatable
{
private:
    P<WiringConnection> power;
public:
    Light();
    
    virtual void update(float delta);
};

#endif//LIGHT_H
