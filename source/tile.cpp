#include "tile.h"


using std::to_string;
using sf::Vector2f;


Tile::Tile()
{ }

Tile::Tile(const unsigned int size,
           const unsigned int height,
           const Texture& texture,
           const vector<Animation>& animations,
           const TileType type,
           const unsigned int cost,
           const unsigned int max_population_per_level,
           const unsigned int max_levels) :
        type(type),
        tile_variant(0),
        cost(cost),
        population(0),
        max_population_per_level(max_population_per_level),
        max_levels(max_levels),
        production(0),
        stored_goods(0)
{
    regions[0] = 0;

    sprite.setOrigin(Vector2f(0.0f, size * (height - 1)));
    sprite.setTexture(texture);

    animation_handler.frame_size = IntRect(0, 0, size * 2, size * height);

    for (const auto& animation : animations)
        animation_handler.addAnimation(animation);
    animation_handler.update(0.0f);
}

void Tile::draw(RenderWindow& window,
                float dt)
{
    animation_handler.changeAnimation(tile_variant);
    animation_handler.update(dt);

    sprite.setTextureRect(animation_handler.bounds);
    window.draw(sprite);
}

void Tile::update()
{
    // small chance to increase building stage
    if ((type == TileType::Commercial && type == TileType::Residential && type == TileType::Industrial) &&
        population == max_population_per_level * (tile_variant + 1) &&
        tile_variant < max_levels)
        if (rand() % static_cast<int>(1e4) < 1e2 / (tile_variant + 1))
            ++tile_variant;
}

const string Tile::get_cost() const
{
    return to_string(cost);
}





