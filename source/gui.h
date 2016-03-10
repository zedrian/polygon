#pragma once


#include <utility>
#include <vector>
#include <SFML/Graphics.hpp>

#include "gui-entry.h"
#include "gui-style.h"


using std::pair;
using std::vector;
using sf::Drawable;
using sf::RenderStates;
using sf::RenderTarget;
using sf::Transformable;
using sf::Vector2f;


class Gui : public Drawable,
            public Transformable
{
public:
    Gui(const Vector2f& dimensions,
        int padding,
        bool horizontal,
        const GuiStyle& style,
        const vector<pair<string, string>>& entries);
    virtual ~Gui() {}

    Vector2f size() const;

    int getEntry(const Vector2f& mouse_position);
    void setEntryText(int entry,
                      string text);

    void setDimensions(const Vector2f& dimensions);

    virtual void draw(RenderTarget& target,
                      RenderStates states) const;

    void show();
    void hide();

    void highlight(const int entry);

    string activate(const int entry);
    string activate(const Vector2f& mouse_position);


public:
    vector<GuiEntry> entries;
    bool visible;


private:
    bool _horizontal;
    GuiStyle _style;
    Vector2f _dimensions;
    int _padding;
};