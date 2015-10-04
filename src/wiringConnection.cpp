#include <unordered_set>
#include "wiringConnection.h"

WiringConnection::WiringConnection(string name)
: name(name)
{
    producing_voltage_level = 0.0;
    
    group = new WiringGroup();
    group->addConnection(this);
}

void WiringConnection::destroy()
{
    foreach(Wire, wire, wires)
    {
        wire->disconnect();
    }
    group = nullptr;
    PObject::destroy();
}

string WiringConnection::getName()
{
    return name;
}

void WiringConnection::setProducingVoltageLevel(float voltage_level)
{
    producing_voltage_level = voltage_level;
    group->update();
}

float WiringConnection::getProductionVoltageLevel()
{
    return producing_voltage_level;
}

float WiringConnection::getVoltageLevel()
{
    return group->getVoltageLevel();
}

float WiringConnection::requestWattSeconds(float watt_seconds)
{
    return group->requestWattSeconds(watt_seconds);
}

void WiringConnection::setWattSecondsRequestFunction(function_watt_seconds_request_t function)
{
    watt_seconds_request_function = function;
}

void WiringConnection::recursiveFindAllConnections(WiringConnection* connection, std::unordered_set<WiringConnection* >& set)
{
    set.insert(connection);
    foreach(Wire, wire, connection->wires)
    {
        if (set.find(*wire->a) == set.end())
            recursiveFindAllConnections(*wire->a, set);
        if (set.find(*wire->b) == set.end())
            recursiveFindAllConnections(*wire->b, set);
    }
}

void WiringConnection::sendMessage(string message)
{
    group->sendMessage(this, message);
}

void WiringConnection::setLastMessage(string message)
{
    last_received_message = message;
}

string WiringConnection::consumeLastMessage()
{
    string ret;
    ret = last_received_message;
    last_received_message = "";
    return ret;
}

void WiringConnection::updateGroup()
{
    std::unordered_set<WiringConnection* > connections;
    recursiveFindAllConnections(this, connections);
    
    group = new WiringGroup();
    for(WiringConnection* connection : connections)
    {
        connection->group = group;
        group->addConnection(connection);
    }
    
    group->update();
}
