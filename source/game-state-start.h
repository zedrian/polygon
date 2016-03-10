#pragma once


#include <SFML/Graphics.hpp>

#include "game-state.h"
#include "gui.h"


using sf::View;


class GameStateStart : public GameState
{
public:
    GameStateStart(Game* game);
    virtual ~GameStateStart();

    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();


private:
    void loadGame();


private:
    View _view;
    map<string, Gui> _gui_system;
};