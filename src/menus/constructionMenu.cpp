#include "main.h"
#include "constructionMenu.h"
#include "spaceship.h"
#include "spaceshipObjects/wall.h"
#include "spaceshipObjects/electrical/computer.h"
#include "spaceshipObjects/electrical/light.h"
#include "spaceshipObjects/electrical/database.h"

class ToolBase : public GuiElement
{
public:
    ToolBase(GuiContainer* owner, string id)
    : GuiElement(owner, id)
    {
    }
    
    virtual void resetTool()
    {
    }
};

class ToolAreaSelect : public ToolBase
{
private:
    bool mouse_down;
    sf::Vector2f mouse_down_position;
    sf::Vector2f mouse_position;
public:
    ToolAreaSelect(GuiContainer* owner, string id)
    : ToolBase(owner, id)
    {
        setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
        setPosition(0, 0, ATopLeft);
        hide();
        mouse_down = false;
    }

    virtual bool onMouseDown(sf::Vector2f position)
    {
        mouse_down_position = position + view_location;
        mouse_position = position + view_location;
        mouse_down = true;
        return true;
    }

    virtual void onMouseDrag(sf::Vector2f position)
    {
        mouse_position = position + view_location;
    }

    virtual void onMouseUp(sf::Vector2f position)
    {
        sf::Vector2i p0 = sf::Vector2i(mouse_down_position / GridSize);
        sf::Vector2i p1 = sf::Vector2i(mouse_position / GridSize);
        if (p1.x < p0.x) std::swap(p1.x, p0.x);
        if (p1.y < p0.y) std::swap(p1.y, p0.y);
        
        for(int y=p0.y; y<=p1.y; y++)
        {
            for(int x=p0.x; x<=p1.x; x++)
            {
                onAreaSelect(sf::Vector2i(x, y));
            }
        }

        mouse_down = false;
    }
    
    virtual void onDraw(sf::RenderTarget& window)
    {
        if (!mouse_down)
        {
            mouse_down_position = InputHandler::getMousePos() + view_location;
            mouse_position = mouse_down_position;
        }
        
        sf::Vector2i p0 = sf::Vector2i(mouse_down_position / GridSize);
        sf::Vector2i p1 = sf::Vector2i(mouse_position / GridSize);
        if (p1.x < p0.x) std::swap(p1.x, p0.x);
        if (p1.y < p0.y) std::swap(p1.y, p0.y);
        
        sf::RectangleShape rect(sf::Vector2f((p1.x - p0.x + 1) * GridSize, (p1.y - p0.y + 1) * GridSize));
        rect.setOutlineColor(sf::Color(255, 255, 255, 128));
        rect.setOutlineThickness(3);
        rect.setFillColor(sf::Color::Transparent);
        rect.setPosition(sf::Vector2f(p0) * GridSize - view_location);
        window.draw(rect);
    }
    
    virtual void onAreaSelect(sf::Vector2i position) = 0;
};

class ToolLocationSelect : public ToolBase
{
private:
    sf::Vector2i selection_size;
public:
    ToolLocationSelect(GuiContainer* owner, string id)
    : ToolBase(owner, id)
    {
        setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
        setPosition(0, 0, ATopLeft);
        hide();
        
        selection_size = sf::Vector2i(1, 1);
    }
    
    void setSelectionSize(sf::Vector2i size)
    {
        selection_size = size;
    }

    virtual bool onMouseDown(sf::Vector2f position)
    {
        return true;
    }

    virtual void onMouseDrag(sf::Vector2f position)
    {
    }

    virtual void onMouseUp(sf::Vector2f position)
    {
        sf::Vector2i p = sf::Vector2i((position + view_location) / GridSize - sf::Vector2f(selection_size - sf::Vector2i(1, 1)) / 2.0f);
        onPositionSelect(p);
    }
    
    virtual void onDraw(sf::RenderTarget& window)
    {
        sf::Vector2f mouse_position = InputHandler::getMousePos();
        
        sf::Vector2i p = sf::Vector2i((mouse_position + view_location) / GridSize - sf::Vector2f(selection_size - sf::Vector2i(1, 1)) / 2.0f);
        
        sf::RectangleShape rect(sf::Vector2f(selection_size) * GridSize);
        rect.setOutlineColor(sf::Color(255, 255, 255, 128));
        rect.setOutlineThickness(3);
        rect.setFillColor(sf::Color::Transparent);
        rect.setPosition(sf::Vector2f(p) * GridSize - view_location);
        window.draw(rect);
    }
    
    virtual void onPositionSelect(sf::Vector2i position) = 0;
};

class ToolAddFloor : public ToolAreaSelect
{
public:
    ToolAddFloor(GuiContainer* owner, string id)
    : ToolAreaSelect(owner, id)
    {
    }
    
    virtual void onAreaSelect(sf::Vector2i position) override
    {
        spaceship->setFloor(position);
    }
};

class ToolRemoveFloor : public ToolAreaSelect
{
public:
    ToolRemoveFloor(GuiContainer* owner, string id)
    : ToolAreaSelect(owner, id)
    {
    }
    
    virtual void onAreaSelect(sf::Vector2i position) override
    {
        spaceship->removeFloor(position);
    }
};

class ToolCreateObject : public ToolLocationSelect
{
private:
    std::function<SpaceshipObject*()> create_function;
public:
    ToolCreateObject(GuiContainer* owner, string id, sf::Vector2i size, std::function<SpaceshipObject*()> create_function)
    : ToolLocationSelect(owner, id)
    {
        setSelectionSize(size);
        this->create_function = create_function;
    }
    
    virtual void onPositionSelect(sf::Vector2i position) override
    {
        spaceship->addObject(create_function(), position);
    }
};
class ToolWireCreate : public ToolLocationSelect
{
private:
    std::vector<sf::Vector2f> wire_positions;
    P<WiringConnection> a;
    P<WiringConnection> b;
    enum State
    {
        SelectPointA,
        SelectConnectionA,
        BuildWire,
        SelectConnectionB
    };
    State state;
    
    GuiListbox* connection_list;
public:
    ToolWireCreate(GuiContainer* owner, string id)
    : ToolLocationSelect(owner, id)
    {
        connection_list = new GuiListbox(this, "", [this](int index, string value)
        {
            switch(state)
            {
            case SelectConnectionA:
                a = spaceship->getObject(sf::Vector2i(wire_positions[0]))->getWiringConnection(index);
                state = BuildWire;
                break;
            case SelectConnectionB:
                b = spaceship->getObject(sf::Vector2i(wire_positions.back()))->getWiringConnection(index);
                finalizeWire();
                break;
            default:
                break;
            }
            connection_list->hide();
        });
        connection_list->setSize(300, 600)->setPosition(100, 100, ATopLeft);
        
        resetTool();
    }
    
    virtual void resetTool()
    {
        state = SelectPointA;
        wire_positions.clear();
        a = nullptr;
        b = nullptr;
        connection_list->hide();
    }
    
    virtual void onPositionSelect(sf::Vector2i position) override
    {
        switch(state)
        {
        case SelectPointA:
            if (spaceship->getObject(position) && spaceship->getObject(position)->getWiringConnection(0))
            {
                if (addWireToPoint(position))
                {
                    connection_list->setOptions({});
                    for(int n=0; spaceship->getObject(position)->getWiringConnection(n); n++)
                    {
                        connection_list->addEntry(spaceship->getObject(position)->getWiringConnection(n)->getName(), "");
                    }
                    connection_list->show();
                    state = SelectConnectionA;
                }
            }
            break;
        case SelectConnectionA:
            resetTool();
            break;
        case BuildWire:
            if (addWireToPoint(position))
            {
                if (spaceship->getObject(position) && spaceship->getObject(position)->getWiringConnection(0))
                {
                    connection_list->setOptions({});
                    for(int n=0; spaceship->getObject(position)->getWiringConnection(n); n++)
                    {
                        connection_list->addEntry(spaceship->getObject(position)->getWiringConnection(n)->getName(), "");
                    }
                    connection_list->show();
                    state = SelectConnectionB;
                }
            }
            break;
        case SelectConnectionB:
            resetTool();
            break;
        }
    }
    
    sf::Vector2f getNextWirePoint(sf::Vector2i position)
    {
        if (wire_positions.size() == 0)
        {
            return sf::Vector2f(position.x + random(0.3, 0.7), position.y + random(0.3, 0.7));
        }
        
        sf::Vector2i last_position = sf::Vector2i(wire_positions.back());
        if (last_position.x == position.x)
        {
            return sf::Vector2f(wire_positions.back().x, position.y + random(0.3, 0.7));
        }
        if (last_position.y == position.y)
        {
            return sf::Vector2f(position.x + random(0.3, 0.7), wire_positions.back().y);
        }
        if (abs(last_position.x - position.x) < abs(last_position.y - position.y))
        {
            return sf::Vector2f(wire_positions.back().x, position.y + random(0.3, 0.7));
        }else{
            return sf::Vector2f(position.x + random(0.3, 0.7), wire_positions.back().y);
        }
    }
    
    bool addWireToPoint(sf::Vector2i position)
    {
        sf::Vector2f new_point = getNextWirePoint(position);
        wire_positions.push_back(new_point);
        return sf::Vector2i(new_point) == position;
    }
    
    void finalizeWire()
    {
        if (a != b)
        {
            P<Wire> w = new Wire(sf::Color::White);
            for(sf::Vector2f pos : wire_positions)
                w->addWirePosition(pos);
            w->connect(a, b);
        }
        resetTool();
    }

    virtual void onDraw(sf::RenderTarget& window)
    {
        ToolLocationSelect::onDraw(window);
        sf::VertexArray a(sf::LinesStrip, wire_positions.size() + ((state == BuildWire) ? 1 : 0));
        if (a.getVertexCount() < 2)
            return;
        for(unsigned int n=0; n<wire_positions.size(); n++)
        {
            a[n].position = wire_positions[n] * GridSize - view_location;
            a[n].color = sf::Color::Yellow;
        }
        if (state == BuildWire)
        {
            a[wire_positions.size()].position = getNextWirePoint(sf::Vector2i((InputHandler::getMousePos() + view_location) / GridSize)) * GridSize - view_location;
        }
        window.draw(a);
    }
};

ConstructionMenu::ConstructionMenu()
{
    addToolGroup("Floor");
    addTool("Add floor", new ToolAddFloor(this, ""));
    addTool("Remove floor", new ToolRemoveFloor(this, ""));
    addToolGroup("Add objects");
    addTool("Add wall", new ToolCreateObject(this, "", sf::Vector2i(1, 1), []() { return new Wall(); }));
    addTool("Add computer", new ToolCreateObject(this, "", sf::Vector2i(2, 2), []() { return new Computer(); }));
    addTool("Add light", new ToolCreateObject(this, "", sf::Vector2i(1, 1), []() { return new Light(); }));
    addTool("Add database", new ToolCreateObject(this, "", sf::Vector2i(2, 3), []() { return new Database(); }));
    addToolGroup("Wiring");
    addTool("Add wiring", new ToolWireCreate(this, ""));
    
    for(GuiAutoLayout* layout : tool_groups)
        layout->moveToFront();
    for(GuiButton* button : tool_buttons)
        button->moveToFront();
}

void ConstructionMenu::addToolGroup(string name)
{
    unsigned int n = tool_groups.size();
    GuiButton* button = new GuiButton(this, "", name, [this, n]() {
        for(ToolBase* tool : tools)
        {
            tool->hide();
            tool->resetTool();
        }
        for(GuiAutoLayout* layout : tool_groups)
        {
            layout->hide();
        }
        tool_groups[n]->show();
    });
    button->setPosition(sf::Vector2f(20 + 300 * n, 20), ATopLeft)->setSize(300, 50);
    tool_buttons.push_back(button);
    
    GuiAutoLayout* layout = new GuiAutoLayout(this, "", GuiAutoLayout::LayoutVerticalTopToBottom);
    layout->setPosition(20 + 300 * n, 70)->setSize(300, GuiElement::GuiSizeMax);
    layout->hide();
    tool_groups.push_back(layout);
}

void ConstructionMenu::addTool(string name, ToolBase* tool)
{
    GuiButton* button = new GuiButton(tool_groups.back(), "", name, [this, tool]()
    {
        for(GuiAutoLayout* layout : tool_groups)
        {
            layout->hide();
        }
        tool->show();
    });
    button->setSize(GuiElement::GuiSizeMax, 50);
    tool_buttons.push_back(button);
    
    tools.push_back(tool);
}
