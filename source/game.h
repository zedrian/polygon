#pragma once


#include <stack>
#include <SFML/Graphics/RenderWindow.hpp>

#include "texture-manager.h"
#include "tile.h"
#include "gui-style.h"


using std::map;
using std::stack;
using sf::RenderWindow;
using sf::Sprite;


class GameState;


class Game
{
public:
    const static int TileSize = 8;


public:
    Game();
    ~Game();

    void pushState(GameState* state);
    void popState();
    void changeState(GameState* state);
    GameState* peekState();

    void main(int argc,
              char** argv);


public:
    stack<GameState*> states;
    RenderWindow window;
    TextureManager texture_manager;
    Sprite background;
    map<string, Tile> tile_atlas;
    map<string, GuiStyle> stylesheets;
    map<string, Font> fonts;


private:
    void loadTextures();
    void loadTiles();
    void loadStylesheets();
    void loadFonts();
};