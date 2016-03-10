#include "gui-style.h"


GuiStyle::GuiStyle()
{ }

GuiStyle::GuiStyle(Font* font,
                   float border_size,
                   const Color& body_color,
                   const Color& border_color,
                   const Color& text_color,
                   const Color& body_highlight_color,
                   const Color& border_highlight_color,
                   const Color& text_highlight_color) :
        body_color(body_color),
        body_highlight_color(body_highlight_color),
        border_color(border_color),
        border_highlight_color(border_highlight_color),
        text_color(text_color),
        text_highlight_color(text_highlight_color),
        font(font),
        border_size(border_size)
{ }