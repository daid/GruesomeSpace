#ifndef WIRING_GROUP_H
#define WIRING_GROUP_H

#include "engine.h"

class WiringConnection;

/**
    Wiring groups are created and maintained by WiringConnections.
    A WiringGroup has a reference to all the WiringConnections that are hooked together with Wires.
*/
class WiringGroup : virtual public PObject
{
private:
    float voltage_level;
    PVector<WiringConnection> connections;

public:
    void addConnection(P<WiringConnection> connection);

    void update();
    
    float getVoltageLevel();
    float requestWattSeconds(float watt_seconds);
    
    void sendMessage(P<WiringConnection> from_connection, string message);
};

#include "wiringConnection.h"

#endif//WIRING_GROUP_H
