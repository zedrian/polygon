#include "game-state-editor.h"
#include "game.h"


using std::make_pair;
using std::to_string;
using sf::Color;
using sf::Event;
using sf::Mouse;
using sf::Vector2f;
using sf::Vector2i;


GameStateEditor::GameStateEditor(Game* game) :
        GameState(game),
        _zoom_level(1.0f),
        _action_state(ActionState::None),
        _selection_start(0, 0),
        _selection_end(0, 0)
{
    auto size = Vector2f(game->window.getSize());
    _gui_view.setSize(size);
    _game_view.setSize(size);

    size *= 0.5f;
    _gui_view.setCenter(size);
    _game_view.setCenter(size);

    _city = City("../data/city", Game::TileSize, (game->tile_atlas));
    _city.shuffleTiles();

    _gui_system.emplace("right click menu", Gui(Vector2f(196, 16), 2, false, game->stylesheets.at("button"),
                                                {
                                                        make_pair("Flatten $" + game->tile_atlas["grass"].get_cost(), "grass"),
                                                        make_pair("Forest $" + game->tile_atlas["forest"].get_cost(), "forest"),
                                                        make_pair("Residential zone $" + game->tile_atlas["residential"].get_cost(), "residential"),
                                                        make_pair("Commercial zone $" + game->tile_atlas["commercial"].get_cost(), "commercial"),
                                                        make_pair("Industrial zone $" + game->tile_atlas["industrial"].get_cost(), "industrial"),
                                                        make_pair("Road $" + game->tile_atlas["road"].get_cost(), "road")
                                                }));
    _gui_system.emplace("selection cost text", Gui(Vector2f(196, 16), 0, false, game->stylesheets.at("text"), {make_pair("", "")}));
    _gui_system.emplace("info bar", Gui(Vector2f(game->window.getSize().x / 5, 16), 2, true, game->stylesheets.at("button"),
                                        {
                                                make_pair("time", "time"),
                                                make_pair("funds", "funds"),
                                                make_pair("population", "population"),
                                                make_pair("employment", "employment"),
                                                make_pair("current tile", "tile")
                                        }));
    _gui_system.at("info bar").setPosition(Vector2f(0, game->window.getSize().y - 16));
    _gui_system.at("info bar").show();

    Vector2f center(_city.map.width, _city.map.height * 0.5f);
    center *= float(_city.map.tile_size);
    _game_view.setCenter(center);

    _current_tile = &game->tile_atlas.at("grass");
}

GameStateEditor::~GameStateEditor()
{ }

void GameStateEditor::draw(const float dt)
{
    game->window.clear(Color::Black);

    game->window.setView(_gui_view);
    game->window.draw(game->background);

    game->window.setView(_game_view);
    _city.map.draw(game->window, dt);

    game->window.setView(_gui_view);
    for (auto& gui : _gui_system)
        game->window.draw(gui.second);
}

void GameStateEditor::update(const float dt)
{
    _city.update(dt);

    _gui_system.at("info bar").setEntryText(0, "Day: " + to_string(_city.day));
    _gui_system.at("info bar").setEntryText(1, "$" + to_string(static_cast<long>(_city.funds)));
    _gui_system.at("info bar").setEntryText(2, to_string(static_cast<long>(_city.population)) + " (" + to_string(static_cast<long>(_city.getHomeless())) + ")");
    _gui_system.at("info bar").setEntryText(3, to_string(static_cast<long>(_city.employable)) + " (" + to_string(static_cast<long>(_city.getUnemployed())) + ")");
    _gui_system.at("info bar").setEntryText(4, tileTypeToString(_current_tile->type));
}

void GameStateEditor::handleInput()
{
    Event event;

    auto gui_position = game->window.mapPixelToCoords(Mouse::getPosition(game->window), _gui_view);
    auto game_position = game->window.mapPixelToCoords(Mouse::getPosition(game->window), _game_view);

    while (game->window.pollEvent(event))
        switch (event.type)
        {
            case Event::Closed:
                game->window.close();
                break;

            case Event::Resized:
                _game_view.setSize(event.size.width, event.size.height);
                _game_view.zoom(_zoom_level);
                _gui_view.setSize(event.size.width, event.size.height);

                _gui_system.at("info bar").setDimensions(Vector2f(event.size.width / _gui_system.at("info bar").entries.size(), 16));
                _gui_system.at("info bar").setPosition(game->window.mapPixelToCoords(Vector2i(0, event.size.height - 16), _gui_view));
                _gui_system.at("info bar").show();

                game->background.setPosition(game->window.mapPixelToCoords(Vector2i(0, 0), _gui_view));
                game->background.setScale(float(event.size.width) / float(game->background.getTexture()->getSize().x),
                                           float(event.size.height) / float(game->background.getTexture()->getSize().y));
                break;

            case Event::MouseMoved:
                if (_action_state == ActionState::Panning)
                {
                    auto position = Vector2f(Mouse::getPosition(game->window) - _panning_anchor);
                    _game_view.move(-1.0f * position * _zoom_level);
                    _panning_anchor = Mouse::getPosition(game->window);
                }
                else if (_action_state == ActionState::Selecting)
                {
                    auto position = game->window.mapPixelToCoords(Mouse::getPosition(game->window), _game_view);
                    _selection_end.x = position.y / _city.map.tile_size + position.x / (2 * _city.map.tile_size) - _city.map.width * 0.5f - 0.5f;
                    _selection_end.y = position.y / _city.map.tile_size - position.x / (2 * _city.map.tile_size) + _city.map.width * 0.5f + 0.5f;

                    _city.map.clearSelected();
                    if (_current_tile->type == TileType::Grass)
                        _city.map.select(_selection_start, _selection_end,
                                    {
                                            _current_tile->type,
                                            TileType::Water
                                    });
                    else
                        _city.map.select(_selection_end, _selection_end,
                                    {
                                            _current_tile->type,
                                            TileType::Forest,
                                            TileType::Water,
                                            TileType::Road,
                                            TileType::Residential,
                                            TileType::Commercial,
                                            TileType::Industrial
                                    });

                    _gui_system.at("selection cost text").setEntryText(0, "$" + to_string(_current_tile->cost * _city.map.selected_tiles_number));
                    if (_city.funds <= _city.map.selected_tiles_number * _current_tile->cost)
                        _gui_system.at("selection cost text").highlight(0);
                    else
                        _gui_system.at("selection cost text").highlight(-1);
                    _gui_system.at("selection cost text").setPosition(gui_position + Vector2f(16, -16));
                    _gui_system.at("selection cost text").show();
                }

                _gui_system.at("right click menu").highlight(_gui_system.at("right click menu").getEntry(gui_position));
                break;

            case Event::MouseButtonPressed:
                if (event.mouseButton.button == Mouse::Middle)
                {
                    _gui_system.at("right click menu").hide();
                    _gui_system.at("selection cost text").hide();

                    if (_action_state != ActionState::Panning)
                    {
                        _action_state = ActionState::Panning;
                        _panning_anchor = Mouse::getPosition(game->window);
                    }
                }
                else if (event.mouseButton.button == Mouse::Left)
                {
                    if (_gui_system.at("right click menu").visible)
                    {
                        auto message = _gui_system.at("right click menu").activate(gui_position);
                        if (message != "flatten" && message != "null")
                            _current_tile = &game->tile_atlas.at(message);

                        _gui_system.at("right click menu").hide();
                    }
                    else if (_action_state != ActionState::Selecting)
                    {
                        _action_state = ActionState::Selecting;
                        _selection_start.x = game_position.y / _city.map.tile_size + game_position.x / (2 * _city.map.tile_size) - _city.map.width * 0.5f - 0.5f;
                        _selection_start.y = game_position.y / _city.map.tile_size - game_position.x / (2 * _city.map.tile_size) + _city.map.width * 0.5f + 0.5f;
                    }
                }
                else if (event.mouseButton.button == Mouse::Right)
                if (_action_state == ActionState::Selecting)
                {
                    _action_state = ActionState::None;
                    _gui_system.at("selection cost text").hide();
                    _city.map.clearSelected();
                }
                else
                {
                    auto position = gui_position;

                    if (position.x > game->window.getSize().x - _gui_system.at("right click menu").size().x)
                        position.x -= _gui_system.at("right click menu").size().x;
                    if (position.y > game->window.getSize().y - _gui_system.at("right click menu").size().y)
                        position.y -= _gui_system.at("right click menu").size().y;

                    _gui_system.at("right click menu").setPosition(position);
                    _gui_system.at("right click menu").show();
                }
                break;

            case Event::MouseButtonReleased:
                if (event.mouseButton.button == Mouse::Middle)
                    _action_state = ActionState::None;
                else if (event.mouseButton.button == Mouse::Left)
                if (_action_state == ActionState::Selecting)
                {
                    if (_current_tile != nullptr)
                    {
                        auto cost = _current_tile->cost * _city.map.selected_tiles_number;
                        if (_city.funds >= cost)
                        {
                            _city.bulldoze(*_current_tile);
                            _city.funds -= _current_tile->cost * _city.map.selected_tiles_number;
                            _city.tileChanged();
                        }
                    }
                    _gui_system.at("selection cost text").hide();
                    _action_state = ActionState::None;
                    _city.map.clearSelected();
                }
                break;

            case Event::MouseWheelMoved:
                if (event.mouseWheel.delta < 0)
                {
                    _game_view.zoom(2.0f);
                    _zoom_level *= 2.0f;
                }
                else
                {
                    _game_view.zoom(0.5f);
                    _zoom_level *= 0.5f;
                }
                break;

            default:
                break;
        }
}

