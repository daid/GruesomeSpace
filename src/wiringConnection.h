#ifndef WIRING_CONNECTION_H
#define WIRING_CONNECTION_H

#include "engine.h"
#include <unordered_set>
#include <functional>

class Wire;
class WiringGroup;

/**
    Damn it, wiring is complex.
    
    The following features need to be supported:
    * Voltage level (for different voltage power producers and consumers)
    * Power production and consumption, as well as handling out of power situations
    * Data communication, both stream data as well as continues sensor data (sensor data can be done as voltage level)
    
    You can wire multiple wiringconnections together with wires, which makes everything a bit chaotic.
*/
class WiringConnection : public PObject
{
public:
    typedef std::function<float(float)> function_watt_seconds_request_t;
private:
    string name;
    
    string last_received_message;
    
    float producing_voltage_level;
    
    P<WiringGroup> group;
    PVector<Wire> wires;
    function_watt_seconds_request_t watt_seconds_request_function;
public:

    WiringConnection(string name);
    
    virtual void destroy();
    
    string getName();
    
    void setProducingVoltageLevel(float voltage_level);
    float getProductionVoltageLevel();
    
    float getVoltageLevel();
    
    //Request a certain amount of power from this wiring connection. Should be called from the update function of power using objects.
    // Returns the actual amount of watt seconds received.
    float requestWattSeconds(float watt_seconds);
    
    void setWattSecondsRequestFunction(function_watt_seconds_request_t function);
    
    void sendMessage(string message);
    
    void setLastMessage(string message);
    
    string consumeLastMessage();
    
    void updateGroup();

private:
    static void recursiveFindAllConnections(WiringConnection* connection, std::unordered_set<WiringConnection* >& set);

    friend class Wire;
    friend class WiringGroup;
};

#include "wire.h"
#include "wiringGroup.h"

#endif//WIRING_CONNECTION_H
