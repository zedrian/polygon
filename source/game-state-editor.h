#pragma once


#include <SFML/Graphics.hpp>

#include "action-state.h"
#include "city.h"
#include "game-state.h"
#include "gui.h"


using sf::View;


class GameStateEditor : public GameState
{
public:
    GameStateEditor(Game* game);
    virtual ~GameStateEditor();

    virtual void draw(const float dt);
    virtual void update(const float dt);
    virtual void handleInput();


private:
    ActionState _action_state;
    View _game_view;
    View _gui_view;

    map<string, Gui> _gui_system;

    City _city;

    Vector2i _panning_anchor;
    float _zoom_level;

    Vector2i _selection_start;
    Vector2i _selection_end;

    Tile* _current_tile;
};