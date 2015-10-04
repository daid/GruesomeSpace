#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "engine.h"

#include "gui/mouseRenderer.h"
#include "menus/constructionMenu.h"

#include "spaceship.h"
#include "spaceshipObjects/wall.h"
#include "spaceshipObjects/electrical/solarPanel.h"
#include "spaceshipObjects/electrical/light.h"
#include "spaceshipObjects/electrical/computer.h"
#include "spaceshipObjects/electrical/database.h"

sf::Vector2f view_location;
RenderLayer* background_layer;
RenderLayer* floor_layer;
RenderLayer* object_layer;
RenderLayer* wiring_layer;
RenderLayer* hud_layer;
RenderLayer* mouse_layer;
sf::Font* main_font;

int main(int argc, char** argv)
{
#ifdef DEBUG
    Logging::setLogLevel(LOGLEVEL_DEBUG);
#endif

    new Engine();

    new DirectoryResourceProvider("resources/");
    textureManager.setDefaultSmooth(false);
    textureManager.setDefaultRepeated(false);
    textureManager.setAutoSprite(true);

    //Setup the rendering layers.
    background_layer = new RenderLayer();
    floor_layer = new RenderLayer(background_layer);
    object_layer = new RenderLayer(floor_layer);
    wiring_layer = new RenderLayer(object_layer);
    hud_layer = new RenderLayer(wiring_layer);
    mouse_layer = new RenderLayer(hud_layer);

    defaultRenderLayer = object_layer;

    int width = 1200;
    int height = 900;
    int fsaa = 0;
    bool fullscreen = false;

    P<ResourceStream> stream = getResourceStream("sansation.ttf");
    main_font = new sf::Font();
    main_font->loadFromStream(**stream);

    P<WindowManager> window_manager = new WindowManager(width, height, fullscreen, mouse_layer, fsaa);
    window_manager->setAllowVirtualResize(true);
    engine->registerObject("windowManager", window_manager);
    
    new MouseRenderer();
    
    new ConstructionMenu();
    
    new Spaceship();
    
    for(int y=0; y<20; y++)
    {
        for(int x=0; x<20; x++)
        {
            spaceship->setFloor(sf::Vector2i(x, y));
        }
        spaceship->addObject(new Wall(), sf::Vector2i(0, y));
        spaceship->addObject(new Wall(), sf::Vector2i(19, y));
    }
    for(int x=0; x<20; x++)
    {
        spaceship->addObject(new Wall(), sf::Vector2i(x, 0));
        spaceship->addObject(new Wall(), sf::Vector2i(x, 19));
    }

    spaceship->addObject(new SolarPanel(), sf::Vector2i(1, 1));
    spaceship->addObject(new Light(), sf::Vector2i(6, 6));
    spaceship->addObject(new Computer(), sf::Vector2i(1, 6));
    spaceship->addObject(new Database(), sf::Vector2i(3, 6));
    P<Wire> w = new Wire(sf::Color::White);
    w->connect(spaceship->getObject(sf::Vector2i(1, 1))->getWiringConnection(0), spaceship->getObject(sf::Vector2i(6, 6))->getWiringConnection(0));
    w->addWirePosition(sf::Vector2f(1.5, 1.4));
    w->addWirePosition(sf::Vector2f(6.7, 1.4));
    w->addWirePosition(sf::Vector2f(6.7, 6.5));
    w = new Wire(sf::Color::White);
    w->connect(spaceship->getObject(sf::Vector2i(1, 1))->getWiringConnection(0), spaceship->getObject(sf::Vector2i(1, 6))->getWiringConnection(0));
    w->addWirePosition(sf::Vector2f(1.3, 1.4));
    w->addWirePosition(sf::Vector2f(1.3, 6.4));

    w = new Wire(sf::Color::White);
    w->connect(spaceship->getObject(sf::Vector2i(4, 3))->getWiringConnection(0), spaceship->getObject(sf::Vector2i(4, 6))->getWiringConnection(0));
    w->addWirePosition(sf::Vector2f(4.3, 3.4));
    w->addWirePosition(sf::Vector2f(4.3, 6.4));
    w = new Wire(sf::Color::White);
    w->connect(spaceship->getObject(sf::Vector2i(2, 6))->getWiringConnection(1), spaceship->getObject(sf::Vector2i(3, 6))->getWiringConnection(1));
    w->addWirePosition(sf::Vector2f(2.3, 6.4));
    w->addWirePosition(sf::Vector2f(3.3, 6.4));

    view_location = sf::Vector2f(-100, -100);

    engine->runMainLoop();

    delete engine;    

    return 0;
}
