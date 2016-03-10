#pragma once


#include <string>


using std::string;


enum class TileType : unsigned char
{
    Void,
    Grass,
    Forest,
    Water,
    Residential,
    Commercial,
    Industrial,
    Road
};


const string tileTypeToString(TileType type);