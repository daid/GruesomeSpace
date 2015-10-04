#include "computer.h"

constexpr const char* BootCode = R"EOF(
    while true do
        clearscreen()
        print("Booting...\n")
        sleep(0.1)
        sendtoconnection(0, "READ BOOT")
        script_data = readfromconnection(0)
        if script_data == nil then
            print("No response for READ BOOT command\n")
        else
            script, error = load(script_data, "BOOT")
            if script ~= nil then
                result, error = pcall(script)
                if result then
                    print("\nCode finished.\nRebooting...\n")
                    sleep(1.0)
                else
                    print("\nError running boot code:\n", error, "\n")
                    sleep(5.0)
                end
            else
                print("\nError loading code:\n", error, "\n")
                sleep(5.0)
            end
        end
    end
)EOF";

Computer::Computer()
: SpaceshipObject(sf::Vector2i(2, 2), "computer.png")
{
    L = nullptr;
    
    power = new WiringConnection("power (24V)");
    addWiringConnection(power);
    
    for(int n=0; n<IOPortCount; n++)
    {
        io_port[n] = new WiringConnection("IO (" + string(n) + ")");
        addWiringConnection(io_port[n]);
    }
    
    clearScreen();
    
    powerDown();
}

Computer::~Computer()
{
    haltExecution();
}

void Computer::update(float delta)
{
    if (power->getVoltageLevel() > 28.0f)
    {
        //Too much voltage, computer say no!
        if (state != BrokenDown)
        {
            state = BrokenDown;
            haltExecution();
        }
    }else if (power->getVoltageLevel() < 22.0f)
    {
        if (state == Operational)
            powerDown();
    }else{
        float watt_seconds_request = IdleWattUsage * delta;
        if (run_state == Running)
            watt_seconds_request = RunningWattUsage * delta;
        float watt_seconds_received = power->requestWattSeconds(watt_seconds_request);
        
        if (watt_seconds_received < watt_seconds_request * 0.99f)
        {
            if (state == Operational)
                powerDown();
        }else{
            if (state == PoweredDown)
            {
                createExecutionEnvironment();
            }
        }
    }
    
    if (L)
    {
        switch(run_state)
        {
        case Running:
            resumeExecution();
            break;
        case Sleep:
            sleep_time -= delta;
            if (sleep_time <= 0.0)
            {
                run_state = Running;
                resumeExecution();
            }
            break;
        case WaitingForMessage:
            for(int n=0; n<IOPortCount; n++)
            {
                if (io_port_selection & (1 << n))
                {
                    string message = io_port[n]->consumeLastMessage();
                    if (message.size() > 0)
                    {
                        run_state = Running;
                        lua_pushstring(L, message.c_str());
                        resumeExecution(1);
                        break;
                    }
                }
            }
            sleep_time -= delta;
            if (run_state != Running && sleep_time <= 0.0)
            {
                run_state = Running;
                resumeExecution();
            }
            break;
        }
    }
}

void Computer::render(sf::RenderTarget& window)
{
//*
    textureManager.getTexture("font.png", sf::Vector2i(16, 6));
    for(int y=0;y<ScreenHeight;y++)
    {
        for(int x=0;x<ScreenWidth;x++)
        {
            char c = screen[x + y * ScreenWidth];
            if (c >= 32 && c < 32 + 16 * 6)
            {
                sf::Sprite sprite;
                textureManager.setTexture(sprite, "font.png", c - 32);
                sprite.setPosition(x * 18, y * 18);
                sprite.setOrigin(0, 0);
                sprite.setScale(sprite.getScale() * 2.0f);
                window.draw(sprite);
            }
        }
    }
//*/
    SpaceshipObject::render(window);
}

void Computer::powerDown()
{
    sprite.setColor(sf::Color(192, 192, 192));
    haltExecution();
    state = PoweredDown;
}

void Computer::haltExecution()
{
    if (L)
    {
        lua_close(L);
    }
    L = nullptr;
}

static const luaL_Reg loadedlibs[] = {
  {"_G", luaopen_base},
//  {LUA_LOADLIBNAME, luaopen_package},
//  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
//  {LUA_IOLIBNAME, luaopen_io},
//  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
//  {LUA_BITLIBNAME, luaopen_bit32},
  {LUA_MATHLIBNAME, luaopen_math},
//  {LUA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};

void Computer::createExecutionEnvironment()
{
    haltExecution();
    
    L = luaL_newstate();

    /* call open functions from 'loadedlibs' and set results to global table */
    for (const luaL_Reg *lib = loadedlibs; lib->func; lib++)
    {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);  /* remove lib */
    }
    
    lua_register(L, "sleep", luaSleep);
    lua_register(L, "clearscreen", luaClearScreen);
    lua_register(L, "print", luaPrint);
    lua_register(L, "printsetxy", luaSetPrintXY);
    lua_register(L, "sendtoconnection", luaSendToConnection);
    lua_register(L, "readfromconnection", luaReadFromConnection);
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    lua_pushnil(L);
    lua_setglobal(L, "loadfile");
    lua_sethook(L, luaCyclesCallback, LUA_MASKCOUNT, 100);
    if (luaL_loadbuffer(L, BootCode, strlen(BootCode), "BOOT ROM"))
    {
        print("BOOT ROM error:\n");
        print(lua_tolstring(L, 1, nullptr));
        print("\n");
        LOG(ERROR) << "Lua BootCode loading error: " << lua_tolstring(L, 1, nullptr);
        haltExecution();
        return;
    }
    run_state = Running;
    state = Operational;
    sprite.setColor(sf::Color::White);
}

void Computer::resumeExecution(int nr_arg)
{
    current_computer = this;
    int ret = lua_resume(L, L, nr_arg);
    if (ret == LUA_ERRRUN)
    {
        print("BOOT ROM error:\n");
        print(lua_tolstring(L, -1, nullptr));
        print("\n");
        LOG(ERROR) << "Lua Error: " << lua_tolstring(L, -1, nullptr);
        haltExecution();
    }
}

void Computer::clearScreen()
{
    memset(screen, ' ', sizeof(screen));
    print_position = 0;
}

void Computer::print(const char* str)
{
    while(*str)
    {
        if (*str == '\n')
        {
            print_position = (print_position / ScreenWidth) * ScreenWidth + ScreenWidth;
            str++;
        }else{
            screen[print_position] = *str;
            print_position++;
            str++;
        }
        while(print_position >= ScreenWidth * ScreenHeight)
        {
            memmove(screen, &screen[ScreenWidth], ScreenWidth * (ScreenHeight - 1));
            memset(&screen[ScreenWidth * (ScreenHeight - 1)], ' ', ScreenWidth);
            print_position -= ScreenWidth;
        }
    }
}

Computer* Computer::current_computer;

int Computer::luaSleep(lua_State* L)
{
    current_computer->sleep_time = luaL_checknumber(L, 1);
    current_computer->run_state = Sleep;
    return lua_yield(L, 0);
}

int Computer::luaClearScreen(lua_State* L)
{
    current_computer->clearScreen();
    return 0;
}

int Computer::luaPrint(lua_State* L)
{
    int top = lua_gettop(L);
    for(int n=1; n<=top; n++)
    {
        const char* str = lua_tolstring(L, n, nullptr);
        if (n > 1)
            current_computer->print(" ");
        if (str)
            current_computer->print(str);
    }
    return 0;
}

int Computer::luaSetPrintXY(lua_State* L)
{
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    x = std::max(0, std::min(x, ScreenWidth - 1));
    y = std::max(0, std::min(y, ScreenHeight - 1));
    current_computer->print_position = x + y * ScreenWidth;
    return 0;
}

int Computer::luaSendToConnection(lua_State* L)
{
    int io_nr = luaL_checkinteger(L, 1);
    const char* message = luaL_checkstring(L, 2);
    
    if (io_nr < 0 || io_nr >= IOPortCount)
    {
        luaL_argerror(L, 1, "IO port out of range");
    }
    
    current_computer->io_port[io_nr]->sendMessage(message);
    
    current_computer->sleep_time = strlen(message) / 9600.0f;
    current_computer->run_state = Sleep;
    return lua_yield(L, 0);
}

int Computer::luaReadFromConnection(lua_State* L)
{
    current_computer->io_port_selection = 0;
    for(int n=1; n<=lua_gettop(L); n++)
    {
        current_computer->io_port_selection |= 1 << luaL_checkinteger(L, n);
    }
    current_computer->sleep_time = 15.0f;
    current_computer->run_state = WaitingForMessage;
    return lua_yield(L, 0);
}

void Computer::luaCyclesCallback(lua_State *L, lua_Debug *ar)
{
    lua_yield(L, 0);
}
