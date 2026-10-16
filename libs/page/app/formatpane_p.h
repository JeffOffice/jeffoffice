#pragma once
// What the pages of the Format pane share (formatpane_*.cpp): which objects take what, and the pages themselves.

#include "app/formatpane.h"
#include "core/items.h"

#include <functional>

namespace jp {

class PanePage;

namespace format {

// Whether an object has a fill of its own, and a line of its own. A line takes no fill; a table's are its cells'.
inline bool takesFill(const Item &it)
{
    switch (it.type()) {
    case ItemType::Text:
    case ItemType::Picture:
    case ItemType::Shape:
    case ItemType::TextArt: return true;
    default: return false;
    }
}

inline bool takesLine(const Item &it)
{
    switch (it.type()) {
    case ItemType::Text:
    case ItemType::Picture:
    case ItemType::TextArt:
    case ItemType::Shape:
    case ItemType::Line: return true;
    default: return false;
    }
}

template <class F> bool anyTakes(const QVector<Item *> &items, F &&takes)
{
    for (const Item *it : items)
        if (takes(*it)) return true;
    return false;
}

// Builds the controls of each page.
void buildPages(FormatPane *pane, PanePage *fill, PanePage *effects, PanePage *size, PanePage *picture);

} // namespace format
} // namespace jp
