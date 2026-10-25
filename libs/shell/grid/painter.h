#pragma once
// Draws the grid: the cells in view and nothing else, from the layout (which rows and columns are where) and the model (what each cell looks like and says). A sheet is a million
// rows of sixteen thousand columns and a file may make every number in it as large as it likes, so each thing the drawing does is bounded, and each is counted as it is done
// (PaintStats): the shown rows and columns visited, the cells looked at, the fills, borders, and runs of text drawn, the neighbors looked at for text that spills, the merged
// ranges, and whether a bound was reached and cells were left out. A screen's work is that of the cells in it, whatever the sheet.
//
// The order is: the backdrop, the gridlines (the last pixel of each cell's right and bottom, under any fill), merged ranges' fills, each row's fills, its borders (where two
// cells meet the heavier line wins), and its text (which goes over the fills of the empty cells it spills into), the merged ranges' text, the selection, then the headers.
// Every part of the grid (the frozen corner, top, and left, and the body) is drawn with its own clip, and a clip smaller than the window (the strip a scroll leaves to be
// drawn) costs the rows and columns in it.

#include "grid/gridsheet.h"
#include "grid/selection.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QHash>
#include <QPainter>

#include <memory>
#include <vector>

namespace jp::grid {

struct PaintOptions {
    bool gridlines = true;
    bool headers = true;
    const Selection *selection = nullptr;
    bool fillHandle = false;                               // a small square at the corner of the selection to drag to fill
    bool monochrome = false;                               // black text and lines, and no fills (a page printed in black and white)
    bool strokedDashes = false;                            // a dashed border is one stroke of a dashed pen, not a mark for each dash (for paper: a page of them is tens of thousands of marks)
    const GridRange *fillPreview = nullptr;                // where a fill that is being dragged would go: a dashed border
    const GridRange *copyRange = nullptr;                  // the cells that were copied or cut: a dashed border that moves
    int copyPhase = 0;                                     // where the dashes of it are (its animation step)
    QRect dropDown;                                        // the button of a list on the cell with the cursor (empty for none)
    int resizeGuide = -1;                                  // where the border of a row or column being dragged is now (a place in the window), or -1
    bool resizeGuideColumn = true;                         // a column's border (a vertical line), else a row's
    QColor gridColor = QColor(0xD4, 0xD4, 0xD4);
    QColor backdrop = QColor(0xE6, 0xE6, 0xE6);            // beyond the last row and column
    QColor paper = QColor(Qt::white);                      // the cells with no fill
    QColor headerBackground = QColor(0xF3, 0xF3, 0xF3);
    QColor headerText = QColor(0x44, 0x44, 0x44);
    QColor headerLine = QColor(0xC8, 0xC8, 0xC8);
    QColor headerSelected = QColor(0xD6, 0xE9, 0xDC);      // a header whose row or column is selected
    QColor accent = QColor(0x13, 0x77, 0x3A);
    QFont headerFont;
};

struct PaintStats {
    qint64 panes = 0;            // parts of the grid drawn
    qint64 rowsVisited = 0;      // shown rows and columns whose cells were drawn (in all parts)
    qint64 colsVisited = 0;
    qint64 cellsLooked = 0;      // cells of the sheet looked at, those beside the screen that text may spill from among them
    qint64 spillProbes = 0;      // cells looked at to see whether text may go into them
    qint64 fills = 0;
    qint64 borderSegments = 0;
    qint64 dashSteps = 0;        // the dashes and gaps of dashed borders stepped over
    qint64 gridLines = 0;
    qint64 textRuns = 0;         // texts drawn
    qint64 textChars = 0;        // their characters
    qint64 hashed = 0;           // numbers too wide for their cells, drawn as "#"
    qint64 mergesDrawn = 0;
    qint64 mergesLeftOut = 0;    // merged ranges on the screen that were not drawn for want of room (the bound is far above what a sheet has)
    qint64 mergeMarks = 0;       // cells marked as part of a merged range
    qint64 headersDrawn = 0;
    qint64 condCells = 0;        // cells the rules of conditional formatting were asked about
    qint64 bars = 0, icons = 0;  // data bars and icons drawn
    qint64 textsMade = 0;        // texts of cells made (not found in the cache) during the drawing
    qint64 looksMade = 0;
    bool truncated = false;      // a bound was reached: some cells in view were left out
};

class GridPainter {
public:
    // Shown rows (and columns) drawn in a part at most, cells looked at in a part, runs of text, neighbors looked at for spilling text, merged ranges drawn in a part, and cells
    // marked as under a merged range. A font smaller than this many pixels is not drawn (text that small is not readable, and costs the same).
    static constexpr int kMaxRows = 2048, kMaxCols = 2048;
    static constexpr qint64 kMaxCells = 60000, kMaxTextRuns = 12000, kMaxSpillProbes = 20000, kMaxMergeMarks = 200000;
    static constexpr int kMaxMerges = 100000;   // far above what a screen of a real sheet has (its cells are at most 60,000, and a merged range is two of them or more); the ones left out are counted
    static constexpr int kMinFontPixels = 4;
    static constexpr qint64 kMaxCondCells = 60000;   // cells of a part the rules of conditional formatting are asked about

    explicit GridPainter(GridSheet *sheet);
    ~GridPainter();

    // Draws what the clip (in the window's coordinates) needs. The painter's clip is changed, and is left as it was.
    void paint(QPainter *painter, const QRect &clip, const PaintOptions &options);
    const PaintStats &stats() const { return m_stats; }
    // Forgets the fonts made (the zoom, the fonts, or the looks changed).
    void resetFonts();
    // The meter this uses for the model (the fonts of the cells, at 100%), already given to the sheet.
    const TextMeter &meter() const;

private:
    class QtMeter;
    class GlyphFace;
    struct Span {
        qint64 index;
        int start, size;
        bool ring;   // a row or column beside those drawn, looked at only for the borders it shares
    };
    struct PaneWork;

    void paintPane(QPainter *p, Pane which, const QRect &clip, const PaintOptions &o);
    void paintHeaders(QPainter *p, const QRect &clip, const PaintOptions &o);
    void paintSelection(QPainter *p, Pane which, const QRect &area, const PaintOptions &o);
    void paintDashedRange(QPainter *p, Pane which, const QRect &area, const GridRange &r, const QColor &color, int phase, bool outside);
    void drawFill(QPainter *p, const QRect &rect, const CellLook &look, const PaintOptions &o);
    void drawEdge(QPainter *p, const LineLook &line, int x, int y, int length, bool vertical, const QRect &clip);
    void drawText(QPainter *p, const QRect &cell, const QRect &box, const QRect &area, const CellLook &look, const Shown &shown, const PaintOptions &o);
    void drawHeaderText(QPainter *p, const QRect &cell, const QString &text);
    void drawBar(QPainter *p, const QRect &cell, const CondBar &bar, bool gridlines);
    void drawIcon(QPainter *p, const QRect &cell, const QString &set, int index, int count, int size);
    const QFont &fontFor(const CellLook &look);
    QBrush patternBrush(PatternKind kind, QRgb fg, QRgb bg);

    GridSheet *m_sheet;
    PaintStats m_stats;
    QtMeter *m_meter;
    std::vector<QFont> m_fonts;                       // by font id, at the zoom of the last paint
    std::vector<bool> m_fontMade;
    double m_fontZoom = 0;
    QHash<quint64, QBrush> m_patterns;
    std::vector<std::unique_ptr<GlyphFace>> m_faces;          // by font id, at the zoom of the last paint
    int m_iconInset = 0;              // the room the icon of the cell being drawn takes at its left (device pixels)
    bool m_drawingMirrored = false;   // drawText is drawing a text of a sheet that reads right to left, on the screen's places
    int m_lastFontId = -1;      // what the painter was last given, so a run in the same font and color sets neither again
    double m_lastFontZoom = 0;
    QRgb m_lastInk = 0;
    bool m_mono = false;        // the paint in hand is in black and white (PaintOptions::monochrome)
    bool m_strokedDashes = false;   // and dashes are strokes (PaintOptions::strokedDashes)
};

} // namespace jp::grid
