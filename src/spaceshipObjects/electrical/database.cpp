#include "database.h"

Database::Database()
: SpaceshipObject(sf::Vector2i(2, 3), "database.png")
{
    power = new WiringConnection("power (24V)");
    addWiringConnection(power);
    comms = new WiringConnection("communication");
    addWiringConnection(comms);
}

void Database::update(float delta)
{
    if (power->getVoltageLevel() > 20.0f)
    {
        float watt_requirement = 10.0f;
        float watt_seconds_requirement = delta * watt_requirement;
        if (power->requestWattSeconds(watt_seconds_requirement) > watt_seconds_requirement * 0.99)
        {
            string message = comms->consumeLastMessage();
            if (message.size() > 0)
            {
                std::vector<string> parts = message.split(" ");
                if (parts[0] == "READ")
                {
                    comms->sendMessage("print(\"TEST\")");
                }
            }
        }
    }
}
