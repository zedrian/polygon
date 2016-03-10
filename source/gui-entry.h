#pragma once


#include <string>
#include <SFML/Graphics.hpp>


using std::string;
using sf::RectangleShape;
using sf::Text;


class GuiEntry
{
public:
    GuiEntry();
    GuiEntry(const RectangleShape& shape,
             const string& message,
             const Text& text);

public:
    RectangleShape shape;
    string message;
    Text text;
};