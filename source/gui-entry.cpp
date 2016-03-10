#include "gui-entry.h"


GuiEntry::GuiEntry()
{ }

GuiEntry::GuiEntry(const RectangleShape& shape,
                   const string& message,
                   const Text& text) :
        shape(shape),
        message(message),
        text(text)
{ }