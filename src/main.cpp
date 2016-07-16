#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "engine.h"

#include "gui/mouseRenderer.h"
#include "cellgrid/cellgrid.h"

RenderLayer* background_layer;
RenderLayer* world_layer;
RenderLayer* hud_layer;
RenderLayer* mouse_layer;
sf::Font* main_font;

sf::Shader* basicShader;

class CellGridTest : public Updatable, public Renderable
{
public:
    CellGrid grid;

    CellGridTest()
    : grid(sf::Vector2i(50, 50))
    {
        for(int x=10; x<=40; x++)
            grid.cells[x + 10 * grid.size.x].blocked = true;
        for(int x=10; x<=40; x++)
            grid.cells[x + 40 * grid.size.x].blocked = true;
        for(int y=10; y<=40; y++)
            grid.cells[12 + y * grid.size.x].blocked = true;
        for(int y=10; y<=40; y++)
            grid.cells[40 + y * grid.size.x].blocked = true;
    }
    
    virtual void update(float delta)
    {
        if (InputHandler::keyboardIsDown(sf::Keyboard::A))
            grid.cells[20 + 20 * grid.size.x].addMaterial(MaterialType::oxygen, 200.0f);
        if (InputHandler::keyboardIsDown(sf::Keyboard::B))
            grid.cells[40 + 20 * grid.size.x].addMaterial(MaterialType::water, 200.0f);
        //if (InputHandler::keyboardIsPressed(sf::Keyboard::Space))
        {
            grid.updateStep();
        }
    }
    
    virtual void render(sf::RenderTarget& window)
    {
        sf::RectangleShape rect(sf::Vector2f(10, 10));
        for(int y=0; y<grid.size.y; y++)
        {
            for(int x=0; x<grid.size.x; x++)
            {
                Cell& c = grid.cells[x + y * grid.size.x];
                
                float r=0, g=0, b=0;
                auto it = c.material_content.find(MaterialType::oxygen);
                if (it != c.material_content.end())
                {
                    float f = std::min(255.0f, it->second);
                    r += f;
                    g += f;
                    b += f;
                }
                it = c.material_content.find(MaterialType::water);
                if (it != c.material_content.end())
                {
                    b += std::min(255.0f, it->second);
                }

                rect.setPosition(x * 10, y * 10);
                rect.setFillColor(sf::Color(std::min(255.0f, r), std::min(255.0f, g), std::min(255.0f, b)));
                window.draw(rect);
            }
        }
    }
};

int main(int argc, char** argv)
{
#ifdef DEBUG
    Logging::setLogLevel(LOGLEVEL_DEBUG);
#else
    Logging::setLogLevel(LOGLEVEL_INFO);
#endif

    new Engine();

    initMaterialData();

    new DirectoryResourceProvider("resources/");
    textureManager.setDefaultSmooth(false);
    textureManager.setDefaultRepeated(false);
    textureManager.setAutoSprite(true);

    //Setup the rendering layers.
    background_layer = new RenderLayer();
    world_layer = new RenderLayer(background_layer);
    hud_layer = new RenderLayer(world_layer);
    mouse_layer = new RenderLayer(hud_layer);

    defaultRenderLayer = hud_layer;

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
    
    engine->registerObject("mouseRenderer", new MouseRenderer());

    new CellGridTest();

    engine->runMainLoop();

    delete engine;

    return 0;
}
