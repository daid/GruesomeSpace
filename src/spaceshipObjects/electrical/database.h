#ifndef DATABASE_H
#define DATABASE_H

#include "../spaceshipObject.h"

class Database : public SpaceshipObject, public Updatable
{
private:
    P<WiringConnection> power;
    P<WiringConnection> comms;
public:
    Database();
    
    virtual void update(float delta);
};

#endif//LIGHT_H
