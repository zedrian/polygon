#include <SFML/System.hpp>

#include "game-state.h"
#include "game.h"


using sf::Clock;
using sf::Time;
using sf::Color;


Game::Game()
{
    loadTextures();
    loadTiles();
    loadFonts();
    loadStylesheets();

    window.create(sf::VideoMode(800, 600), "Polygon SFML test");
    window.setFramerateLimit(60);

    background.setTexture(texture_manager.get("background"));
}

Game::~Game()
{
    while (!states.empty())
        popState();
}

void Game::pushState(GameState* state)
{
    states.push(state);
}

void Game::popState()
{
    delete states.top();
    states.pop();
}

void Game::changeState(GameState* state)
{
    if (!states.empty())
        popState();

    pushState(state);
}

GameState* Game::peekState()
{
    if (states.empty())
        return nullptr;

    return states.top();
}


void Game::main(int argc,
                char** argv)
{
    Clock clock;

    while (window.isOpen())
    {
        Time elapsed = clock.restart();
        auto dt = elapsed.asSeconds();

        if (peekState() == nullptr)
            continue;

        peekState()->handleInput();
        peekState()->update(dt);

        window.clear(Color::Black);
        peekState()->draw(dt);
        window.display();
    }
}


void Game::loadTextures()
{
    texture_manager.loadTexture("grass", "../data/grass.png");
    texture_manager.loadTexture("forest", "../data/forest.png");
    texture_manager.loadTexture("water", "../data/water.png");
    texture_manager.loadTexture("residential", "../data/residential.png");
    texture_manager.loadTexture("commercial", "../data/commercial.png");
    texture_manager.loadTexture("industrial", "../data/industrial.png");
    texture_manager.loadTexture("road", "../data/road.png");

    texture_manager.loadTexture("background", "../data/background.png");
}

void Game::loadTiles()
{
    Animation static_animation(0, 0, 1.0f);

    tile_atlas["grass"] = Tile(TileSize, 1, texture_manager.get("grass"),
                               {
                                       static_animation
                               }, TileType::Grass, 50, 0, 1);
    tile_atlas["forest"] = Tile(TileSize, 1, texture_manager.get("forest"),
                                {
                                        static_animation
                                }, TileType::Forest, 100, 0, 1);
    tile_atlas["water"] = Tile(TileSize, 1, texture_manager.get("water"),
                               {
                                       Animation(0, 3, 0.5f),
                                       Animation(0, 3, 0.5f),
                                       Animation(0, 3, 0.5f)
                               }, TileType::Water, 0, 0, 1);
    tile_atlas["residential"] = Tile(TileSize, 2, texture_manager.get("residential"),
                                     {
                                             static_animation,
                                             static_animation,
                                             static_animation,
                                             static_animation,
                                             static_animation,
                                             static_animation
                                     }, TileType::Residential, 300, 50, 6);
    tile_atlas["commercial"] = Tile(TileSize, 2, texture_manager.get("commercial"),
                                    {
                                            static_animation,
                                            static_animation,
                                            static_animation,
                                            static_animation
                                    }, TileType::Commercial,
                                    300, 50, 4);
    tile_atlas["industrial"] = Tile(TileSize, 2, texture_manager.get("industrial"),
                                    {
                                            static_animation,
                                            static_animation,
                                            static_animation,
                                            static_animation
                                    }, TileType::Industrial,
                                    300, 50, 4);
    tile_atlas["road"] = Tile(TileSize, 1, texture_manager.get("road"),
                              {
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation,
                                      static_animation
                              }, TileType::Road, 100, 0, 1);
}

void Game::loadStylesheets()
{
    stylesheets["button"] = GuiStyle(&fonts.at("main"), 1,
                                     Color(0xc6, 0xc6, 0xc6), Color(0x94, 0x94, 0x94), Color(0x00, 0x00, 0x00),
                                     Color(0x61, 0x61, 0x61), Color(0x94, 0x94, 0x94), Color(0x00, 0x00, 0x00));
    stylesheets["text"] = GuiStyle(&fonts.at("main"), 0,
                                   Color(0x00, 0x00, 0x00, 0x00), Color(0x00, 0x00, 0x00), Color(0xff, 0xff, 0xff),
                                   Color(0x00, 0x00, 0x00, 0x00), Color(0x00, 0x00, 0x00), Color(0xff, 0x00, 0x00));
}

void Game::loadFonts()
{
    Font font;
    font.loadFromFile("../data/font.ttf");
    fonts["main"] = font;
}