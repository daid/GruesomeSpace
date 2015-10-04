#include "wiringGroup.h"

void WiringGroup::addConnection(P<WiringConnection> connection)
{
    connections.push_back(connection);
}

void WiringGroup::update()
{
    voltage_level = 0.0;
    foreach(WiringConnection, connection, connections)
    {
        voltage_level = std::max(voltage_level, connection->getProductionVoltageLevel());
    }
}

float WiringGroup::getVoltageLevel()
{
    return voltage_level;
}

float WiringGroup::requestWattSeconds(float watt_seconds)
{
    float result = 0.0f;
    foreach(WiringConnection, connection, connections)
    {
        if (connection->watt_seconds_request_function)
        {
            result += connection->watt_seconds_request_function(watt_seconds - result);
            if (watt_seconds - result <= 0.0)
                return watt_seconds;
        }
    }
    return result;
}

void WiringGroup::sendMessage(P<WiringConnection> from_connection, string message)
{
    if (voltage_level > 0.0)
        return;
    foreach(WiringConnection, connection, connections)
    {
        if (connection != from_connection)
            connection->setLastMessage(message);
    }
}
