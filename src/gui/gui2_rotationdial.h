#ifndef GUI2_ROTATION_DAIL_H
#define GUI2_ROTATION_DAIL_H

#include "gui2_element.h"

class GuiRotationDial : public GuiElement
{
public:
    typedef std::function<void(float value)> func_t;
protected:
    float min_value;
    float max_value;
    float value;
    func_t func;
    sf::Keyboard::Key up_hotkey, down_hotkey;
public:
    GuiRotationDial(GuiContainer* owner, string id, float min_value, float max_value, float start_value, func_t func);

    virtual void onDraw(sf::RenderTarget& window);
    virtual bool onMouseDown(sf::Vector2f position);
    virtual void onMouseDrag(sf::Vector2f position);
    virtual void onMouseUp(sf::Vector2f position);
    virtual bool onHotkey(sf::Keyboard::Key key, int unicode);
    
    GuiRotationDial* setValue(float value);
    float getValue();
};

#endif//GUI2_ROTATION_DAIL_H
