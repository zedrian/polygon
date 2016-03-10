#include "gui.h"


Gui::Gui(const Vector2f& dimensions,
         int padding,
         bool horizontal,
         const GuiStyle& style,
         const vector<pair<string, string>>& entries) :
        _dimensions(dimensions),
        _padding(padding),
        _horizontal(horizontal),
        _style(style),
        visible(false)
{
    RectangleShape shape;
    shape.setSize(dimensions);
    shape.setFillColor(style.body_color);
    shape.setOutlineThickness(-style.border_size);
    shape.setOutlineColor(style.border_color);

    for (auto& entry : entries)
    {
        Text text;
        text.setString(entry.first);
        text.setFont(*style.font);
        text.setColor(style.text_color);
        text.setCharacterSize(dimensions.y - style.border_size - padding);

        this->entries.push_back(GuiEntry(shape, entry.second, text));
    }
}

Vector2f Gui::size() const
{
    return sf::Vector2f(_dimensions.x, _dimensions.y * entries.size());
}

int Gui::getEntry(const Vector2f& mouse_position)
{
    if (entries.empty())
        return -1;
    if (!visible)
        return -1;

    for (int i = 0; i < entries.size(); ++i)
    {
        auto point = mouse_position;
        point += entries[i].shape.getOrigin();
        point -= entries[i].shape.getPosition();

        if (point.x < 0 || point.x > entries[i].shape.getScale().x * _dimensions.x)
            continue;
        if (point.y < 0 || point.y > entries[i].shape.getScale().y * _dimensions.y)
            continue;

        return i;
    }

    return -1;
}

void Gui::setEntryText(int entry,
                       string text)
{
    if (entry >= entries.size())
        return;
    if (entry < 0)
        return;

    entries[entry].text.setString(text);
}

void Gui::setDimensions(const Vector2f& dimensions)
{
    _dimensions = dimensions;

    for (auto& entry : entries)
    {
        entry.shape.setSize(dimensions);
        entry.text.setCharacterSize(dimensions.y - _style.border_size - _padding);
    }
}

void Gui::draw(RenderTarget& target,
               RenderStates states) const
{
    if (!visible)
        return;

    for (auto& entry : entries)
    {
        target.draw(entry.shape, states);
        target.draw(entry.text, states);
    }
}

void Gui::show()
{
    Vector2f offset(0.0f, 0.0f);

    visible = true;

    for (auto& entry : entries)
    {
        auto origin = getOrigin();
        origin -= offset;
        entry.shape.setOrigin(origin);
        entry.text.setOrigin(origin);

        entry.shape.setPosition(getPosition());
        entry.text.setPosition(getPosition());

        if (_horizontal)
            offset.x += _dimensions.x;
        else
            offset.y += _dimensions.y;
    }
}

void Gui::hide()
{
    visible = false;
}

void Gui::highlight(const int entry)
{
    for (int i = 0; i < entries.size(); ++i)
        if (i == entry)
        {
            entries[i].shape.setFillColor(_style.body_highlight_color);
            entries[i].shape.setOutlineColor(_style.border_highlight_color);
            entries[i].text.setColor(_style.text_highlight_color);
        }
        else
        {
            entries[i].shape.setFillColor(_style.body_color);
            entries[i].shape.setOutlineColor(_style.border_color);
            entries[i].text.setColor(_style.text_color);
        }
}

string Gui::activate(const int entry)
{
    if (entry == -1)
        return "null";
    return entries[entry].message;
}

string Gui::activate(const Vector2f& mouse_position)
{
    auto entry = getEntry(mouse_position);
    return activate(entry);
}

