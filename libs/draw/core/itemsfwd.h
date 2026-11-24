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
// The extensions of a picture's blip kept as another program wrote them (see PictureItem::keptBlip): how many, how long each one's text, how many parts each points at, and the bytes of all of one picture's parts.
constexpr int kMaxKeptBlipExtensions = 4;
constexpr int kMaxKeptBlipParts = 4;
constexpr int kMaxKeptBlipXml = 16 << 10;
constexpr qint64 kMaxKeptBlipBytes = 8 << 20;
// ... and the bytes of all the pictures of one file together.
constexpr qint64 kMaxKeptBlipFileBytes = 64 << 20;

} // namespace jp
