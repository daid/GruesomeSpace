#ifndef GUI2_ELEMENT_H
#define GUI2_ELEMENT_H

#include <functional>
#include "stringImproved.h"
#include "gui2_container.h"

enum EGuiAlign
{
    ATopLeft,
    ATopRight,
    ATopCenter,
    ACenterLeft,
    ACenterRight,
    ACenter,
    ABottomLeft,
    ABottomRight,
    ABottomCenter
};

class GuiElement : public GuiContainer
{
private:
    sf::Vector2f position;
    sf::Vector2f size;
    EGuiAlign position_alignment;
protected:
    GuiContainer* owner;
    sf::FloatRect rect;
    bool visible;
    bool enabled;
    bool hover;
    bool focus;
    string id;
public:
    constexpr static float GuiSizeMatchHeight = -1.0;
    constexpr static float GuiSizeMatchWidth = -1.0;
    constexpr static float GuiSizeMax = -2.0;

    GuiElement(GuiContainer* owner, string id);
    virtual ~GuiElement();

    virtual void onDraw(sf::RenderTarget& window) {}
    virtual bool onMouseDown(sf::Vector2f position);
    virtual void onMouseDrag(sf::Vector2f position);
    virtual void onMouseUp(sf::Vector2f position);
    virtual bool onKey(sf::Keyboard::Key key, int unicode);
    virtual bool onHotkey(sf::Keyboard::Key key, int unicode);
    virtual bool onJoystickXYMove(sf::Vector2f position);
    virtual bool onJoystickZMove(float position);
    virtual bool onJoystickRMove(float position);
    
    GuiElement* setSize(sf::Vector2f size);
    GuiElement* setSize(float x, float y);
    sf::Vector2f getSize();
    GuiElement* setPosition(float x, float y, EGuiAlign alignment = ATopLeft);
    GuiElement* setPosition(sf::Vector2f position, EGuiAlign alignment = ATopLeft);
    sf::Vector2f getPositionOffset();
    GuiElement* setVisible(bool visible);
    GuiElement* hide();
    GuiElement* show();
    bool isVisible();
    GuiElement* setEnable(bool enable);
    GuiElement* enable();
    GuiElement* disable();
    bool isEnabled();
    
    void moveToFront();
    void moveToBack();
    
    sf::Vector2f getCenterPoint();
    
    friend class GuiContainer;
    friend class GuiCanvas;
private:
    void updateRect(sf::FloatRect window_rect);
protected:
    /*!
     * Draw a certain text on the screen with horizontal orientation.
     * \param rect Area to draw in
     * \param align Alighment of text.
     * \param text_size Size of the text
     * \param color Color of text
     */
    void drawText(sf::RenderTarget& window, sf::FloatRect rect, string text, EGuiAlign align = ATopLeft, float text_size = 30, sf::Color color=sf::Color::White);

    /*!
     * Draw a certain text on the screen with vertical orientation
     * \param rect Area to draw in
     * \param align Alighment of text.
     * \param text_size Size of the text
     * \param color Color of text
     */
    void drawVerticalText(sf::RenderTarget& window, sf::FloatRect rect, string text, EGuiAlign align = ATopLeft, float text_size = 30, sf::Color color=sf::Color::White);

    void draw9Cut(sf::RenderTarget& window, sf::FloatRect rect, string texture, sf::Color color=sf::Color::White, float width_factor = 1.0);
    
    void drawArrow(sf::RenderTarget& window, sf::FloatRect rect, sf::Color=sf::Color::White, float rotation=0);
};

#endif//GUI2_ELEMENT_H
