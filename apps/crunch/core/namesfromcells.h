#pragma once
// Names made from the labels round a block of cells: the top row of the block names the columns under it, the left column names the rows beside it, and the bottom row and right column
// do the same for the cells above and to the left. A label is turned into a name (letters, digits, underscores, and dots; "_" in front when it would begin wrongly or be a cell's
// address). No GUI.

#include "sheet/cellref.h"

#include <QString>
#include <QVector>

#include <functional>

namespace jp::crunch {

enum LabelEdge : int { TopEdge = 1, LeftEdge = 2, BottomEdge = 4, RightEdge = 8 };

// The name a label makes, or empty if it makes none (an empty label, or one with no letter or digit in it).
QString nameFromLabel(const QString &label);

struct LabelName {
    QString name;
    jp::office::sheet::CellRange cells;   // what it stands for
};
// The names of the block's edges that are asked for (`edges`: LabelEdge flags), in order: the top row's, then the left column's, the bottom row's, the right column's. `labelAt(row, col)` gives
// the text of a cell. A label that makes no name, and one that makes a name an earlier one made, are left out; `skipped`, if given, counts them. A block with a single row or column has
// nothing under or beside its labels, and makes none.
std::vector<LabelName> namesFromLabels(const jp::office::sheet::CellRange &block, int edges, const std::function<QString(quint32 row, quint32 col)> &labelAt, int *skipped = nullptr);

} // namespace jp::crunch
