#pragma once


#include <map>

#include "tile.h"


using std::map;
using sf::Vector2i;


class Map
{
public:
    Map();
    Map(const string& file_name,
        unsigned int width,
        unsigned int height,
        const map<string, Tile>& tile_atlas);

    void load(const string& file_name,
              unsigned int width,
              unsigned int height,
              const map<string, Tile>& tile_atlas);
    void save(const string& file_name);

    void draw(RenderWindow& window,
              float dt);

    void findConnectedRegions(vector<TileType> white_list,
                              int type);

    void updateDirection(TileType tile_type);

    void select(Vector2i start,
                Vector2i end,
                const vector<TileType>& black_list);
    void clearSelected();


public:
    unsigned int width;
    unsigned int height;

    vector<Tile> tiles;
    vector<int> resources;

    unsigned int tile_size;

    unsigned int num_regions[1];

    vector<unsigned char> selected_tiles;
    unsigned int selected_tiles_number;


private:
    void depthFirstSearch(vector<TileType>& white_list,
                          Vector2i position,
                          int label,
                          int type = 0);
};