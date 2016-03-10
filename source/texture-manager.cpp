#include "texture-manager.h"


void TextureManager::loadTexture(const string& name,
                                 const string& file_name)
{
    Texture texture;
    texture.loadFromFile(file_name);

    _textures[name] = texture;
}

Texture& TextureManager::get(const string& name)
{
    return _textures.at(name);
}