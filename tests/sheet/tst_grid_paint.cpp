// Tests of drawing the grid (libs/shell/grid/painter.h): what lands on the pixels (fills, gridlines, borders in their styles and the heavier line where two meet, patterns, text and
// where it is cut or spills, merged ranges, frozen panes, the selection, the headers), and the work a paint does, counted as it is done, for sheets of a million rows and for
// hostile ones (rows of one pixel, hidden rows, a hundred thousand merged ranges, texts of 32,000 characters, a million cell formats). Cells are made here; the checks are on
// colors and on counts, never on the shape of a letter, so they hold on every system's fonts.
#include <QRawFont>
#include <QFontInfo>
#include <QFontMetricsF>
#include "core/fonts.h"
#include "sheethelpers.h"

#include "grid/painter.h"

#include <memory>

using namespace jp::grid;

namespace {

constexpr QRgb kWhite = 0xFFFFFFFF;
constexpr QRgb kBlack = 0xFF000000;

// A sheet in the making, and what it is drawn on. Cells begin at the corner (no headers) unless a test asks for them: column c begins at 64c, row r at 20r, and the last
// pixel of each is where its gridline is.
struct Fixture {
    Sheet sheet;
    Styles styles;
    SharedStrings strings;
    GridSheet grid;
    std::unique_ptr<GridPainter> painter;
    PaintOptions options;
    QSize size{640, 400};

    Fixture()
    {
        FontSpec f;
        f.name = QStringLiteral("Calibri");   // (11 points of it: a widest digit of 7, so columns of 64 and rows of 20)
        f.size = 11;
        styles.fonts = {f};
        styles.fills = {FillSpec()};
        styles.borders = {BorderSpec()};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
        sheet.view.showHeadings = false;
        options.headers = false;
    }
    // A cell format; its place is what a cell's style says.
    quint32 format(const CellFormat &f)
    {
        styles.formats.append(f);
        styles.cellXfs.append(f.numFmtId);
        return quint32(styles.formats.size() - 1);
    }
    quint32 fill(PatternKind kind, quint32 fg, quint32 bg = 0xFFFFFF)
    {
        FillSpec s;
        s.pattern = kind;
        s.fg.kind = ColorSpec::Kind::Rgb;
        s.fg.rgb = fg;
        s.bg.kind = ColorSpec::Kind::Rgb;
        s.bg.rgb = bg;
        styles.fills.append(s);
        CellFormat f;
        f.fill = quint32(styles.fills.size() - 1);
        return format(f);
    }
    quint32 border(const BorderSpec &b, HAlign h = HAlign::General)
    {
        styles.borders.append(b);
        CellFormat f;
        f.border = quint32(styles.borders.size() - 1);
        f.horizontal = h;
        return format(f);
    }
    static BorderSide side(BorderStyle s, quint32 rgb)
    {
        BorderSide b;
        b.style = s;
        b.color.kind = ColorSpec::Kind::Rgb;
        b.color.rgb = rgb;
        return b;
    }
    void text(quint32 row, quint32 col, const QString &t, quint32 style = 0)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::InlineString;
        c.number = sheet.texts.size();
        c.style = style;
        sheet.texts.append(t);
        sheet.cells.add(c);
    }
    void number(quint32 row, quint32 col, double v, quint32 style = 0)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::Number;
        c.number = v;
        c.style = style;
        sheet.cells.add(c);
    }
    void blank(quint32 row, quint32 col, quint32 style)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.style = style;
        sheet.cells.add(c);
    }
    void build()
    {
        sheet.cells.finish();
        GridData d;
        d.styles = &styles;
        d.strings = &strings;
        d.sheet = &sheet;
        grid.setData(d);
        grid.layout().setViewport(size);
        painter = std::make_unique<GridPainter>(&grid);
    }
    QImage render(const QRect &clip = QRect())
    {
        QImage img(size, QImage::Format_RGB32);
        img.fill(QColor(0xFF, 0x00, 0xFF));   // (anything the paint leaves is magenta)
        QPainter p(&img);
        painter->paint(&p, clip.isNull() ? QRect(QPoint(0, 0), size) : clip, options);
        return img;
    }
};

QRgb px(const QImage &img, int x, int y) { return img.pixel(x, y); }

// Pixels in a block that are not white (text ink), and pixels of exactly this color.
int inkIn(const QImage &img, const QRect &r)
{
    int n = 0;
    for (int y = r.top(); y <= r.bottom(); ++y)
        for (int x = r.left(); x <= r.right(); ++x) {
            const QRgb c = img.pixel(x, y);
            if (qRed(c) < 160 && qGreen(c) < 160 && qBlue(c) < 160) ++n;
        }
    return n;
}
int pixelsOf(const QImage &img, const QRect &r, QRgb color)
{
    int n = 0;
    for (int y = r.top(); y <= r.bottom(); ++y)
        for (int x = r.left(); x <= r.right(); ++x)
            if (img.pixel(x, y) == color) ++n;
    return n;
}

// The rectangle of the pixels in a block that are dark (ink): invalid when there are none.
QRect inkBox(const QImage &img, const QRect &r, int limit = 160)
{
    int x0 = 1 << 30, y0 = 1 << 30, x1 = -1, y1 = -1;
    for (int y = r.top(); y <= r.bottom(); ++y)
        for (int x = r.left(); x <= r.right(); ++x) {
            const QRgb c = img.pixel(x, y);
            if (qRed(c) < limit && qGreen(c) < limit && qBlue(c) < limit) {
                x0 = std::min(x0, x);
                x1 = std::max(x1, x);
                y0 = std::min(y0, y);
                y1 = std::max(y1, y);
            }
        }
    return x1 < 0 ? QRect() : QRect(QPoint(x0, y0), QPoint(x1, y1));
}

CellRange mergeOf(quint32 r1, quint32 c1, quint32 r2, quint32 c2)
{
    CellRange r;
    r.row1 = r1;
    r.col1 = c1;
    r.row2 = r2;
    r.col2 = c2;
    r.valid = true;
    return r;
}

} // namespace

// What the fixture's font is on this computer, for a failure that depends on it: the family the system gave, its size, and the width of a text by the font's
// metrics and by the glyphs the painter draws with (the two should agree).
static QString fontReport(const QString &text)
{
    QFont f(QStringLiteral("Calibri"));
    f.setPixelSize(15);   // (11 points at 96 a inch, as the painter makes it)
    f.setKerning(false);
    f.setHintingPreference(QFont::PreferNoHinting);
    const QRawFont raw = QRawFont::fromFont(f);
    qreal glyphs = 0;
    for (const QPointF &a : raw.advancesForGlyphIndexes(raw.glyphIndexesForString(text))) glyphs += a.x();
    return QStringLiteral("font %1 (asked Calibri), pixel size %2, raw pixel size %3; \"%4\" is %5 px by metrics, %6 px by glyphs")
        .arg(QFontInfo(f).family()).arg(QFontInfo(f).pixelSize()).arg(raw.pixelSize()).arg(text).arg(QFontMetricsF(f).horizontalAdvance(text)).arg(glyphs);
}

class GridPaintTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The program's own fonts and their stand-ins, as JeffCrunch loads them at its start: without them a computer with no Calibri (and, under the offscreen platform on
    // Windows, with no fonts at all) draws every letter as wide as the font is tall.
    void initTestCase() { jp::initCore(); }

    // The gridlines are the last pixel of each cell's right and bottom, and a fill covers the cell whole, so a filled cell has none; with gridlines off there are none.
    void gridlinesAreTheLastPixelAndAFillCoversThem()
    {
        Fixture f;
        const quint32 green = f.fill(PatternKind::Solid, 0x00FF00);
        f.blank(1, 1, green);
        f.build();
        QImage img = f.render();
        const QRgb grid = f.options.gridColor.rgb();
        QCOMPARE(px(img, 10, 10), kWhite);
        QCOMPARE(px(img, 63, 5), grid);
        QCOMPARE(px(img, 10, 19), grid);
        QCOMPARE(px(img, 64 + 10, 20 + 10), QColor(0x00FF00).rgb());
        QCOMPARE(px(img, 127, 25), QColor(0x00FF00).rgb());   // the filled cell's last column
        QCOMPARE(px(img, 70, 39), QColor(0x00FF00).rgb());    // and its last row
        QCOMPARE(px(img, 191, 25), grid);                      // the next cell's gridline
        f.options.gridlines = false;
        img = f.render();
        QCOMPARE(px(img, 63, 5), kWhite);
        QCOMPARE(px(img, 10, 19), kWhite);
        // beyond the sheet the backdrop shows: a sheet of 3 rows and 2 columns
        Fixture g;
        g.number(0, 0, 1);
        g.sheet.extentRows = 1;
        g.build();
        g.grid.layout().rows.setCount(3);
        g.grid.layout().cols.setCount(2);
        const QImage small = g.render();
        QCOMPARE(px(small, 10, 10), kWhite);
        QCOMPARE(px(small, 200, 10), g.options.backdrop.rgb());
        QCOMPARE(px(small, 10, 100), g.options.backdrop.rgb());
    }

    // A border is drawn on the edge between two cells, as thick as its style says, and where two cells' borders meet the heavier line is the one drawn.
    void bordersAreDrawnInTheirStylesAndTheHeavierLineWins()
    {
        Fixture f;
        BorderSpec thin;
        thin.top = Fixture::side(BorderStyle::Thin, 0xFF0000);
        BorderSpec medium;
        medium.top = Fixture::side(BorderStyle::Medium, 0x00AA00);
        BorderSpec thick;
        thick.top = Fixture::side(BorderStyle::Thick, 0x0000FF);
        BorderSpec twice;
        twice.top = Fixture::side(BorderStyle::Double, 0x800080);
        BorderSpec dotted;
        dotted.top = Fixture::side(BorderStyle::Dotted, 0x000000);
        BorderSpec dashed;
        dashed.top = Fixture::side(BorderStyle::Dashed, 0x000000);
        f.blank(2, 0, f.border(thin));
        f.blank(2, 1, f.border(medium));
        f.blank(2, 2, f.border(thick));
        f.blank(2, 3, f.border(twice));
        f.blank(2, 4, f.border(dotted));
        f.blank(2, 5, f.border(dashed));
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        // the edge above row 2 is at 40: its pixels are those of row 1's last lines
        QCOMPARE(px(img, 10, 39), QColor(0xFF0000).rgb());
        QCOMPARE(px(img, 10, 38), kWhite);
        QCOMPARE(px(img, 10, 40), kWhite);
        QCOMPARE(px(img, 74, 39), QColor(0x00AA00).rgb());
        QCOMPARE(px(img, 74, 40), QColor(0x00AA00).rgb());
        QCOMPARE(px(img, 74, 38), kWhite);
        QCOMPARE(px(img, 138, 38), QColor(0x0000FF).rgb());
        QCOMPARE(px(img, 138, 39), QColor(0x0000FF).rgb());
        QCOMPARE(px(img, 138, 40), QColor(0x0000FF).rgb());
        QCOMPARE(px(img, 138, 37), kWhite);
        // a double line is two with a gap
        QCOMPARE(px(img, 202, 38), QColor(0x800080).rgb());
        QCOMPARE(px(img, 202, 39), kWhite);
        QCOMPARE(px(img, 202, 40), QColor(0x800080).rgb());
        // dotted: every other pixel along the edge; dashed: more on than off
        const int dots = pixelsOf(img, QRect(256, 39, 62, 1), kBlack);
        QVERIFY2(dots >= 25 && dots <= 37, qPrintable(QString::number(dots)));
        const int dashes = pixelsOf(img, QRect(320, 39, 62, 1), kBlack);
        QVERIFY2(dashes >= 40 && dashes <= 55, qPrintable(QString::number(dashes)));
        // where a cell's bottom border meets the top border of the cell below, the heavier is drawn (a thin red line above, a thick blue below)
        Fixture g;
        BorderSpec bottom;
        bottom.bottom = Fixture::side(BorderStyle::Thin, 0xFF0000);
        BorderSpec top;
        top.top = Fixture::side(BorderStyle::Thick, 0x0000FF);
        g.blank(0, 0, g.border(bottom));
        g.blank(1, 0, g.border(top));
        g.options.gridlines = false;
        g.build();
        const QImage m = g.render();
        QCOMPARE(px(m, 10, 19), QColor(0x0000FF).rgb());
        QCOMPARE(px(m, 10, 18), QColor(0x0000FF).rgb());
        // and one that is alone is drawn: the bottom border of a cell with nothing below it that has a border
        Fixture h;
        h.blank(0, 0, h.border(bottom));
        h.options.gridlines = false;
        h.build();
        QCOMPARE(px(h.render(), 10, 19), QColor(0xFF0000).rgb());
        // the left and right of a cell: a border on the first column is on its right-hand side's pixel, drawn
        Fixture v;
        BorderSpec right;
        right.right = Fixture::side(BorderStyle::Medium, 0x008000);
        v.blank(0, 0, v.border(right));
        v.options.gridlines = false;
        v.build();
        const QImage vi = v.render();
        QCOMPARE(px(vi, 63, 10), QColor(0x008000).rgb());
        QCOMPARE(px(vi, 64, 10), QColor(0x008000).rgb());
        QCOMPARE(px(vi, 62, 10), kWhite);
    }

    // A pattern fill is its foreground color over its background, in the share of the cell its kind says; a solid fill is all of it.
    void patternFillsDrawTheirForegroundOverTheirBackground()
    {
        Fixture f;
        f.blank(0, 0, f.fill(PatternKind::MediumGray, 0xFF0000, 0x0000FF));
        f.blank(0, 1, f.fill(PatternKind::Gray125, 0xFF0000, 0x0000FF));
        f.blank(0, 2, f.fill(PatternKind::DarkHorizontal, 0xFF0000, 0x0000FF));
        f.blank(0, 3, f.fill(PatternKind::DarkGrid, 0xFF0000, 0x0000FF));
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        auto share = [&](int col) {
            const QRect r(col * 64, 0, 64, 20);
            const int fg = pixelsOf(img, r, QColor(0xFF0000).rgb()), bg = pixelsOf(img, r, QColor(0x0000FF).rgb());
            return fg + bg == 64 * 20 ? double(fg) / (64 * 20) : -1.0;   // (-1: some pixel is neither of the two colors)
        };
        QVERIFY2(std::abs(share(0) - 0.5) < 0.06, qPrintable(QString::number(share(0))));
        QVERIFY2(std::abs(share(1) - 0.125) < 0.05, qPrintable(QString::number(share(1))));
        QVERIFY2(std::abs(share(2) - 0.5) < 0.06, qPrintable(QString::number(share(2))));
        QVERIFY2(share(3) > 0.6 && share(3) < 0.9, qPrintable(QString::number(share(3))));
        // the pattern does not move with where the cell is: the same in a cell further on
        Fixture g;
        g.blank(0, 0, g.fill(PatternKind::LightGrid, 0x000000));
        g.blank(5, 7, 1);
        g.options.gridlines = false;
        g.build();
        const QImage a = g.render();
        QVERIFY(pixelsOf(a, QRect(0, 0, 64, 20), kBlack) > 100);
    }

    // Text is drawn in its cell, cut at the edge when the next cell has something in it, and spills into empty cells on the side it is aligned to.
    void textIsCutByItsNeighborAndSpillsIntoEmptyCells()
    {
        const QString wide(40, QLatin1Char('W'));
        Fixture f;
        f.text(0, 0, QStringLiteral("Hello"));
        f.text(1, 0, wide);
        f.text(1, 1, QStringLiteral(" "));   // (a text of a space: not empty, and shows nothing)
        f.text(2, 0, wide);                  // (next cell empty)
        f.text(3, 4, wide, f.border(BorderSpec(), HAlign::Right));
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        QVERIFY(inkIn(img, QRect(0, 0, 64, 20)) > 5);
        QVERIFY2(inkIn(img, QRect(64, 0, 64, 20)) == 0, qPrintable(fontReport(QStringLiteral("Hello"))));
        // cut: ink in the first cell, none in the second
        QVERIFY(inkIn(img, QRect(0, 20, 64, 20)) > 50);
        QCOMPARE(inkIn(img, QRect(64, 20, 64, 20)), 0);
        // spilled: ink in the cell beside
        QVERIFY(inkIn(img, QRect(0, 40, 64, 20)) > 50);
        QVERIFY(inkIn(img, QRect(64, 40, 64, 20)) > 50);
        // right-aligned text goes the other way, over the empty cells on its left, and not into the one on its right
        QVERIFY(inkIn(img, QRect(4 * 64, 60, 64, 20)) > 50);
        QVERIFY(inkIn(img, QRect(3 * 64, 60, 64, 20)) > 50);
        QCOMPARE(inkIn(img, QRect(5 * 64, 60, 64, 20)), 0);
        // the gridline between a cell and the empty cell its text runs into is not drawn (the cells are one to the eye); the line at the end of the text, and between cells whose text is
        // cut, stay
        Fixture h;
        h.text(0, 0, wide);
        h.text(1, 0, wide);
        h.text(1, 1, QStringLiteral(" "));
        h.build();
        const QImage lines = h.render();
        QCOMPARE(px(lines, 63, 1), kWhite);                           // (row 0: the line between A1 and B1 is under the text)
        QCOMPARE(px(lines, 63, 21), h.options.gridColor.rgb());       // (row 1: B2 holds something: the line stays)
        QCOMPARE(px(lines, 3 * 64 + 63, 1), kWhite);                  // (the text of 40 W's runs over the next five or six cells)
        // the text's color is the font's
        Fixture g;
        FontSpec red;
        red.name = QStringLiteral("Arial");
        red.size = 14;
        red.bold = true;
        red.color.kind = ColorSpec::Kind::Rgb;
        red.color.rgb = 0xFF0000;
        g.styles.fonts.append(red);
        CellFormat fmt;
        fmt.font = 1;
        g.text(0, 0, QStringLiteral("III"), g.format(fmt));
        g.options.gridlines = false;
        g.build();
        const QImage r = g.render();
        QVERIFY(pixelsOf(r, QRect(0, 0, 64, 20), QColor(0xFF0000).rgb()) >= 3);
    }

    // A number too wide for its cell is "#"s that fill it, and counted.
    void aNumberTooWideIsDrawnAsHashes()
    {
        Fixture f;
        CellFormat money;
        money.numFmtId = 164;
        f.styles.numFmts.insert(164, QStringLiteral("#,##0.00"));
        const quint32 style = f.format(money);
        f.number(0, 0, 123456789012.5, style);
        f.number(1, 0, 5, style);
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        QVERIFY2(f.painter->stats().hashed == 1, qPrintable(QString::number(f.painter->stats().hashed) + QStringLiteral(" hashed; ") + fontReport(QStringLiteral("5.00"))));
        QVERIFY(inkIn(img, QRect(0, 0, 64, 20)) > 20);
        QCOMPARE(inkIn(img, QRect(64, 0, 64, 20)), 0);   // (not spilled)
        QVERIFY(inkIn(img, QRect(0, 20, 64, 20)) > 5);
    }

    // A merged range is one filled rectangle (its gridlines inside gone), its first cell's text is in it, and what the other cells hold is not drawn.
    void aMergedRangeIsDrawnAsOneCell()
    {
        Fixture f;
        f.sheet.merges = {mergeOf(1, 1, 2, 2), mergeOf(4, 1, 5, 2)};
        f.blank(1, 1, f.fill(PatternKind::Solid, 0xFFFF00));
        f.text(2, 2, QStringLiteral("Hidden under the merge"));
        f.text(4, 2, QStringLiteral("Not shown either"));
        f.text(4, 1, QStringLiteral("M"));
        f.build();
        const QImage img = f.render();
        const QRgb yellow = QColor(0xFFFF00).rgb();
        QCOMPARE(px(img, 70, 25), yellow);
        QCOMPARE(px(img, 127, 25), yellow);        // (the gridline between its two columns is not there)
        QCOMPARE(px(img, 190, 59), yellow);        // (the last pixel of its far corner: a fill covers its own edge)
        QCOMPARE(inkIn(img, QRect(64, 20, 128, 40)), 0);
        // without a fill: the white, with no gridline inside, and the gridline round it
        QCOMPARE(px(img, 127, 85), kWhite);
        QCOMPARE(px(img, 70, 99), kWhite);
        QCOMPARE(px(img, 191, 85), f.options.gridColor.rgb());
        QCOMPARE(px(img, 70, 119), f.options.gridColor.rgb());
        QVERIFY(inkIn(img, QRect(64, 80, 128, 40)) > 3);   // (the "M")
        QCOMPARE(f.painter->stats().mergesDrawn, qint64(2));
    }

    // Frozen rows and columns stay where they are while the rest scrolls, each drawn in its own part of the window.
    void frozenPartsStayWhileTheRestScrolls()
    {
        Fixture f;
        f.sheet.view.frozen = true;
        f.sheet.view.frozenRows = 1;
        f.sheet.view.frozenCols = 1;
        f.sheet.view.paneRow = 2;     // the part that scrolls begins at row 2, column 2
        f.sheet.view.paneCol = 2;
        f.text(0, 0, QStringLiteral("Corner"));
        f.text(0, 3, QStringLiteral("Top"));
        f.text(3, 0, QStringLiteral("Left"));
        f.text(3, 3, QStringLiteral("Body"));
        f.text(1, 0, QStringLiteral("Gone"));   // (a row scrolled out of view under the frozen row)
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        // the scrolling part begins at row 2 and column 2, shown right of the frozen column and under the frozen row: row 3 is the second line, column 3 the second column
        QVERIFY(inkIn(img, QRect(0, 0, 64, 20)) > 5);            // the corner
        QVERIFY(inkIn(img, QRect(64 + 64, 0, 64, 20)) > 3);      // "Top": column 3, in the first line of the part that scrolls
        QVERIFY(inkIn(img, QRect(0, 20 + 20, 64, 20)) > 3);      // "Left": row 3, in the frozen column
        QVERIFY(inkIn(img, QRect(64 + 64, 20 + 20, 64, 20)) > 3);  // "Body"
        QCOMPARE(inkIn(img, QRect(0, 20, 64, 20)), 0);           // "Gone" is not on the screen (row 1 is above the part that scrolls)
        // the same sheet scrolled: the frozen parts do not move
        f.grid.layout().setScroll(128, 40);   // (the part that scrolls begins at column 3 and row 3)
        const QImage moved = f.render();
        QVERIFY(inkIn(moved, QRect(0, 0, 64, 20)) > 5);
        QVERIFY(inkIn(moved, QRect(64, 0, 64, 20)) > 3);         // "Top" is now in the first column of the part that scrolls
        QVERIFY(inkIn(moved, QRect(0, 20, 64, 20)) > 3);         // "Left" is under the frozen row
    }

    // The selection is the cells tinted, a border of the accent color, and the active cell left plain; the headers of what is selected are marked.
    void theSelectionIsTintedBorderedAndTheHeadersShowIt()
    {
        Fixture f;
        f.options.gridlines = false;
        f.build();
        Navigator nav(&f.grid.layout(), nullptr);
        Selection s;
        s.activeRow = 1;
        s.activeCol = 1;
        s.range = GridRange{1, 1, 2, 2};
        f.options.selection = &s;
        QImage img = f.render();
        const QRgb accent = f.options.accent.rgb();
        QCOMPARE(px(img, 64 + 10, 20), accent);                 // the border, 2 pixels, inside the range
        QCOMPARE(px(img, 64 + 10, 21), accent);
        QCOMPARE(px(img, 64, 30), accent);
        QCOMPARE(px(img, 64 * 3 - 1, 30), accent);
        QCOMPARE(px(img, 10, 10), kWhite);                       // outside: plain
        QCOMPARE(px(img, 64 + 10, 20 + 10), kWhite);             // the active cell: plain
        QVERIFY(px(img, 64 * 2 + 20, 20 + 10) != kWhite);        // the others: tinted
        QVERIFY(px(img, 64 * 2 + 20, 20 + 10) != accent);
        // with headers: the selected ones are marked
        Fixture g;
        g.sheet.view.showHeadings = true;
        g.options.headers = true;
        g.options.gridlines = false;
        g.build();
        Selection col;
        col.activeRow = 0;
        col.activeCol = 2;
        col.whole = Selection::Whole::Columns;
        col.range = GridRange{0, 2, 1048575, 2};
        g.options.selection = &col;
        const QImage h = g.render();
        QCOMPARE(px(h, 40 + 64 * 2 + 3, 3), g.options.headerSelected.rgb());
        QCOMPARE(px(h, 40 + 64 * 1 + 3, 3), g.options.headerBackground.rgb());
        QVERIFY(g.painter->stats().headersDrawn > 5);
        // the corner
        QCOMPARE(px(h, 3, 3), g.options.headerBackground.rgb());
    }

    // A screen costs what is on it: the same rows, columns, and cells whether the sheet has a hundred rows or a million, wherever it is scrolled to; and what was made is kept.
    void aScreenCostsWhatIsOnItWhateverTheSheet()
    {
        Fixture f;
        f.size = QSize(1280, 800);
        for (quint32 r = 0; r < 100000; ++r)
            for (quint32 c = 0; c < 20; ++c) f.number(r, c, r * 20.0 + c);
        f.build();
        f.options.headers = false;
        f.render();
        const PaintStats top = f.painter->stats();
        QVERIFY2(top.rowsVisited <= 45 && top.rowsVisited >= 30, qPrintable(QString::number(top.rowsVisited)));
        QVERIFY2(top.colsVisited <= 25, qPrintable(QString::number(top.colsVisited)));
        QVERIFY2(top.cellsLooked <= 2000, qPrintable(QString::number(top.cellsLooked)));
        QVERIFY(!top.truncated);
        QVERIFY(top.textRuns >= 400 && top.textRuns <= 1000);
        QCOMPARE(top.textsMade, top.textRuns);
        // scrolled to row 90,000 of the same sheet
        f.grid.layout().setScroll(0, 90000 * 20);
        f.render();
        const PaintStats far = f.painter->stats();
        QVERIFY(far.rowsVisited <= 45 && far.cellsLooked <= 2000 && far.textRuns <= 1000);
        QVERIFY(std::abs(far.rowsVisited - top.rowsVisited) <= 1);
        // the same screen again: nothing is made again
        f.render();
        QCOMPARE(f.painter->stats().textsMade, qint64(0));
        QCOMPARE(f.painter->stats().looksMade, qint64(0));
        // a strip the size of one row (what a scroll by a row leaves to draw) costs about one row
        f.render(QRect(0, 0, 1280, 20));
        const PaintStats strip = f.painter->stats();
        QVERIFY2(strip.rowsVisited <= 3, qPrintable(QString::number(strip.rowsVisited)));
        QVERIFY2(strip.cellsLooked <= 150, qPrintable(QString::number(strip.cellsLooked)));
        // and a clip with nothing in it (a part of the window with no cells in it) costs nothing
        f.render(QRect(0, 0, 0, 0).adjusted(0, 0, -1, -1));
    }

    // Rows and columns of one pixel in a window of 1,920 by 1,080: no more rows, columns, and cells are looked at than the bounds, and the paint says some were left out.
    void rowsAndColumnsOfOnePixelAreBounded()
    {
        Fixture f;
        f.size = QSize(1920, 1080);
        for (quint32 r = 0; r < 3000; r += 3)
            for (quint32 c = 0; c < 3000; c += 7) f.number(r, c, r + c);
        f.build();
        f.grid.layout().rows.setDefaultSize(1);
        f.grid.layout().cols.setDefaultSize(1);
        f.grid.layout().setViewport(f.size);
        f.render();
        const PaintStats s = f.painter->stats();
        QVERIFY(s.rowsVisited <= GridPainter::kMaxRows && s.colsVisited <= GridPainter::kMaxCols);
        QVERIFY(s.cellsLooked <= GridPainter::kMaxCells + GridPainter::kMaxCols);
        QVERIFY(s.textRuns <= GridPainter::kMaxTextRuns);
        QCOMPARE(s.textRuns, qint64(0));   // (text of one pixel is not drawn)
        QVERIFY(s.rowsVisited >= 20 && s.colsVisited >= 1000);   // (as many as the cells' bound lets: 60,000 cells)
        QVERIFY(s.truncated);               // (more cells are on the screen than the bound)
    }

    // A sheet whose rows are all hidden but three costs three rows; a row of 4,000 points (which the grid brings to the most it allows) is one row.
    void hiddenRowsCostNothingAndTallRowsAreOneRow()
    {
        Fixture f;
        for (quint32 r = 0; r < 200; ++r) f.text(r, 0, QStringLiteral("row"));
        f.build();
        f.grid.layout().rows.setCount(1048576);
        std::vector<Axis::Place> places;
        for (qint64 r = 0; r < 1048576; ++r)
            if (r != 5 && r != 50 && r != 150) places.push_back({r, 0});
        f.grid.layout().rows.assign(std::move(places));
        f.render();
        QCOMPARE(f.painter->stats().rowsVisited, qint64(3));
        // a row 20,000 pixels tall
        Fixture g;
        g.text(0, 0, QStringLiteral("tall"));
        g.text(1, 0, QStringLiteral("next"));
        g.build();
        g.grid.layout().rows.setSize(0, 1000000);
        const QImage img = g.render();
        QVERIFY(g.painter->stats().rowsVisited <= 2);
        QVERIFY(inkIn(img, QRect(0, 380, 64, 20)) >= 0);
    }

    // A hundred thousand merged ranges, one for each row: the ones on the screen are drawn. A hundred thousand on top of one another stay within the bounds.
    void manyMergedRangesAreBounded()
    {
        Fixture f;
        for (quint32 r = 0; r < 100000; ++r) f.sheet.merges.append(mergeOf(r, 0, r, 1));
        for (quint32 r = 0; r < 100000; ++r) f.text(r, 0, QStringLiteral("m"));
        f.build();
        f.render();
        const PaintStats s = f.painter->stats();
        QVERIFY2(s.mergesDrawn >= 18 && s.mergesDrawn <= 25, qPrintable(QString::number(s.mergesDrawn)));
        QVERIFY(s.mergeMarks <= 100);
        // each a different size, all taking in the screen
        Fixture g;
        for (quint32 i = 0; i < 100000; ++i) g.sheet.merges.append(mergeOf(0, 0, 20 + i % 900, 1 + i / 900));
        g.text(0, 0, QStringLiteral("x"));
        g.build();
        g.render();
        const PaintStats h = g.painter->stats();
        QVERIFY2(h.mergesDrawn <= GridPainter::kMaxMerges, qPrintable(QString::number(h.mergesDrawn)));
        QVERIFY2(h.mergeMarks <= GridPainter::kMaxMergeMarks, qPrintable(QString::number(h.mergeMarks)));
        QVERIFY(h.cellsLooked <= GridPainter::kMaxCells * 2);
    }

    // Two hundred texts of 32,000 characters: each is cut to what a cell shows, so a screen of them costs no more characters than that.
    void veryLongTextsCostWhatACellShows()
    {
        Fixture f;
        const QString huge(32000, QLatin1Char('x'));
        for (quint32 r = 0; r < 20; ++r)
            for (quint32 c = 0; c < 10; ++c) f.text(r, c, huge);
        f.build();
        f.render();
        const PaintStats s = f.painter->stats();
        QVERIFY2(s.textChars <= 200 * 1024, qPrintable(QString::number(s.textChars)));
        QVERIFY(s.textRuns <= 200 && s.textRuns >= 150);
    }

    // A million cell formats, each used once by a cell on the screen: the looks made are those of the cells drawn.
    void aMillionCellFormatsCostTheCellsDrawn()
    {
        Fixture f;
        for (quint32 i = 0; i < 1000000; ++i) {
            CellFormat c;
            c.indent = quint8(i % 16);
            c.numFmtId = 0;
            f.styles.formats.append(c);
            f.styles.cellXfs.append(0);
        }
        for (quint32 r = 0; r < 20; ++r)
            for (quint32 c = 0; c < 10; ++c) f.number(r, c, 1, 1 + (r * 10 + c) * 4001);
        f.build();
        f.render();
        const PaintStats s = f.painter->stats();
        QVERIFY2(s.looksMade <= 20 * 10 + 5 && s.looksMade >= 100, qPrintable(QString::number(s.looksMade)));
    }

    // A text with nothing in it (a cell that holds the empty string), a font the machine does not have, and letters of other scripts than Latin are drawn, and never fail.
    void emptyTextsMissingFontsAndOtherScriptsAreDrawn()
    {
        Fixture f;
        FontSpec absent;
        absent.name = QStringLiteral("No Such Font Anywhere 123");
        absent.size = 12;
        f.styles.fonts.append(absent);
        CellFormat fmt;
        fmt.font = 1;
        const quint32 style = f.format(fmt);
        FontSpec underlined;
        underlined.name = QStringLiteral("Arial");
        underlined.size = 8;
        underlined.underline = Underline::Single;
        underlined.strike = true;
        f.styles.fonts.append(underlined);
        CellFormat under;
        under.font = 2;
        const quint32 underStyle = f.format(under);
        f.text(0, 0, QString());                                   // the empty string
        f.text(2, 0, QString(), underStyle);                       // the empty string, underlined (a run of no glyphs with a line under it)
        f.text(2, 1, QStringLiteral(" "), underStyle);
        f.text(0, 1, QStringLiteral("text"), style);               // a font that is not there
        f.text(0, 2, QString::fromUtf8("日本語のテキスト"));           // other letters
        f.text(0, 3, QString::fromUtf8("مرحبا بالعالم"));
        f.text(0, 4, QStringLiteral("\t\x01 control letters"));
        f.text(1, 0, QString(500, QChar(0x00E9)));                 // letters of the Latin set that are not ASCII
        f.number(1, 1, 5, style);
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        QVERIFY(inkIn(img, QRect(0, 0, 64 * 5, 20)) > 20);
        QVERIFY(inkIn(img, QRect(0, 20, 64, 20)) > 20);
        QVERIFY(inkIn(img, QRect(64, 20, 64, 20)) > 0);
        QCOMPARE(f.painter->stats().textRuns >= 5, true);
        // the same at a zoom that makes the letters small
        f.grid.layout().setZoom(0.7);
        const QImage small = f.render();
        QVERIFY(inkIn(small, QRect(0, 0, 45 * 5, 14)) > 5);
        f.grid.layout().setZoom(2.5);
        f.render();
        // in rows too low for the letters, which are then cut to the row
        f.grid.layout().setZoom(1.0);
        f.grid.layout().rows.setDefaultSize(9);
        const QImage low = f.render();
        QVERIFY(inkIn(low, QRect(0, 0, 64 * 5, 9)) > 3);
    }

    // The headers: the letters along the top and the numbers down the side, counted, and the corner.
    void headersAreLettersAndNumbers()
    {
        Fixture f;
        f.sheet.view.showHeadings = true;
        f.options.headers = true;
        f.build();
        const QImage img = f.render();
        QVERIFY2(f.painter->stats().headersDrawn >= 10 + 10, qPrintable(QString::number(f.painter->stats().headersDrawn)));
        QCOMPARE(px(img, 3, 3), f.options.headerBackground.rgb());
        QCOMPARE(px(img, 40 + 3, 3), f.options.headerBackground.rgb());
        QCOMPARE(px(img, 3, 20 + 3), f.options.headerBackground.rgb());
        QVERIFY(inkIn(img, QRect(40, 0, 64, 20)) > 2);      // "A"
        QVERIFY(inkIn(img, QRect(0, 20, 40, 20)) > 2);      // "1"
        // the cells begin after them
        QCOMPARE(px(img, 40 + 63, 25), f.options.gridColor.rgb());
        // none with them off
        Fixture g;
        g.build();
        g.render();
        QCOMPARE(g.painter->stats().headersDrawn, qint64(0));
    }

    // A hair border round a merged range of a hundred thousand rows is drawn as the dashes of the part that is on the screen: the steps a paint takes are those of the window,
    // not of the length of the range (which is two million pixels).
    void aDashedBorderOfATallMergeCostsWhatIsOnTheScreen()
    {
        Fixture f;
        BorderSpec b;
        b.left = Fixture::side(BorderStyle::Hair, 0x000000);
        b.right = Fixture::side(BorderStyle::Hair, 0x000000);
        b.top = Fixture::side(BorderStyle::Hair, 0x000000);
        b.bottom = Fixture::side(BorderStyle::Hair, 0x000000);
        const quint32 hair = f.border(b);
        for (quint32 r : {0u, 99999u})
            for (quint32 c : {0u, 1u}) f.blank(r, c, hair);
        f.sheet.merges.append(mergeOf(0, 0, 99999, 1));
        f.build();
        f.render();
        const PaintStats s = f.painter->stats();
        QVERIFY2(s.dashSteps <= 3000, qPrintable(QStringLiteral("%1 dash steps").arg(s.dashSteps)));
        QVERIFY(s.borderSegments >= 2);
    }

    // The tallest default row (409.5 points) at the largest zoom, with a hair border on every cell: each edge is drawn for the part of it that is on the screen.
    void dashedBordersAtTheLargestRowsAndZoomCostTheWindow()
    {
        Fixture f;
        BorderSpec b;
        b.left = Fixture::side(BorderStyle::Dashed, 0x000000);
        b.right = Fixture::side(BorderStyle::Dashed, 0x000000);
        b.top = Fixture::side(BorderStyle::Dashed, 0x000000);
        b.bottom = Fixture::side(BorderStyle::Dashed, 0x000000);
        const quint32 dashed = f.border(b);
        for (quint32 r = 0; r < 3; ++r)
            for (quint32 c = 0; c < 4; ++c) f.blank(r, c, dashed);
        f.build();
        f.grid.layout().rows.setDefaultSize(546);   // (409.5 points)
        f.grid.layout().setZoom(4.0);
        f.grid.layout().setViewport(f.size);
        f.render();
        const PaintStats s = f.painter->stats();
        QVERIFY2(s.dashSteps <= 4000, qPrintable(QStringLiteral("%1 dash steps").arg(s.dashSteps)));
        QVERIFY(s.borderSegments >= 4);
    }

    // A font too small to read is not drawn, and costs nothing: no text is made for the cell, and no neighbor is looked at for it to spill into.
    void textTooSmallToReadMakesNoText()
    {
        Fixture f;
        f.size = QSize(1280, 800);
        for (quint32 r = 0; r < 100; ++r)
            for (quint32 c = 0; c < 10; ++c) f.text(r, c, QStringLiteral("a text that does not fit its cell"));
        f.build();
        f.grid.layout().setZoom(0.1);
        f.grid.layout().setViewport(f.size);
        f.render();
        const PaintStats s = f.painter->stats();
        QCOMPARE(s.textRuns, qint64(0));
        QCOMPARE(s.textsMade, qint64(0));
        QCOMPARE(s.spillProbes, qint64(0));
        // (and the same cells at 100% have their text)
        f.grid.layout().setZoom(1.0);
        f.painter->resetFonts();
        f.render();
        QVERIFY(f.painter->stats().textRuns > 100);
    }

    // Every merged range on the screen is drawn, however many (a form of merged cells at a small zoom has thousands: the old bound was two thousand, and the rest were left out).
    void everyMergeOnTheScreenIsDrawn()
    {
        Fixture f;
        f.size = QSize(1280, 800);
        for (quint32 r = 0; r < 160; ++r)
            for (quint32 c = 0; c < 40; ++c) f.sheet.merges.append(mergeOf(r, c * 2, r, c * 2 + 1));
        f.build();
        f.grid.layout().setZoom(0.25);   // (cells of 16 by 5 pixels: eighty columns and a hundred and sixty rows)
        f.grid.layout().setViewport(f.size);
        f.render();
        const PaintStats s = f.painter->stats();
        QCOMPARE(s.mergesDrawn, qint64(6400));
        QCOMPARE(s.mergesLeftOut, qint64(0));
        QVERIFY(!s.truncated);
    }

    // A file that stacks merged ranges over the screen (a hundred and sixty thousand, each a different size, all taking in what is on it) is drawn as far as a few screens of fill, and
    // every merge that is not drawn is counted: the ones that cross the block and the ones left out of the list add up to all of them.
    void mergesPastTheBoundsAreCounted()
    {
        Fixture f;
        for (quint32 r = 1; r <= 400; ++r)
            for (quint32 c = 1; c <= 400; ++c) f.sheet.merges.append(mergeOf(0, 0, r, c));
        f.build();
        f.render();
        const PaintStats s = f.painter->stats();
        QCOMPARE(s.mergesDrawn + s.mergesLeftOut, qint64(160000));
        QVERIFY(s.mergesLeftOut > 150000);
        QVERIFY(s.mergesDrawn >= 50 && s.mergesDrawn <= 2000);
        QVERIFY(s.truncated);
    }

    // Text turned by an angle is placed by the cell's alignment, not centered: the box it turns in is at the corner or edge the alignment names (turned up from the bottom left by
    // default), and text that is turned fully up or down runs along the cell's height.
    void turnedTextIsPlacedByTheAlignment()
    {
        struct Case {
            int rotation;
            HAlign h;
            VAlign v;
            const char *what;
        };
        const Case cases[] = {{90, HAlign::Left, VAlign::Bottom, "up, left and bottom"}, {90, HAlign::Right, VAlign::Top, "up, right and top"}, {90, HAlign::Center, VAlign::Center, "up, centered"},
                              {45, HAlign::Left, VAlign::Bottom, "45 degrees, left and bottom"}, {135, HAlign::Left, VAlign::Top, "down at 45 degrees, left and top"},
                              {180, HAlign::Right, VAlign::Bottom, "straight down, right and bottom"}};
        for (const Case &c : cases) {
            Fixture f;
            CellFormat fmt;
            fmt.rotation = quint8(c.rotation);
            fmt.horizontal = c.h;
            fmt.vertical = c.v;
            f.text(0, 0, QStringLiteral("HHH"), f.format(fmt));
            f.options.gridlines = false;
            f.build();
            f.grid.layout().rows.setDefaultSize(80);
            const QImage img = f.render();
            const QRect cell(0, 0, 64, 80);
            const QRect box = inkBox(img, cell);
            QVERIFY2(box.isValid(), c.what);
            const QString say = QStringLiteral("%1: ink at %2,%3 to %4,%5").arg(QLatin1String(c.what)).arg(box.left()).arg(box.top()).arg(box.right()).arg(box.bottom());
            if (c.h == HAlign::Left) QVERIFY2(box.left() <= 8, qPrintable(say));
            if (c.h == HAlign::Right) QVERIFY2(box.right() >= 64 - 10, qPrintable(say));
            if (c.h == HAlign::Center) QVERIFY2(std::abs(box.center().x() - 32) <= 6, qPrintable(say));
            if (c.v == VAlign::Bottom) QVERIFY2(box.bottom() >= 80 - 10, qPrintable(say));
            if (c.v == VAlign::Top) QVERIFY2(box.top() <= 8, qPrintable(say));
            if (c.v == VAlign::Center) QVERIFY2(std::abs(box.center().y() - 40) <= 6, qPrintable(say));
            if (c.rotation == 90 || c.rotation == 180) QVERIFY2(box.height() > box.width(), qPrintable(say));   // (along the cell's height)
        }
    }

    // A double underline is two lines, a single one is one, and an accounting underline is as wide as the cell; the lines are under the text.
    void underlinesAreDrawnAsTheyAre()
    {
        auto lines = [](const QImage &img, const QImage &plain, const QRect &area, int fromX, int toX) {
            // the rows where the ink reaches across the text, as groups of rows next to each other; a row the
            // letters fill as well without the underline is the letters' own (a font's flat tops can reach across)
            auto full = [&](const QImage &im, int y) {
                int dark = 0;
                for (int x = fromX; x <= toX; ++x)
                    if (qRed(im.pixel(x, y)) < 200) ++dark;
                return dark >= (toX - fromX + 1) * 9 / 10;
            };
            QVector<int> groups;
            bool inLine = false;
            for (int y = area.top(); y <= area.bottom(); ++y) {
                const bool line = full(img, y) && !full(plain, y);
                if (line && !inLine) groups.append(y);
                inLine = line;
            }
            return groups;
        };
        struct Case {
            Underline kind;
            int groups;
            bool wide;
        };
        for (const Case &c : {Case{Underline::Single, 1, false}, Case{Underline::Double, 2, false}, Case{Underline::SingleAccounting, 1, true}, Case{Underline::DoubleAccounting, 2, true}}) {
            auto draw = [](Underline kind) {
                Fixture f;
                FontSpec font;
                font.name = QStringLiteral("Calibri");
                font.size = 11;
                font.underline = kind;
                f.styles.fonts.append(font);
                CellFormat fmt;
                fmt.font = 1;
                f.text(0, 0, QStringLiteral("mmm"), f.format(fmt));
                f.options.gridlines = false;
                f.build();
                f.grid.layout().rows.setDefaultSize(30);
                return f.render();
            };
            const QImage img = draw(c.kind), plain = draw(Underline::None);
            const QRect text = inkBox(img, QRect(0, 0, 64, 30), 200);
            QVERIFY(text.isValid());
            const int from = c.wide ? 3 : text.left() + 1, to = c.wide ? 60 : text.right() - 1;
            const QVector<int> found = lines(img, plain, QRect(0, 0, 64, 30), from, to);
            QStringList rows;
            for (int y : found) rows << QString::number(y);
            QVERIFY2(found.size() == c.groups, qPrintable(QStringLiteral("underline %1: %2 lines, at rows %3 (text %4 to %5)").arg(int(c.kind)).arg(found.size()).arg(rows.join(QLatin1Char(','))).arg(text.top()).arg(text.bottom())));
            if (c.groups == 2) QVERIFY2(found[1] - found[0] >= 2, qPrintable(QStringLiteral("the lines of %1 are %2 apart").arg(int(c.kind)).arg(found[1] - found[0])));
        }
    }

    // Fill alignment repeats the text to fill the cell's width, and center across selection puts the text in the middle of the cell and the empty cells beside it that are formatted so.
    void fillRepeatsTheTextAndCenterAcrossCentersOverEmptyCells()
    {
        Fixture f;
        CellFormat fill;
        fill.horizontal = HAlign::Fill;
        CellFormat plain;
        plain.horizontal = HAlign::Left;
        f.text(0, 0, QStringLiteral("ab"), f.format(fill));
        f.text(1, 0, QStringLiteral("ab"), f.format(plain));
        CellFormat across;
        across.horizontal = HAlign::CenterContinuous;
        const quint32 acrossStyle = f.format(across);
        f.text(2, 0, QStringLiteral("mid"), acrossStyle);
        f.blank(2, 1, acrossStyle);
        f.blank(2, 2, acrossStyle);
        CellFormat centered;
        centered.horizontal = HAlign::Center;
        f.text(3, 0, QStringLiteral("mid"), f.format(centered));
        f.options.gridlines = false;
        f.build();
        const QImage img = f.render();
        QVERIFY(inkIn(img, QRect(40, 0, 22, 20)) > 10);            // (the text reaches the right of the cell)
        QCOMPARE(inkIn(img, QRect(40, 20, 22, 20)), 0);           // (a left-aligned one does not)
        QVERIFY(inkIn(img, QRect(3, 0, 20, 20)) > 5);
        const QRect across3 = inkBox(img, QRect(0, 40, 3 * 64, 20));
        QVERIFY(across3.isValid());
        QVERIFY2(std::abs(across3.center().x() - 96) <= 8, qPrintable(QString::number(across3.center().x())));   // (the middle of three cells)
        const QRect single = inkBox(img, QRect(0, 60, 3 * 64, 20));
        QVERIFY2(std::abs(single.center().x() - 32) <= 8, qPrintable(QString::number(single.center().x())));
    }

    // A sheet that reads right to left is drawn from the right: column A is at the right edge, the headers are at the right, the letters are not turned round, and a text that a cell
    // aligns to the left is at the left of its cell (a text with the general alignment is at the right).
    void aRightToLeftSheetIsDrawnFromTheRight()
    {
        auto make = [](bool rtl, HAlign h) {
            auto f = std::make_unique<Fixture>();
            f->sheet.view.rightToLeft = rtl;
            const quint32 green = f->fill(PatternKind::Solid, 0x00FF00);
            const quint32 red = f->fill(PatternKind::Solid, 0xFF0000);
            f->blank(0, 0, green);
            f->blank(0, 1, red);
            CellFormat fmt;
            fmt.horizontal = h;
            f->text(1, 0, QStringLiteral("Lq"), f->format(fmt));
            f->options.gridlines = false;
            f->build();
            return f;
        };
        auto ltr = make(false, HAlign::Left), rtl = make(true, HAlign::Left);
        const QImage a = ltr->render(), b = rtl->render();
        const int w = ltr->size.width();
        QCOMPARE(px(a, 10, 5), QRgb(0xFF00FF00));
        QCOMPARE(px(a, 64 + 10, 5), QRgb(0xFFFF0000));
        QCOMPARE(px(b, w - 1 - 10, 5), QRgb(0xFF00FF00));         // column A at the right edge
        QCOMPARE(px(b, w - 1 - 64 - 10, 5), QRgb(0xFFFF0000));    // and B to its left
        QCOMPARE(px(b, 10, 5), kWhite);
        // the text of the first column, at the left of its cell, is the same picture as the left-to-right sheet draws at the left of its cell (nothing is turned round)
        const QImage inLtr = a.copy(QRect(0, 20, 64, 20)), inRtl = b.copy(QRect(w - 64, 20, 64, 20));
        QVERIFY(inkIn(inLtr, QRect(0, 0, 64, 20)) > 5);
        QCOMPARE(inRtl, inLtr);
        // the general alignment of a text is the right of the cell in a sheet that reads right to left, and the left in the other
        auto generalLtr = make(false, HAlign::General), generalRtl = make(true, HAlign::General);
        const QImage g1 = generalLtr->render(), g2 = generalRtl->render();
        const QRect box1 = inkBox(g1, QRect(0, 20, 64, 20)), box2 = inkBox(g2, QRect(w - 64, 20, 64, 20));
        QVERIFY(box1.left() <= 8);
        QVERIFY2(box2.left() - (w - 64) <= 8, qPrintable(QString::number(box2.left())));   // (a Latin text begins at the left, in any sheet)
        // a number, and a text with no letter of either direction, are at the right of the cell in a sheet that reads right to left
        Fixture n;
        n.sheet.view.rightToLeft = true;
        n.number(1, 0, 5);
        n.text(2, 0, QStringLiteral("123"));
        n.options.gridlines = false;
        n.build();
        const QImage ni = n.render();
        QVERIFY(inkBox(ni, QRect(w - 64, 20, 64, 20)).left() - (w - 64) >= 35);
        QVERIFY(inkBox(ni, QRect(w - 64, 40, 64, 20)).left() - (w - 64) >= 35);
        // the headers are at the right
        Fixture h;
        h.sheet.view.rightToLeft = true;
        h.sheet.view.showHeadings = true;
        h.options.headers = true;
        h.build();
        const QImage hi = h.render();
        QCOMPARE(px(hi, w - 1 - 3, 3), h.options.headerBackground.rgb());            // the corner
        QCOMPARE(px(hi, w - 1 - 3, 20 + 3), h.options.headerBackground.rgb());       // the row numbers
        QVERIFY(inkIn(hi, QRect(w - 40, 20, 40, 20)) > 2);                            // "1"
        QVERIFY(inkIn(hi, QRect(w - 40 - 64, 0, 64, 20)) > 2);                        // "A"
    }

    // The direction of the text of a cell: the general alignment of a text follows it (right for a reading order that runs right to left, and for text that begins with a letter of such a
    // script when the order is the text's own), and numbers stay at the right.
    void thePlaceOfTextWithTheGeneralAlignmentFollowsItsDirection()
    {
        const QString hebrew = QString(QChar(0x05D0)) + QChar(0x05D1) + QChar(0x05D2);
        struct Case {
            int order;
            QString text;
            HAlign want;
        };
        const Case cases[] = {{0, QStringLiteral("abc"), HAlign::Left}, {0, hebrew, HAlign::Right}, {2, QStringLiteral("abc"), HAlign::Right}, {1, hebrew, HAlign::Left}, {0, QStringLiteral("123"), HAlign::Left}};
        for (const Case &c : cases) {
            Fixture f;
            CellFormat fmt;
            fmt.readingOrder = quint8(c.order);
            f.text(0, 0, c.text, f.format(fmt));
            f.build();
            const Shown sh = f.grid.shown(0, 0, *f.grid.cellAt(0, 0));
            QVERIFY2(sh.align == c.want, qPrintable(QStringLiteral("order %1, \"%2\": %3").arg(c.order).arg(c.text).arg(int(sh.align))));
        }
        // a sheet that reads right to left: the text of a cell of the context order is at the right, and a number too
        Fixture f;
        f.sheet.view.rightToLeft = true;
        f.text(0, 0, QStringLiteral("abc"));
        f.number(1, 0, 5);
        f.build();
        f.text(2, 0, QStringLiteral("123"));
        f.build();
        // (in the grid's own places, which are mirrored for the screen: a Latin text begins at the screen's left, which is the grid's right; a number, and a text with no letter, at the screen's right)
        QVERIFY(f.grid.shown(0, 0, *f.grid.cellAt(0, 0)).align == HAlign::Right);
        QVERIFY(f.grid.shown(1, 0, *f.grid.cellAt(1, 0)).align == HAlign::Left);
        QVERIFY(f.grid.shown(2, 0, *f.grid.cellAt(2, 0)).align == HAlign::Left);
    }

    // A color scale gives the cell its fill, a rule with a differential format changes the fill and the text color of the cells it holds, and the cells it does not hold stay as they were.
    void colorScalesAndDifferentialFormatsAreDrawn()
    {
        Fixture f;
        f.number(0, 0, 0);
        f.number(1, 0, 100);
        f.number(0, 1, 5);
        f.number(1, 1, 50);
        CondRule scale;
        scale.type = CondRule::Type::ColorScale;
        scale.ranges = {mergeOf(0, 0, 1, 0)};
        scale.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        scale.colors = {ColorSpec::fromRgb(0xFF0000), ColorSpec::fromRgb(0x0000FF)};
        scale.priority = 1;
        DifferentialFormat d;
        d.hasFill = true;
        d.fill.pattern = PatternKind::Solid;
        d.fill.bg = ColorSpec::fromRgb(0xFFEB9C);
        d.fontColor = ColorSpec::fromRgb(0xFF0000);
        d.bold = true;
        f.styles.dxfs.append(d);
        CondRule over;
        over.type = CondRule::Type::CellIs;
        over.op = CondRule::Operator::GreaterThan;
        over.formula1 = QStringLiteral("10");
        over.dxf = 0;
        over.ranges = {mergeOf(0, 1, 1, 1)};
        over.priority = 2;
        f.sheet.conditionalFormats = {scale, over};
        f.build();
        const QImage img = f.render();
        QCOMPARE(px(img, 3, 3), QRgb(0xFFFF0000));         // the lowest: the first color, over the whole cell
        QCOMPARE(px(img, 50, 16), QRgb(0xFFFF0000));
        QCOMPARE(px(img, 3, 23), QRgb(0xFF0000FF));        // the highest: the last
        QCOMPARE(px(img, 63, 39), QRgb(0xFF0000FF));       // (its gridline is covered, as a fill covers it)
        QCOMPARE(px(img, 64 + 3, 3), kWhite);              // 5 is not over 10
        QCOMPARE(px(img, 64 + 3, 23), QRgb(0xFFFFEB9C));   // 50 is: the fill of the differential format
        // the text of the second is in the font color of the format (ink of that color, and not of the default black)
        int reddish = 0, black = 0;
        for (int y = 20; y < 40; ++y)
            for (int x = 64; x < 128; ++x) {
                const QRgb c = img.pixel(x, y);
                if (qRed(c) > 200 && qGreen(c) < 90 && qBlue(c) < 90) ++reddish;
                if (qRed(c) < 60 && qGreen(c) < 60 && qBlue(c) < 60) ++black;
            }
        QVERIFY(reddish > 5);
        QCOMPARE(black, 0);
    }

    // A data bar is drawn inside the cell from the left, as long as its value is of the way from the lowest to the highest (between the shortest length and the longest), under the text,
    // and the text is left out when the rule says so.
    void dataBarsAreDrawnInTheCell()
    {
        auto make = [](bool showValue, bool gradient) {
            auto f = std::make_unique<Fixture>();
            f->number(0, 0, 0);
            f->number(1, 0, 50);
            f->number(2, 0, 100);
            CondRule bar;
            bar.type = CondRule::Type::DataBar;
            bar.ranges = {mergeOf(0, 0, 2, 0)};
            bar.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
            bar.colors = {ColorSpec::fromRgb(0x638EC6)};
            bar.showValue = showValue;
            bar.gradient = gradient;
            f->sheet.conditionalFormats = {bar};
            f->build();
            return f;
        };
        auto f = make(true, false);
        const QImage img = f->render();
        const QRgb blue = QRgb(0xFF638EC6);
        // the cell is 64 wide with a gridline, the bar is within 2 pixels of its sides: 59 wide; the lengths are 10, 50, and 90 percent of it
        QCOMPARE(px(img, 4, 10), blue);
        QCOMPARE(px(img, 3 + 6, 10), kWhite);                 // (the shortest bar is six pixels: 2 to 7)
        QCOMPARE(px(img, 4, 20 + 10), blue);
        QCOMPARE(px(img, 2 + 29, 20 + 3), blue);              // (the middle one reaches 2 + 30)
        QCOMPARE(px(img, 2 + 31, 20 + 3), kWhite);
        QCOMPARE(px(img, 2 + 52, 40 + 3), blue);              // (the longest, 53 pixels)
        QCOMPARE(px(img, 2 + 55, 40 + 3), kWhite);
        QCOMPARE(px(img, 10, 0), kWhite);                     // the bar has room above and below
        QCOMPARE(px(img, 4, 1), kWhite);
        QCOMPARE(px(img, 4, 18), kWhite);
        QVERIFY(f->painter->stats().bars == 3);
        // the value is drawn over it
        QVERIFY(inkIn(img, QRect(0, 40, 64, 20)) > 5);
        auto hidden = make(false, false);
        const QImage without = hidden->render();
        QCOMPARE(inkIn(without, QRect(0, 40, 64, 20)), 0);
        QCOMPARE(px(without, 2 + 52, 40 + 3), blue);
        // a gradient is solid at the left and lighter at the right
        auto gradient = make(false, true);
        const QImage g = gradient->render();
        const QRgb left = px(g, 3, 40 + 8), right = px(g, 2 + 50, 40 + 8);
        QVERIFY(qRed(left) <= qRed(blue) + 8);
        QVERIFY(qRed(right) > qRed(blue) + 40);
        QVERIFY(qRed(right) > qRed(left) + 30);
    }

    // An icon set puts an icon of the value's rank at the left of the cell, in its own colors, and text that begins at the left moves over to make room for it; with the rule saying to
    // leave the value out, there is no text.
    void iconsAreDrawnAtTheLeft()
    {
        auto make = [](bool showValue, HAlign align) {
            auto f = std::make_unique<Fixture>();
            CellFormat fmt;
            fmt.horizontal = align;
            const quint32 style = f->format(fmt);
            const double values[] = {1, 5, 9, 5};
            for (int i = 0; i < 4; ++i) f->number(quint32(i), i == 3 ? 1 : 0, values[i], style);
            CondRule icons;
            icons.type = CondRule::Type::IconSet;
            icons.iconSet = QStringLiteral("3TrafficLights1");
            icons.ranges = {mergeOf(0, 0, 2, 0)};
            icons.values = {{CondValue::Type::Number, QStringLiteral("0"), true}, {CondValue::Type::Number, QStringLiteral("4"), true}, {CondValue::Type::Number, QStringLiteral("8"), true}};
            icons.showValue = showValue;
            f->sheet.conditionalFormats = {icons};
            f->build();
            return f;
        };
        auto f = make(true, HAlign::Left);
        const QImage img = f->render();
        // (a disc of 16 pixels from x = 2, in the middle of a row of 20: its middle is solid)
        QCOMPARE(px(img, 10, 10), QRgb(0xFFD63E3E));
        QCOMPARE(px(img, 10, 30), QRgb(0xFFE8B41C));
        QCOMPARE(px(img, 10, 50), QRgb(0xFF2EA04F));
        QCOMPARE(px(img, 1, 10), kWhite);
        QVERIFY(f->painter->stats().icons == 3);
        // the text of the first column is to the right of the icon; the same text in a cell no rule holds begins at the left
        const QRect withIcon = inkBox(img, QRect(18, 22, 44, 16)), plain = inkBox(img, QRect(64, 62, 62, 16));
        QVERIFY(withIcon.isValid() && plain.isValid());
        QVERIFY2(withIcon.left() - 0 >= 18 + 2, qPrintable(QString::number(withIcon.left())));
        QVERIFY2(withIcon.left() >= plain.left() - 64 + 14, qPrintable(QStringLiteral("%1 against %2").arg(withIcon.left()).arg(plain.left() - 64)));
        // without the value
        auto hidden = make(false, HAlign::Left);
        const QImage without = hidden->render();
        QCOMPARE(px(without, 10, 30), QRgb(0xFFE8B41C));
        QCOMPARE(inkIn(without, QRect(18, 22, 44, 16)), 0);
        // a number at the right stays at the right
        auto right = make(true, HAlign::Right);
        const QRect rightInk = inkBox(right->render(), QRect(18, 22, 44, 16));
        QVERIFY(rightInk.isValid() && rightInk.right() >= 50);
    }

    // What the rules keep of the cells is bounded, and is let go of when it is full. A paint that fills it must still draw each cell as the rules say: what a paint holds is its own, so the
    // rules letting go of theirs in the middle of it cannot show in a cell.
    void aPaintThatFillsWhatTheRulesKeepDrawsAsOneThatDoesNot()
    {
        auto make = [](bool fillTheCache) {
            auto f = std::make_unique<Fixture>();
            for (quint32 i = 0; i < 20; ++i) f->number(i, 0, double(i * i));
            CondRule scale;
            scale.type = CondRule::Type::ColorScale;
            scale.ranges = {mergeOf(0, 0, 9999, 25)};
            scale.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
            scale.colors = {ColorSpec::fromRgb(0xFF0000), ColorSpec::fromRgb(0x0000FF)};
            scale.priority = 1;
            f->sheet.conditionalFormats = {scale};
            f->build();
            if (fillTheCache)
                for (quint32 row = 100; row < 100 + 7688; ++row)
                    for (quint32 col = 0; col < 26; ++col) f->grid.conditional().at(row, col);   // (199,888 cells asked about: what is kept is all but full)
            return f;
        };
        const QImage clean = make(false)->render();
        auto filled = make(true);
        const QImage full = filled->render();
        QCOMPARE(full, clean);
        QCOMPARE(filled->grid.conditional().cacheClearsInPaint(), qint64(0));   // (what was kept was not let go of in the middle of the paint)
        QCOMPARE(px(clean, 3, 3), QRgb(0xFFFF0000));   // (the cell of the lowest)
        QCOMPARE(px(clean, 3, 19 * 20 + 3), QRgb(0xFF0000FF));   // (the cell of the highest)
    }

    // The rules are asked about the cells that are drawn, not about the million rows of the sheet.
    void aBigSheetIsAskedAboutWhatIsDrawn()
    {
        Fixture f;
        for (quint32 i = 0; i < 100000; ++i) f.number(i, 0, double((i * 7919) % 100003));
        CondRule bar;
        bar.type = CondRule::Type::DataBar;
        bar.ranges = {mergeOf(0, 0, 99999, 0)};
        bar.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        bar.colors = {ColorSpec::fromRgb(0x638EC6)};
        CondRule scale;
        scale.type = CondRule::Type::ColorScale;
        scale.ranges = {mergeOf(0, 1, 99999, 1)};
        scale.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        scale.colors = {ColorSpec::fromRgb(0xFF0000), ColorSpec::fromRgb(0x00FF00)};
        for (quint32 i = 0; i < 100000; i += 1000) f.number(i, 1, double(i));
        f.sheet.conditionalFormats = {bar, scale};
        f.build();
        f.grid.conditional().settle();   // (the figures of a big sheet are made in slices between paints, not in the paint)
        f.render();
        QVERIFY2(f.painter->stats().condCells <= 40, qPrintable(QString::number(f.painter->stats().condCells)));   // (20 rows of two columns)
        QVERIFY(f.painter->stats().bars == 20);
        QVERIFY(f.painter->stats().condCells > 0);
    }

    // In a sheet that reads right to left a bar is at the right of the cell (the screen's) and grows to the left, and an icon is at the right with the text moved over from it.
    void barsAndIconsInARightToLeftSheet()
    {
        Fixture f;
        f.sheet.view.rightToLeft = true;
        f.number(0, 0, 0);
        f.number(1, 0, 100);
        f.number(2, 0, 9);
        CondRule bar;
        bar.type = CondRule::Type::DataBar;
        bar.ranges = {mergeOf(0, 0, 1, 0)};
        bar.values = {{CondValue::Type::Min, QString(), true}, {CondValue::Type::Max, QString(), true}};
        bar.colors = {ColorSpec::fromRgb(0x638EC6)};
        bar.gradient = false;
        bar.showValue = false;
        CondRule icons;
        icons.type = CondRule::Type::IconSet;
        icons.iconSet = QStringLiteral("3TrafficLights1");
        icons.ranges = {mergeOf(2, 0, 2, 0)};
        icons.values = {{CondValue::Type::Number, QStringLiteral("0"), true}, {CondValue::Type::Number, QStringLiteral("4"), true}, {CondValue::Type::Number, QStringLiteral("8"), true}};
        f.sheet.conditionalFormats = {bar, icons};
        f.build();
        const QImage img = f.render();
        const int w = f.size.width();
        // column A is at the right edge; the bar of the highest value (53 pixels) begins 2 pixels in from the right and is not at the cell's other side
        QCOMPARE(px(img, w - 1 - 4, 20 + 10), QRgb(0xFF638EC6));
        QCOMPARE(px(img, w - 1 - 53, 20 + 10), QRgb(0xFF638EC6));
        QCOMPARE(px(img, w - 1 - 57, 20 + 10), kWhite);
        // the icon is at the right too (its disc in the middle of its square), and the number is in the room left of it, not over it
        QCOMPARE(px(img, w - 1 - 10, 40 + 10), QRgb(0xFF2EA04F));
        QVERIFY(inkBox(img, QRect(w - 64, 42, 64 - 20, 16)).isValid());
        QCOMPARE(inkIn(img, QRect(w - 1 - 17, 42, 16, 16)), 0);
    }
};

JP_TEST_CLASS(GridPaintTests)
#include "tst_grid_paint.moc"
