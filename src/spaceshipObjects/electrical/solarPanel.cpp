#include "solarPanel.h"

SolarPanel::SolarPanel()
: SpaceshipObject(sf::Vector2i(4, 4), "solarpanel.png")
{
    watt_seconds = 0.0f;

    output = new WiringConnection("output (24V)");
    addWiringConnection(output);
    
    output->setProducingVoltageLevel(24.0f);
    output->setWattSecondsRequestFunction([this](float request)
    {
        if (output->getProductionVoltageLevel() < 20.0f)
            return 0.0f;
        watt_seconds -= request;
        if (watt_seconds < 0.0f)
        {
            request += watt_seconds;
            watt_seconds = 0.0f;
        }
        return request;
    });
}

void SolarPanel::update(float delta)
{
    if (output->getVoltageLevel() > 42.0f)
    {
        //Someone is feeding a lot of power into the solar panels. Guess what... it breaks then.
        //TODO: Feedback on the fact that this happend.
        output->setProducingVoltageLevel(0.0f);
    }else{
        watt_seconds = delta * 300; //Producing 300W of power, but not storing any power.
    }
}
