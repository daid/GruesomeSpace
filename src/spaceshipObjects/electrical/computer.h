#ifndef COMPUTER_H
#define COMPUTER_H

#include "../spaceshipObject.h"
#include "lua/lua.hpp"

/**
    Computers are 1x1 objects that run specific code.
    
    They need power to run. They run a piece of bootloader ROM to receive other code.
    Computers have an ingame visual interface at which the code can draw and receive input from.
*/
class Computer : public SpaceshipObject, public Updatable
{
private:
    static constexpr float IdleWattUsage = 5.0f;
    static constexpr float RunningWattUsage = 35.0f;
    static constexpr int IOPortCount = 32;
    static constexpr int ScreenWidth = 40;
    static constexpr int ScreenHeight = 25;
    
    lua_State* L;

    enum State
    {
        PoweredDown,
        Operational,
        BrokenDown,
    };
    enum RunState
    {
        Running,
        Sleep,
        WaitingForMessage
    };
    
    State state;
    RunState run_state;
    float sleep_time;
    uint32_t io_port_selection;
    
    P<WiringConnection> power;
    P<WiringConnection> io_port[IOPortCount];
    
    char screen[ScreenWidth*ScreenHeight];
    int print_position;
public:
    Computer();
    virtual ~Computer();
    
    virtual void update(float delta);

    virtual void render(sf::RenderTarget& window);
private:
    void powerDown();
    void haltExecution();
    void createExecutionEnvironment();
    void resumeExecution(int nr_arg=0);

    void clearScreen();
    void print(const char* str);
private:
    static Computer* current_computer;
    static int luaSleep(lua_State* L);
    static int luaClearScreen(lua_State* L);
    static int luaPrint(lua_State* L);
    static int luaSetPrintXY(lua_State* L);
    static int luaSendToConnection(lua_State* L);
    static int luaReadFromConnection(lua_State* L);
    static void luaCyclesCallback(lua_State *L, lua_Debug *ar);
};

#endif//COMPUTER_H
