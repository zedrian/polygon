#pragma once


#include <SFML/Graphics.hpp>

#include "animation-handler.h"
#include "tile-type.h"


using sf::RenderWindow;
using sf::Sprite;
using sf::Texture;


class Tile
{
public:
    Tile();
    Tile(const unsigned int size,
         const unsigned int height,
         const Texture& texture,
         const vector<Animation>& animations,
         const TileType type,
         const unsigned int cost,
         const unsigned int max_population_per_level,
         const unsigned int max_levels);

    void draw(RenderWindow& window,
              float dt);
    void update();

    const string get_cost() const;


public:
    AnimationHandler animation_handler;
    Sprite sprite;
    TileType type;

    int tile_variant;
    unsigned int regions[1];

    unsigned int cost;
    double population;
    unsigned int max_population_per_level;
    unsigned int max_levels;
    float production;
    float stored_goods;
};