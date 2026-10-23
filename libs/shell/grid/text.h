#pragma once
// What a cell says: its value turned to the text a person sees through the cell's number format, and where that text goes in the cell (the alignment a value has when the
// format says "general", and how far text may spill into the empty cells beside it). No drawing and no fonts: the width of a text is asked of a TextMeter, which the grid view
// gives (and a test gives one of its own), always in the grid's own units at 100%.

#include "calc/dates.h"
#include "calc/value.h"
#include "grid/look.h"

#include <QString>

#include <functional>

namespace jp::grid {

// Measures text for the cell's font; the width in the grid's units at 100% (pixels, at 96 to the inch), of the text in one line.
class TextMeter {
public:
    virtual ~TextMeter();
    virtual double width(const QString &text, const CellLook &look) const = 0;
};

enum class Kind : quint8 { Empty, Text, Number, Bool, Error };

// What one cell shows.
struct Shown {
    QString text;              // what is drawn (a number too wide for its cell is the "#" that fills it)
    Kind kind = Kind::Empty;
    QRgb color = 0;            // the color the number format asked for; 0 for the text's own
    bool hashes = false;       // a number too wide for the cell: `text` is "#" repeated
    double width = 0;          // of the text, as the meter says
    HAlign align = HAlign::Left;   // where it sits (the look's, or for "general" the one the value has: numbers right, text left, true and false and errors centered)
    int fillAt = -1;           // a "*x" in the number format: the text is padded with this character to fill the cell (where in `text`), or -1
    QChar fillChar;
};

// The room text has in a cell of this width (the grid's units at 100%): the width less what is left at each side, and the indent.
constexpr double kCellPadding = 3;
double textRoom(double cellWidth, const CellLook &look);

// What the value shows in a cell of `cellWidth`, in the look. `generalChars` is how many characters a "general" number may take (from the width: see charsFor()).
Shown showValue(const jp::calc::Value &value, const CellLook &look, jp::calc::DateSystem system, double cellWidth, int maxDigitWidth, const TextMeter &meter);
// The characters of the widest digit that fit in a column of this width.
int charsFor(double cellWidth, int maxDigitWidth);

// How far the text of a cell goes into the cells beside it: how many columns on each side (0 for none).
struct Spill {
    int left = 0, right = 0;
    double extra = 0;          // the width of what it took, left and right together
};
// `neighbor(step)` says the width of the column `step` away (-1 left, +1 right, ...) when its cell is empty and the text may go into it, and a negative number when it may
// not (a cell with something in it, or the edge of the sheet); a column that is hidden has width 0 and is stepped over. Text of a left-aligned cell goes right, a right-aligned
// goes left, and a centered one both ways, each as far as it needs; none that is wrapped, shrunk, turned, a number (or true, false, an error), or too wide, and none of more
// than `kMaxSpill` columns.
constexpr int kMaxSpill = 256;
Spill spillOf(const Shown &shown, const CellLook &look, double cellWidth, const std::function<double(int step)> &neighbor);

} // namespace jp::grid
