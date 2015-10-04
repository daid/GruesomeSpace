#ifndef GUI2_AUTO_LAYOUT_H
#define GUI2_AUTO_LAYOUT_H

#include "gui2_element.h"

class GuiAutoLayout : public GuiElement
{
public:
    enum ELayoutMode
    {
        LayoutHorizontalLeftToRight,
        LayoutHorizontalRightToLeft,
        LayoutVerticalTopToBottom,
        LayoutVerticalBottomToTop
    };
private:
    ELayoutMode mode;
public:
    GuiAutoLayout(GuiContainer* owner, string id, ELayoutMode mode);
    
    virtual void onDraw(sf::RenderTarget& window);
protected:
    virtual void drawElements(sf::FloatRect window_rect, sf::RenderTarget& window);
};

#endif//GUI2_AUTO_LAYOUT_H
