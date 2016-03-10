#include "tile-type.h"


const string tileTypeToString(TileType type)
{
    switch(type)
    {
        case TileType::Void:
            return "Void";

        case TileType::Grass:
            return "Grass";

        case TileType::Forest:
            return "Forest";

        case TileType::Water:
            return "Water";

        case TileType::Residential:
            return "Residential";

        case TileType::Commercial:
            return "Commercial";

        case TileType::Industrial:
            return "Industrial";

        case TileType::Road:
            return "Road";

        default:
            return "Unknown";
    }
}