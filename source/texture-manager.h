#pragma once


#include <map>
#include <string>
#include <SFML/Graphics.hpp>


using std::map;
using std::string;
using sf::Texture;


class TextureManager
{
public:
    void loadTexture(const string& name,
                     const string& file_name);
    Texture& get(const string& name);


private:
    map<string, Texture> _textures;
};