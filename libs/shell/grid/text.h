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
    // The width the text has in the cell's font at its exact size, in pixels with fractions (what a General number is fitted to its column by; the font drawn for a family that is not
    // installed is its stand-in's, so the answer is the same on every machine). A meter that has no finer measure gives width().
    virtual double fitWidth(const QString &text, const CellLook &look) const { return width(text, look); }
    // The height of a line of text in the cell's font (what a row is made high enough for).
    virtual double lineHeight(const CellLook &look) const { return look.pointSize * 96.0 / 72.0 * 1.36; }
};

enum class Kind : quint8 { Empty, Text, Number, Bool, Error };

// A stretch of a text in a font of its own (the formatting inside one cell's text): from `start` to the next run's start. A run that has no font of its own is in the cell's.
struct TextRun {
    int start = 0;
    bool own = false;          // the run has a font of its own: family, pointSize, and the styles below
    QString family;
    double pointSize = 11;
    bool bold = false, italic = false, strike = false;
    Underline underline = Underline::None;
    quint8 vertAlign = 0;
    QRgb color = 0;            // 0: the cell's text color
};

// What one cell shows.
struct Shown {
    QString text;              // what is drawn (a number too wide for its cell is the "#" that fills it)
    Kind kind = Kind::Empty;
    QRgb color = 0;            // the color the number format asked for; 0 for the text's own
    bool hashes = false;       // a number too wide for the cell: `text` is "#" repeated
    double width = 0;          // of the text, as the meter says
    HAlign align = HAlign::Left;   // where it sits (the look's, or for "general" the one the value has: numbers right, text left, true and false and errors centered)
    std::vector<TextRun> runs; // formatting inside the text: in order, the first at 0 (empty for a text in one font)
    int fillAt = -1;           // a "*x" in the number format: the text is padded with this character to fill the cell (where in `text`), or -1
    QChar fillChar;
};

// The room text has in a cell of this width (the grid's units at 100%): the width less what is left at each side, and the indent.
constexpr double kCellPadding = 3;
double textRoom(double cellWidth, const CellLook &look);
// What a column keeps free of a General number's text, in pixels: 4 for a number in fixed notation, the 5 of padding the specification gives a column's width (ECMA-376 18.3.1.13) less its
// gridline's pixel (2 on each side); 6 for one in scientific notation, the room a cell's text has (kCellPadding on each side). Measured on the practice workbooks (the reference pages of
// tools/crunch_print_score.py; the numbers of 108 workbooks, in the exact widths of the cell's font): every text of fixed notation the reference shows leaves at least 4.03 of the column
// free and every one it cuts leaves at most 3.93; for scientific notation 6.1 and 5.8.
constexpr double kGeneralFitPadding = 4.0;
constexpr double kGeneralSciFitPadding = 6.0;
inline double generalFitPadding(const QString &text) { return text.contains(QLatin1Char('E')) ? kGeneralSciFitPadding : kGeneralFitPadding; }

// What the value shows in a cell of `cellWidth`, in the look. A "general" number has the most digits whose text, measured in the cell's font (TextMeter::fitWidth), leaves kGeneralFitPadding of the column
// free, the column being `fitWidth` (the column of the screen: a page is drawn in the printer's pixels, and the text that fits is the screen's; 0: `cellWidth`); when none does it is "#".
// With `symbols`, a number shows the person's decimal mark and digit grouping (the number format codes stay as the file has them).
Shown showValue(const jp::calc::Value &value, const CellLook &look, jp::calc::DateSystem system, double cellWidth, int maxDigitWidth, const TextMeter &meter, const jp::calc::NumberSymbols *symbols = nullptr, double fitWidth = 0);

// A formula as the formula bar and Show Formulas give it: the prefixes a file writes before the functions that are newer than its format and before names of its own are left
// out (they are kept in the model, for saving).
QString displayFormula(const QString &formula);

// A cell shows at most this many characters of a text.
constexpr int kMaxShownChars = 1024;
// And no more than this many are looked at to find them (a text of millions of carriage returns, which show as nothing, is not read to its end: what comes after the first
// sixteen thousand characters of it is not shown).
constexpr int kMaxScannedChars = 16 * kMaxShownChars;
// The text of a text cell as the grid shows it: line breaks are spaces (or kept, when the cell wraps), carriage returns go, and no more than kMaxShownChars remain. `scanned`, if
// given, is told how many characters of the text were looked at.
QString shownLine(const QString &text, bool wrap, qint64 *scanned = nullptr);

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
