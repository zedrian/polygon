#include "game.h"
#include "game-state-editor.h"
#include "game-state-start.h"


using std::make_pair;
using sf::Color;
using sf::Event;
using sf::Keyboard;
using sf::Mouse;
using sf::Vector2i;
using sf::Vector2f;


GameStateStart::GameStateStart(Game* game) :
        GameState(game)
{
    auto size = Vector2f(game->window.getSize());
    _view.setSize(size);

    size *= 0.5f;
    _view.setCenter(size);

    _gui_system.emplace("menu", Gui(Vector2f(192, 32), 4, false, game->stylesheets.at("button"), {make_pair("LOAD GAME", "load game")}));
    _gui_system.at("menu").setPosition(size);
    _gui_system.at("menu").setOrigin(96, 16);
    _gui_system.at("menu").show();
}

GameStateStart::~GameStateStart()
{ }


void GameStateStart::draw(const float dt)
{
    game->window.setView(_view);

    game->window.clear(Color::Black);
    game->window.draw(game->background);

    for (auto& gui : _gui_system)
        game->window.draw(gui.second);
}

void GameStateStart::update(const float dt)
{ }

void GameStateStart::handleInput()
{
    Event event;

    auto mouse_position = game->window.mapPixelToCoords(Mouse::getPosition(game->window), _view);
    auto position = Vector2f(event.size.width, event.size.height);

    while (game->window.pollEvent(event))
        switch (event.type)
        {
            case Event::Closed:
                game->window.close();
                break;

            case Event::Resized:
                _view.setSize(event.size.width, event.size.height);
                game->background.setPosition(game->window.mapPixelToCoords(Vector2i(0, 0)));

                position *= 0.5f;
                position = game->window.mapPixelToCoords(Vector2i(position), _view);
                _gui_system.at("menu").setPosition(position);
                game->background.setScale(float(event.size.width) / float(game->background.getTexture()->getSize().x),
                                            float(event.size.height) / float(game->background.getTexture()->getSize().y));
                break;

            case Event::MouseMoved:
                _gui_system.at("menu").highlight(_gui_system.at("menu").getEntry(mouse_position));
                break;

            case Event::MouseButtonPressed:
                if (event.mouseButton.button == Mouse::Left)
                {
                    auto message = _gui_system.at("menu").activate(mouse_position);
                    if (message == "load game")
                        loadGame();
                }
                break;

            case Event::KeyPressed:
                switch (event.key.code)
                {
                    case Keyboard::Escape:
                        game->window.close();
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
}

void GameStateStart::loadGame()
{
    game->pushState(new GameStateEditor(game));
}