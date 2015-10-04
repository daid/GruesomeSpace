#include "light.h"

Light::Light()
: SpaceshipObject(sf::Vector2i(1, 1), "wall.png")
{
    power = new WiringConnection("power (24V)");
    addWiringConnection(power);
}

void Light::update(float delta)
{
    if (power->getVoltageLevel() > 20.0f)
    {
        float watt_requirement = 30.0f;
        float f = power->requestWattSeconds(delta * watt_requirement) / watt_requirement / delta;
        sprite.setColor(sf::Color(255 * f, 255 * f, 255 * f));
    }else{
        sprite.setColor(sf::Color::Black);
    }
}
