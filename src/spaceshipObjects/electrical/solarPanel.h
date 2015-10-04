#ifndef SOLAR_PANEL_H
#define SOLAR_PANEL_H

#include "../spaceshipObject.h"

class SolarPanel : public SpaceshipObject, public Updatable
{
private:
    P<WiringConnection> output;
    
    float watt_seconds;
public:
    SolarPanel();
    
    virtual void update(float delta);
};

#endif//SOLAR_PANEL_H
