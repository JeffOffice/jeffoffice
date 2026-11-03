#pragma once
// Sparklines: a tiny chart drawn inside one cell from a row or column of values (x14:sparklineGroups in a sheet's extension list). A group holds the look (kind, colors, which
// points are marked, how the axis is chosen) and the sparklines it is for: each has the cell it is drawn in and the cells it reads. The text of a source ("Sheet1!A2:F2") is kept as
// the file has it; the drawing finds its values (see grid/sparklinepaint.h).

#include "sheet/cellref.h"
#include "sheet/styles.h"

#include <QByteArray>
#include <QSet>
#include <QString>
#include <QVector>
#include <QXmlStreamReader>

#include <map>
#include <memory>
#include <mutex>

namespace jp::office::sheet {

struct Sparkline {
    quint32 row = 0, col = 0;       // the cell it is drawn in
    QString source;                 // what it reads, as a range with the sheet's name when the file has one
    bool operator==(const Sparkline &o) const { return row == o.row && col == o.col && source == o.source; }
};

struct SparklineGroup {
    enum class Type : quint8 { Line, Column, WinLoss };
    enum class Empty : quint8 { Gap, Zero, Span };           // what a cell with nothing in it is drawn as
    enum class Axis : quint8 { Individual, Group, Custom };  // how the lowest or highest value of the vertical axis is chosen
    Type type = Type::Line;
    Empty empty = Empty::Zero;
    double lineWeight = 0.75;       // points
    bool markers = false, high = false, low = false, first = false, last = false, negative = false;
    bool showAxis = false;          // a line across at 0 (displayXAxis)
    bool hidden = false;            // cells in hidden rows and columns are drawn too (displayHidden)
    bool dateAxis = false;          // the points are spaced by the dates in `dateRange`
    bool rightToLeft = false;
    Axis minAxis = Axis::Individual, maxAxis = Axis::Individual;
    double manualMin = 0, manualMax = 0;
    ColorSpec series, negativeColor, axisColor, markerColor, firstColor, lastColor, highColor, lowColor;
    QString dateRange;
    QByteArray otherAttributes;     // attributes the program does not know, as written (` xr2:uid="{...}"`), kept
    QVector<Sparkline> sparklines;
    bool operator==(const SparklineGroup &o) const;
    bool operator!=(const SparklineGroup &o) const { return !(*this == o); }
};

// The most the program reads of a sheet's sparklines (groups, and sparklines in all of them).
constexpr int kMaxSparklineGroups = 10000;
constexpr int kMaxSparklines = 100000;

// The most a workbook's sparklines hold in all its sheets: how many, and the characters of the texts of what they read. A sheet's part read again counts once.
constexpr qint64 kMaxWorkbookSparklines = 200000;
constexpr qint64 kMaxWorkbookSparklineChars = 8000000;
constexpr int kMaxSparklineTextChars = 4096;   // one source, or a date range

class SparklineBudget {
public:
    // What is left for `sheet` (the other sheets' use taken off the total).
    void left(int sheet, qint64 *chars, qint64 *count) const
    {
        std::lock_guard<std::mutex> g(m_lock);
        qint64 c = 0, n = 0;
        for (const auto &e : m_used)
            if (e.first != sheet) {
                c += e.second.first;
                n += e.second.second;
            }
        *chars = std::max<qint64>(0, kMaxWorkbookSparklineChars - c);
        *count = std::max<qint64>(0, kMaxWorkbookSparklines - n);
    }
    void set(int sheet, qint64 chars, qint64 count)
    {
        std::lock_guard<std::mutex> g(m_lock);
        m_used[sheet] = {chars, count};
    }
private:
    mutable std::mutex m_lock;
    std::map<int, std::pair<qint64, qint64>> m_used;
};

// The colors a group that gives none is drawn with.
ColorSpec defaultSparklineColor(const char *which);   // "series", "negative", "axis", "markers", "first", "last", "high", "low"

// Reads x14:sparklineGroups as the sheet's stream gives its elements: `start` for each element (`rel` is how deep it is inside the sparklineGroups element, which is 0), `text` for its
// characters, `end` for its close.
class SparklineParser {
public:
    void start(const QXmlStreamReader &r, int rel);
    void text(QStringView t);
    void end(int rel);
    QVector<SparklineGroup> groups;
    int count = 0;                  // the sparklines read, in all groups
    qint64 charsLeft = kMaxWorkbookSparklineChars, countLeft = kMaxWorkbookSparklines;   // what the workbook still allows (see SparklineBudget)
    qint64 charsUsed = 0;
    // The file holds something that reading and writing would lose (a cell range for a sparkline, an element or a color part this does not know, a sparkline past the limits): the sheet's
    // sparklines are shown, but left as the file has them.
    bool lossy = false;
private:
    enum class Into : quint8 { None, DateRange, Source, Cell } into = Into::None;
    SparklineGroup m_group;
    Sparkline m_one;
    QString m_text;
    bool m_inGroup = false, m_inOne = false, m_over = false;
    QSet<QByteArray> m_declared;
};

// The extension element for the sheet's extension list that holds these groups (`<ext uri="...">`), empty for none.
QByteArray sparklineExtensionXml(const QVector<SparklineGroup> &groups);
// The uri of that extension.
const char *sparklineExtensionUri();

} // namespace jp::office::sheet
