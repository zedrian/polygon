#include <fstream>

#include "map.h"


using std::ifstream;
using std::ios;
using std::logic_error;
using std::ofstream;
using std::swap;
using sf::Vector2f;
using sf::Color;


Map::Map() :
        tile_size(8),
        width(0),
        height(0),
        selected_tiles_number(0)
{
    num_regions[0] = 1;
}

Map::Map(const string& file_name,
         unsigned int width,
         unsigned int height,
         const map<string, Tile>& tile_atlas) :
        tile_size(8),
        selected_tiles_number(0)
{
    load(file_name, width, height, tile_atlas);
}


void Map::load(const string& file_name,
               unsigned int width,
               unsigned int height,
               const map<string, Tile>& tile_atlas)
{
    ifstream file;
    file.open(file_name, ios::in | ios::binary);

    this->width = width;
    this->height = height;

    for (int position = 0; position < width * height; ++position)
    {
        resources.push_back(255);
        selected_tiles.push_back(0);

        TileType tile_type;
        file.read(reinterpret_cast<char*>(&tile_type), sizeof(TileType));
        switch (tile_type)
        {
            case TileType::Void:
            case TileType::Grass:
                tiles.push_back(tile_atlas.at("grass"));
                break;

            case TileType::Forest:
                tiles.push_back(tile_atlas.at("forest"));
                break;

            case TileType::Water:
                tiles.push_back(tile_atlas.at("water"));
                break;

            case TileType::Residential:
                tiles.push_back(tile_atlas.at("residential"));
                break;

            case TileType::Commercial:
                tiles.push_back(tile_atlas.at("commercial"));
                break;

            case TileType::Industrial:
                tiles.push_back(tile_atlas.at("industrial"));
                break;

            case TileType::Road:
                tiles.push_back(tile_atlas.at("road"));
                break;

            default:
                throw logic_error("Unknown tile type.");
        }
        auto& tile = tiles.back();
        file.read(reinterpret_cast<char*>(&tile.tile_variant), sizeof(int));
        file.read(reinterpret_cast<char*>(&tile.regions), sizeof(int) * 1);
        file.read(reinterpret_cast<char*>(&tile.population), sizeof(double));
        file.read(reinterpret_cast<char*>(&tile.stored_goods), sizeof(float));
    }

    file.close();
}

void Map::save(const string& file_name)
{
    ofstream file;
    file.open(file_name, ios::out | ios::binary);

    for (auto& tile : tiles)
    {
        file.write(reinterpret_cast<char*>(&tile.type), sizeof(TileType));
        file.write(reinterpret_cast<char*>(&tile.tile_variant), sizeof(int));
        file.write(reinterpret_cast<char*>(&tile.regions), sizeof(int) * 1);
        file.write(reinterpret_cast<char*>(&tile.population), sizeof(double));
        file.write(reinterpret_cast<char*>(&tile.stored_goods), sizeof(float));
    }

    file.close();
}

void Map::draw(RenderWindow& window,
               float dt)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            Vector2f position;
            position.x = (x - y) * tile_size + width * tile_size;
            position.y = (x + y) * tile_size * 0.5f;
            tiles[y * width + x].sprite.setPosition(position);

            if (selected_tiles[y*width+x])
                tiles[y*width+x].sprite.setColor(Color(0x7d, 0x7d, 0x7d));
            else
                tiles[y*width+x].sprite.setColor(Color(0xff, 0xff, 0xff));

            tiles[y * width + x].draw(window, dt);
        }
}

void Map::findConnectedRegions(vector<TileType> white_list,
                               int type)
{
    int regions = 1;

    for (auto& tile : tiles)
        tile.regions[type] = 0;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
        {
            auto found = false;
            for (auto& type : white_list)
                if (tiles[y * width + x].type == type)
                {
                    found = true;
                    break;
                }
            if (tiles[y * width + x].regions[type] == 0 && found)
                depthFirstSearch(white_list, Vector2i(x, y), regions++, type);
        }

    num_regions[type] = regions;
}

void Map::updateDirection(TileType tile_type)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            auto position = y * width + x;
            if (tiles[position].type != tile_type)
                continue;

            bool adjacent_tiles[3][3] =
                    {
                            {false, false, false},
                            {false, false, false},
                            {false, false, false}
                    };

            if (x > 0 && y > 0)
                adjacent_tiles[0][0] = tiles[(y - 1) * width + (x - 1)].type == tile_type;
            if (y > 0)
                adjacent_tiles[0][1] = tiles[(y - 1) * width + x].type == tile_type;
            if (x < width - 1 && y > 0)
                adjacent_tiles[0][2] = tiles[(y - 1) * width + (x + 1)].type == tile_type;
            if (x > 0)
                adjacent_tiles[1][0] = tiles[y * width + (x - 1)].type == tile_type;
            if (x < width - 1)
                adjacent_tiles[1][2] = tiles[y * width + (x + 1)].type == tile_type;
            if (x > 0 && y < height - 1)
                adjacent_tiles[2][0] = tiles[(y + 1) * width + (x - 1)].type == tile_type;
            if (y < height - 1)
                adjacent_tiles[2][1] = tiles[(y + 1) * width + x].type == tile_type;
            if (x < width - 1 && height - 1)
                adjacent_tiles[2][2] = tiles[(y + 1) * width + (x + 1)].type == tile_type;

            if (adjacent_tiles[1][0] && adjacent_tiles[1][2] && adjacent_tiles[0][1] && adjacent_tiles[2][1])
                tiles[position].tile_variant = 2;
            else if (adjacent_tiles[1][0] && adjacent_tiles[1][2] && adjacent_tiles[0][1])
                tiles[position].tile_variant = 7;
            else if (adjacent_tiles[1][0] && adjacent_tiles[1][2] && adjacent_tiles[2][1])
                tiles[position].tile_variant = 8;
            else if (adjacent_tiles[0][1] && adjacent_tiles[2][1] && adjacent_tiles[1][0])
                tiles[position].tile_variant = 9;
            else if (adjacent_tiles[0][1] && adjacent_tiles[2][1] && adjacent_tiles[1][2])
                tiles[position].tile_variant = 10;
            else if (adjacent_tiles[1][0] && adjacent_tiles[1][2])
                tiles[position].tile_variant = 0;
            else if (adjacent_tiles[0][1] && adjacent_tiles[2][1])
                tiles[position].tile_variant = 1;
            else if (adjacent_tiles[2][1] && adjacent_tiles[1][0])
                tiles[position].tile_variant = 3;
            else if (adjacent_tiles[0][1] && adjacent_tiles[1][2])
                tiles[position].tile_variant = 4;
            else if (adjacent_tiles[1][0] && adjacent_tiles[0][1])
                tiles[position].tile_variant = 5;
            else if (adjacent_tiles[2][1] && adjacent_tiles[1][2])
                tiles[position].tile_variant = 6;
            else if (adjacent_tiles[1][0])
                tiles[position].tile_variant = 0;
            else if (adjacent_tiles[1][2])
                tiles[position].tile_variant = 0;
            else if (adjacent_tiles[0][1])
                tiles[position].tile_variant = 1;
            else if (adjacent_tiles[2][1])
                tiles[position].tile_variant = 1;
        }
    };
}

void Map::depthFirstSearch(vector<TileType>& white_list,
                           Vector2i position,
                           int label,
                           int type)
{
    if (position.x < 0 || position.x >= width)
        return;
    if (position.y < 0 || position.y >= height)
        return;
    if (tiles[position.y * width + position.x].regions[type] != 0)
        return;

    auto found = false;
    for (auto& type : white_list)
        if (tiles[position.y * width + position.x].type == type)
        {
            found = true;
            break;
        }
    if (!found)
        return;

    tiles[position.y * width + position.x].regions[type] = label;

    depthFirstSearch(white_list, position + Vector2i(-1, 0), label, type);
    depthFirstSearch(white_list, position + Vector2i(0, 1), label, type);
    depthFirstSearch(white_list, position + Vector2i(1, 0), label, type);
    depthFirstSearch(white_list, position + Vector2i(0, -1), label, type);
}

void Map::select(Vector2i start,
                 Vector2i end,
                 const vector<TileType>& black_list)
{
    if (end.y < start.y)
        swap(start.y, end.y);
    if (end.x < start.x)
        swap(start.x, end.x);

    if (end.x >= width)
        end.x = width - 1;
    else if (end.x < 0)
        end.x = 0;
    if (end.y >= height)
        end.y = height-1;
    else if (end.y < 0)
        end.y = 0;
    if (start.x >= width)
        start.x = width - 1;
    else if (start.x < 0)
        start.x = 0;
    if (start.y >= height)
        start.y = height - 1;
    else if (start.y < 0)
        start.y = 0;

    for (auto y = start.y; y <= end.y; ++y)
        for (auto x = start.x; x <= end.x; ++x)
        {
            selected_tiles[y*width + x] = 1;
            ++selected_tiles_number;

            for (auto& type : black_list)
                if (tiles[y*width+x].type == type)
                {
                    selected_tiles[y*width + x] = 2;
                    --selected_tiles_number;
                    break;
                }
        }
}

void Map::clearSelected()
{
    for (auto& tile : selected_tiles)
        tile = 0;
    selected_tiles_number = 0;
}



