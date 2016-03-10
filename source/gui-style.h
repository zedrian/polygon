#pragma once


#include <SFML/Graphics.hpp>


using sf::Color;
using sf::Font;


class GuiStyle
{
public:
    GuiStyle();
    GuiStyle(Font* font,
             float border_size,
             const Color& body_color,
             const Color& border_color,
             const Color& text_color,
             const Color& body_highlight_color,
             const Color& border_highlight_color,
             const Color& text_highlight_color);


public:
    Font* font;
    float border_size;

    Color body_color;
    Color border_color;
    Color text_color;

    Color body_highlight_color;
    Color border_highlight_color;
    Color text_highlight_color;
};