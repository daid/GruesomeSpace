#ifndef CONSTRUCTION_MENU_H
#define CONSTRUCTION_MENU_H

#include "gui/gui2.h"

class ToolBase;
class ToolCreateObject;
class ToolWireCreate;

class ConstructionMenu : public GuiCanvas
{
private:
    std::vector<ToolBase*> tools;
    std::vector<GuiButton*> tool_buttons;
    std::vector<GuiAutoLayout*> tool_groups;
    
public:
    ConstructionMenu();
    
    void addToolGroup(string name);
    void addTool(string name, ToolBase* tool);
};

#endif//CONSTRUCTION_MENU_H
