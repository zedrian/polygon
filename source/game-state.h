#pragma once


class Game;

class GameState
{
public:
    GameState(Game* game) : game(game) {}
    virtual ~GameState() {}

    virtual void draw(const float dt) = 0;
    virtual void update(const float dt) = 0;
    virtual void handleInput() = 0;


public:
    Game* game;
};