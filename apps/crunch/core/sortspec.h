#pragma once
// What a Sort asks for: the levels (the column, or the row for a sort from left to right, to sort by; by its values, its cell color, or its font color; ascending or descending, or in the
// order of a list), whether the first row is a header, and whether upper and lower case differ. See EditSession::sortRange.

#include "sheet/styles.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::crunch {

constexpr int kMaxSortLevels = 8;

struct SortLevel {
    enum class By : quint8 { Value, CellColor, FontColor };
    int key = 0;                    // the column (the row, for a sort from left to right) to sort by, counted from the first of the range
    By by = By::Value;
    bool descending = false;
    // A sort by color: the color that goes first (onTop) or last. A color that is not set means cells with no color.
    jp::office::sheet::ColorSpec color;
    bool onTop = true;
    // A sort by values in the order of a list (the names of the days, say): values that are in the list go in its order, before those that are not.
    QStringList list;
};

struct SortSpec {
    QVector<SortLevel> levels;
    bool hasHeader = false;         // the first row (the first column, for a sort from left to right) is not sorted
    bool caseSensitive = false;
    bool leftToRight = false;       // sort the columns, by the values in a row
};

// The lists that are offered: the days and months, in full and short, in English and in the person's language.
QVector<QStringList> builtInSortLists();

} // namespace jp::crunch
