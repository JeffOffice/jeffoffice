#pragma once
// The names of the frame types, without the frames (core/items.h has those): for a header that only passes a frame by pointer or reference.

#include <QString>
#include <memory>
#include <vector>

namespace jp {

enum class ItemType { Text, Picture, Shape, Line, Table, TextArt, Group, Equation };
QString itemTypeName(ItemType t);

class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemList = std::vector<ItemPtr>;

QString newId(const char *prefix = "i");

class TextItem;
class PictureItem;
class ShapeItem;
class LineItem;
class TableItem;
class TextArtItem;
class EquationItem;
class GroupItem;

// The most bytes of a chart part a picture keeps (a chart is kept as a chart while its part is no larger).
constexpr qint64 kMaxChartPartBytes = 2 << 20;

} // namespace jp
