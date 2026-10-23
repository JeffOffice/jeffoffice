// Tests of what the grid shows of cells (libs/shell/grid: look.h, text.h, gridsheet.h): how a cell format becomes a look (colors resolved, the widest digit of the default
// font), the text a value makes in its number format and when it is too wide for its cell, where text sits and how far it spills into empty cells, the geometry a sheet's
// file gives the grid, and the cache of what cells say. Fonts are never measured here: a meter of a fixed width stands in, so that the numbers are the same on every system.
#include "sheethelpers.h"

#include "calc/numfmt.h"
#include "grid/gridsheet.h"
#include "grid/look.h"
#include "grid/text.h"
#include "sheet/calculation.h"

#include <cmath>

using namespace jp::grid;

namespace {

// Seven for every character, whatever the font: the width of a digit of the usual default font.
class FixedMeter : public TextMeter {
public:
    double width(const QString &text, const CellLook &) const override { return 7.0 * text.size(); }
};

const FixedMeter kMeter;

CellLook lookWithFormat(const QString &code, HAlign h = HAlign::General)
{
    CellLook look;
    look.format = jp::calc::NumberFormat(code);
    look.hasFormat = !look.format.isGeneral();
    look.horizontal = h;
    return look;
}

Shown show(const jp::calc::Value &v, const CellLook &look, double width)
{
    return showValue(v, look, jp::calc::DateSystem::Date1900, width, 7, kMeter);
}

Cell numberCell(quint32 row, quint32 col, double v, quint32 style = 0)
{
    Cell c;
    c.row = row;
    c.col = quint16(col);
    c.type = CellType::Number;
    c.number = v;
    c.style = style;
    return c;
}

} // namespace

class GridCellTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A cell format becomes a look with every color resolved, the font, the fill, the borders, the alignment, and the number format of its code.
    void aCellFormatBecomesALookWithItsColorsResolved()
    {
        Styles s;
        s.theme[4] = 0x336699;
        FontSpec f;
        f.name = QStringLiteral("Arial");
        f.size = 14.5;
        f.bold = f.italic = f.strike = true;
        f.underline = Underline::Double;
        f.color.kind = ColorSpec::Kind::Rgb;
        f.color.rgb = 0xFF0000;
        s.fonts = {FontSpec(), f};
        FillSpec solid;
        solid.pattern = PatternKind::Solid;
        solid.fg.kind = ColorSpec::Kind::Theme;
        solid.fg.index = 4;
        FillSpec grid;
        grid.pattern = PatternKind::DarkGrid;
        grid.fg.kind = ColorSpec::Kind::Rgb;
        grid.fg.rgb = 0x00FF00;
        s.fills = {FillSpec(), solid, grid};
        BorderSpec b;
        b.left.style = BorderStyle::Thin;
        b.left.color.kind = ColorSpec::Kind::Auto;
        b.right.style = BorderStyle::MediumDashDot;
        b.right.color.kind = ColorSpec::Kind::Rgb;
        b.right.color.rgb = 0x0000FF;
        b.top.style = BorderStyle::Double;
        b.diagonal.style = BorderStyle::Dotted;
        b.diagonalUp = true;
        s.borders = {BorderSpec(), b};
        CellFormat plain, rich, patterned;
        rich.numFmtId = 164;
        rich.font = 1;
        rich.fill = 1;
        rich.border = 1;
        rich.horizontal = HAlign::Center;
        rich.vertical = VAlign::Top;
        rich.wrap = true;
        rich.indent = 2;
        rich.rotation = 45;
        patterned.fill = 2;
        s.formats = {plain, rich, patterned};
        s.cellXfs = {0, 164, 0};
        s.numFmts.insert(164, QStringLiteral("0.00;[Red]-0.00"));
        LookTable t(&s);
        const CellLook &l = t.look(1);
        QCOMPARE(l.family, QStringLiteral("Arial"));
        QCOMPARE(l.pointSize, 14.5);
        QVERIFY(l.bold && l.italic && l.strike);
        QVERIFY(l.underline == Underline::Double);
        QCOMPARE(l.textColor, QRgb(0xFFFF0000));
        QVERIFY(l.pattern == PatternKind::Solid);
        QCOMPARE(l.fg, QRgb(0xFF336699));
        QVERIFY(l.left.style == BorderStyle::Thin);
        QCOMPARE(l.left.color, QRgb(0xFF000000));   // (automatic is black)
        QVERIFY(l.right.style == BorderStyle::MediumDashDot);
        QCOMPARE(l.right.color, QRgb(0xFF0000FF));
        QVERIFY(l.top.style == BorderStyle::Double);
        QVERIFY(l.diagonalUp && !l.diagonalDown);
        QVERIFY(l.hasBorder() && l.hasFill());
        QVERIFY(l.horizontal == HAlign::Center && l.vertical == VAlign::Top);
        QVERIFY(l.wrap && !l.shrink);
        QCOMPARE(l.indent, 2);
        QCOMPARE(l.rotation, 45);
        QVERIFY(l.hasFormat);
        QCOMPARE(l.format.code(), QStringLiteral("0.00;[Red]-0.00"));
        // a pattern is the foreground over the background (which is white where the file names none)
        const CellLook &g = t.look(2);
        QVERIFY(g.pattern == PatternKind::DarkGrid);
        QCOMPARE(g.fg, QRgb(0xFF00FF00));
        QCOMPARE(g.bg, QRgb(0xFFFFFFFF));
        // the plain look: no fill, no border, black text, General
        const CellLook &p = t.look(0);
        QVERIFY(!p.hasFill() && !p.hasBorder() && !p.hasFormat);
        QCOMPARE(p.textColor, QRgb(0xFF000000));
        // a place the file doesn't have is the plain look too
        QVERIFY(!t.look(99).hasFill() && !t.look(99).hasFormat);
        // a solid fill with no color is no fill
        FillSpec none;
        none.pattern = PatternKind::Solid;
        s.fills = {none};
        LookTable t2(&s);
        QVERIFY(!t2.look(2).hasFill());
    }

    // Every look with the same font (family, size, and style) has the same font id, and each other font its own: the drawing keys its fonts by it.
    void looksWithTheSameFontShareAFontId()
    {
        Styles s;
        FontSpec a, b, c;
        a.name = QStringLiteral("Arial");
        a.size = 10;
        b = a;
        c = a;
        c.bold = true;
        s.fonts = {a, b, c};
        CellFormat x, y, z, w;
        x.font = 0;
        y.font = 1;   // another font entry of the same font
        z.font = 2;
        w.font = 0;
        w.fill = 0;
        s.formats = {x, y, z, w};
        s.cellXfs = {0, 0, 0, 0};
        LookTable t(&s);
        QCOMPARE(t.look(0).fontId, t.look(1).fontId);
        QCOMPARE(t.look(0).fontId, t.look(3).fontId);
        QVERIFY(t.look(0).fontId != t.look(2).fontId);
        // (and still the same after the looks were let go of)
        const quint32 id = t.look(2).fontId;
        t.trim();
        for (quint32 i = 0; i < 6000; ++i) t.look(i % 4 == 0 ? 0 : i % 4);
        t.trim();
        QCOMPARE(t.look(2).fontId, id);
    }

    // A table of a million cell formats holds only as many looks as a screen shows: the looks are let go of at the start of a screen, and are right after that.
    void aDeepTableOfCellFormatsHoldsFewLooks()
    {
        Styles s;
        FontSpec f;
        f.name = QStringLiteral("Calibri");
        s.fonts = {f};
        for (int i = 0; i < 20000; ++i) {
            CellFormat c;
            c.indent = quint8(i % 16);
            c.numFmtId = 0;
            s.formats.append(c);
            s.cellXfs.append(0);
        }
        LookTable t(&s);
        // (a screen asks for at most a few hundred at a time)
        for (quint32 i = 0; i < 20000; ++i) {
            if (i % 100 == 0) t.trim();
            QCOMPARE(t.look(i).indent, int(i % 16));
        }
        QCOMPARE(t.made(), qint64(20000));
        QVERIFY(t.held() <= LookTable::kMaxHeld + 100);
        // asked for again, a look held is not made again
        const qint64 before = t.made();
        t.look(19999);
        QCOMPARE(t.made(), before);
    }

    // The width of the widest digit that a column's characters are counted in follows the default font.
    void theWidestDigitFollowsTheDefaultFont()
    {
        auto widthFor = [](const QString &name, double size) {
            Styles s;
            FontSpec f;
            f.name = name;
            f.size = size;
            s.fonts = {f};
            return LookTable(&s).maxDigitWidth();
        };
        QCOMPARE(widthFor("Calibri", 11), 7);
        QCOMPARE(widthFor("Arial", 10), 7);
        QCOMPARE(widthFor("Arial", 11), 8);
        QCOMPARE(widthFor("Verdana", 10), 8);
        QCOMPARE(widthFor("Calibri", 12), 8);
        QCOMPARE(widthFor("Some Font Nobody Has", 11), 8);
        QCOMPARE(LookTable().maxDigitWidth(), 7);   // (no styles: the usual)
        QVERIFY(widthFor("Arial", 400) <= 255);
        QVERIFY(widthFor("Arial", 1) >= 1);
    }

    // The width and height numbers of a file are turned to the grid's pixels the way the format does it.
    void sizesOfAFileBecomePixels()
    {
        QCOMPARE(GridSheet::columnPixels(9.140625, 7), 64);
        QCOMPARE(GridSheet::columnPixels(12.7109375, 7), 89);
        QCOMPARE(GridSheet::columnPixels(0, 7), 0);
        QCOMPARE(GridSheet::columnPixels(255, 7), 1785);
        QCOMPARE(GridSheet::rowPixels(15), 20);
        QCOMPARE(GridSheet::rowPixels(12.75), 17);
        QCOMPARE(GridSheet::rowPixels(409.5), 546);
        QCOMPARE(GridSheet::rowPixels(0), 0);
    }

    // A number is shown through the format of its cell, in the color the format's section asks for; text, true and false, and an error as they are.
    void valuesAreShownThroughTheirFormats()
    {
        const CellLook money = lookWithFormat(QStringLiteral("#,##0.00;[Red]-#,##0.00"));
        Shown s = show(jp::calc::Value::number(1234.5), money, 120);
        QCOMPARE(s.text, QStringLiteral("1,234.50"));
        QCOMPARE(s.color, QRgb(0));
        s = show(jp::calc::Value::number(-1234.5), money, 120);
        QCOMPARE(s.text, QStringLiteral("-1,234.50"));
        QCOMPARE(s.color, QRgb(0xFF000000) | jp::calc::formatColorRgb(3));
        QVERIFY(s.kind == Kind::Number);
        s = show(jp::calc::Value::number(45366), lookWithFormat(QStringLiteral("yyyy-mm-dd")), 120);
        QCOMPARE(s.text, QStringLiteral("2024-03-15"));
        s = show(jp::calc::Value::number(0.256), lookWithFormat(QStringLiteral("0%")), 120);
        QCOMPARE(s.text, QStringLiteral("26%"));
        s = show(jp::calc::Value::text(QStringLiteral("hello")), CellLook(), 120);
        QCOMPARE(s.text, QStringLiteral("hello"));
        QVERIFY(s.kind == Kind::Text);
        s = show(jp::calc::Value::text(QStringLiteral("abc")), lookWithFormat(QStringLiteral("0;0;0;\"text: \"@")), 120);
        QCOMPARE(s.text, QStringLiteral("text: abc"));
        s = show(jp::calc::Value::boolean(true), CellLook(), 120);
        QCOMPARE(s.text, QStringLiteral("TRUE"));
        QVERIFY(s.kind == Kind::Bool);
        s = show(jp::calc::Value::error(jp::calc::ErrorKind::Div0), CellLook(), 120);
        QCOMPARE(s.text, QStringLiteral("#DIV/0!"));
        QVERIFY(s.kind == Kind::Error);
        s = show(jp::calc::Value(), CellLook(), 120);
        QVERIFY(s.kind == Kind::Empty && s.text.isEmpty());
        // a line break in a text that is not wrapped is a space; in one that is wrapped it stays
        s = show(jp::calc::Value::text(QStringLiteral("one\ntwo")), CellLook(), 200);
        QCOMPARE(s.text, QStringLiteral("one two"));
        CellLook wrapped;
        wrapped.wrap = true;
        s = show(jp::calc::Value::text(QStringLiteral("one\ntwo")), wrapped, 200);
        QCOMPARE(s.text, QStringLiteral("one\ntwo"));
        // the format of "fill with this" and the padding of "as wide as this" are kept for the drawing
        s = show(jp::calc::Value::number(5), lookWithFormat(QStringLiteral("_($* #,##0_)")), 200);
        QVERIFY(s.fillAt >= 0);
        QCOMPARE(s.fillChar, QChar(' '));
        // a value the format cannot show fills its cell with "#"
        s = show(jp::calc::Value::number(-5), lookWithFormat(QStringLiteral("yyyy-mm-dd")), 100);
        QVERIFY(s.hashes);
        QVERIFY(s.text.startsWith(QLatin1Char('#')));
    }

    // A number or date wider than its cell shows "#" in as many as fill the cell; text is cut by its cell, never replaced; "general" shows the digits that fit.
    void aNumberTooWideForItsCellShowsHashes()
    {
        const CellLook money = lookWithFormat(QStringLiteral("#,##0.00"));
        // 12 characters of 7 against a cell of 40, less 3 either side
        Shown s = show(jp::calc::Value::number(1234567.891), money, 40);
        QVERIFY(s.hashes);
        QCOMPARE(s.text, QStringLiteral("####"));
        QCOMPARE(s.width, 28.0);
        // wide enough: shown as it is
        s = show(jp::calc::Value::number(1234567.891), money, 100);
        QVERIFY(!s.hashes);
        QCOMPARE(s.text, QStringLiteral("1,234,567.89"));
        QCOMPARE(s.width, 84.0);
        // not even one "#" fits: one is shown, and the cell clips it
        s = show(jp::calc::Value::number(1234567.891), money, 8);
        QCOMPARE(s.text, QStringLiteral("#"));
        // text is not replaced, and neither are true, false, and errors
        s = show(jp::calc::Value::text(QStringLiteral("a very long text indeed")), CellLook(), 40);
        QVERIFY(!s.hashes);
        QCOMPARE(s.text, QStringLiteral("a very long text indeed"));
        QVERIFY(!show(jp::calc::Value::error(jp::calc::ErrorKind::Value), CellLook(), 20).hashes);
        // "general" takes the width of the column: 52 pixels fit 6 of the widest digit (less the 5 the cell keeps)
        QCOMPARE(charsFor(52, 7), 6);
        QCOMPARE(charsFor(3, 7), 1);
        s = show(jp::calc::Value::number(3.14159265358979), CellLook(), 52);
        QVERIFY(!s.hashes);
        QVERIFY(s.text.size() <= 6);
        QVERIFY(s.text.startsWith(QLatin1String("3.14")));
        s = show(jp::calc::Value::number(123456789012), CellLook(), 40);   // too many digits for 5 characters, and in scientific notation too wide
        QVERIFY(s.hashes || s.text.size() <= 5);
    }

    // "General" alignment depends on what the value is: numbers right, text left, true, false, and errors centered. What the format says is kept.
    void alignmentFollowsTheValueUnlessTheFormatSays()
    {
        QVERIFY(show(jp::calc::Value::number(1), CellLook(), 100).align == HAlign::Right);
        QVERIFY(show(jp::calc::Value::text(QStringLiteral("a")), CellLook(), 100).align == HAlign::Left);
        QVERIFY(show(jp::calc::Value::boolean(false), CellLook(), 100).align == HAlign::Center);
        QVERIFY(show(jp::calc::Value::error(jp::calc::ErrorKind::NA), CellLook(), 100).align == HAlign::Center);
        CellLook left = lookWithFormat(QStringLiteral("0"), HAlign::Left);
        QVERIFY(show(jp::calc::Value::number(1), left, 100).align == HAlign::Left);
        CellLook center;
        center.horizontal = HAlign::Center;
        QVERIFY(show(jp::calc::Value::text(QStringLiteral("a")), center, 100).align == HAlign::Center);
    }

    // The text of a line is at most 1,024 characters, as a cell shows at most that many; longer text costs no more to show.
    void aVeryLongTextIsCutToWhatACellShows()
    {
        const QString text(32000, QLatin1Char('x'));
        Shown s = show(jp::calc::Value::text(text), CellLook(), 100);
        QCOMPARE(s.text.size(), 1024);
        QCOMPARE(s.width, 1024 * 7.0);
        CellLook wrapped;
        wrapped.wrap = true;
        s = show(jp::calc::Value::text(text), wrapped, 100);
        QCOMPARE(s.text.size(), 1024);
    }

    // Text spills into the empty cells beside it as far as it needs, and stops at one with something in it; numbers, wrapped, shrunk, and turned text do not.
    void textSpillsIntoEmptyNeighborsOnTheSideItAlignsTo()
    {
        const jp::calc::Value long200 = jp::calc::Value::text(QString(200 / 7 + 1, QLatin1Char('x')));   // 29 characters: 203
        CellLook left;
        const Shown ls = show(long200, left, 64);
        QCOMPARE(ls.width, 203.0);
        auto open = [](int) { return 64.0; };
        // the text and its sides need 209, the cell has 64: 145 more, from three columns of 64
        Spill sp = spillOf(ls, left, 64, open);
        QCOMPARE(sp.right, 3);
        QCOMPARE(sp.left, 0);
        QCOMPARE(sp.extra, 192.0);
        // stopped at the second neighbor
        sp = spillOf(ls, left, 64, [](int step) { return step == 1 ? 64.0 : -1.0; });
        QCOMPARE(sp.right, 1);
        QCOMPARE(sp.extra, 64.0);
        // stopped at once
        sp = spillOf(ls, left, 64, [](int) { return -1.0; });
        QCOMPARE(sp.right + sp.left, 0);
        // right-aligned goes left
        CellLook right;
        right.horizontal = HAlign::Right;
        const Shown rs = show(long200, right, 64);
        sp = spillOf(rs, right, 64, open);
        QCOMPARE(sp.left, 3);
        QCOMPARE(sp.right, 0);
        // centered goes both ways, half the need on each side (72.5: two columns)
        CellLook center;
        center.horizontal = HAlign::Center;
        const Shown cs = show(long200, center, 64);
        sp = spillOf(cs, center, 64, open);
        QCOMPARE(sp.left, 2);
        QCOMPARE(sp.right, 2);
        // hidden columns are stepped over, and take no room
        sp = spillOf(ls, left, 64, [](int step) { return step == 1 || step == 2 ? 0.0 : 64.0; });
        QCOMPARE(sp.right, 5);
        QCOMPARE(sp.extra, 192.0);
        // text that fits spills nowhere
        const Shown fits = show(jp::calc::Value::text(QStringLiteral("abc")), left, 64);
        sp = spillOf(fits, left, 64, open);
        QCOMPARE(sp.left + sp.right, 0);
        // numbers, true, false, errors; wrapped, shrunk, and turned text
        const Shown wide = show(jp::calc::Value::number(1234567890123.0), lookWithFormat(QStringLiteral("0")), 64);
        QCOMPARE(spillOf(wide, left, 64, open).right, 0);
        CellLook wrap = left;
        wrap.wrap = true;
        QCOMPARE(spillOf(ls, wrap, 64, open).right, 0);
        CellLook shrink = left;
        shrink.shrink = true;
        QCOMPARE(spillOf(ls, shrink, 64, open).right, 0);
        CellLook turned = left;
        turned.rotation = 45;
        QCOMPARE(spillOf(ls, turned, 64, open).right, 0);
        // the most is kMaxSpill columns, whatever the text and the room
        const Shown huge = show(jp::calc::Value::text(QString(1000, QLatin1Char('y'))), left, 64);
        sp = spillOf(huge, left, 64, [](int) { return 4.0; });
        QCOMPARE(sp.right, kMaxSpill);
    }

    // The geometry the file gives: widths and heights in pixels (the default from the sheet's own), hidden ones, the first rows and columns held, merges, zoom, and where it was scrolled.
    void aSheetsFileGivesTheGridItsGeometry()
    {
        Sheet sheet;
        sheet.format.defaultRowHeight = 15;
        sheet.format.baseColWidth = 8;
        ColumnSpan c1;
        c1.first = 0;
        c1.last = 1;
        c1.width = 12.7109375;
        ColumnSpan c2;
        c2.first = 3;
        c2.last = 3;
        c2.hidden = true;
        c2.width = 9;
        ColumnSpan c3;
        c3.first = 5;
        c3.last = 6;
        c3.width = 20;
        ColumnSpan overlap;   // (a run that overlaps the one before is trimmed to what is left)
        overlap.first = 6;
        overlap.last = 7;
        overlap.width = 30;
        sheet.columns = {c1, c2, c3, overlap};
        RowSpec tall;
        tall.row = 1;
        tall.height = 30;
        RowSpec hidden;
        hidden.row = 2;
        hidden.hidden = true;
        RowSpec styled;
        styled.row = 4;
        styled.customFormat = true;
        styled.style = 3;
        sheet.rowSpecs = {hidden, tall, styled};   // (not in order)
        CellRange m;
        m.row1 = 0;
        m.col1 = 0;
        m.row2 = 1;
        m.col2 = 1;
        m.valid = true;
        sheet.merges = {m};
        sheet.view.frozen = true;
        sheet.view.frozenRows = 1;
        sheet.view.frozenCols = 2;
        sheet.view.paneRow = 11;
        sheet.view.paneCol = 4;
        sheet.view.zoom = 150;
        Styles styles;
        FontSpec f;
        f.name = QStringLiteral("Calibri");
        f.size = 11;
        styles.fonts = {f};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
        SharedStrings strings;
        GridData d;
        d.styles = &styles;
        d.strings = &strings;
        d.sheet = &sheet;
        GridSheet g;
        g.setMeter(&kMeter);
        g.setData(d);
        const GridLayout &l = g.layout();
        QCOMPARE(l.cols.size(0), 89);
        QCOMPARE(l.cols.size(1), 89);
        QCOMPARE(l.cols.size(2), 64);   // the sheet's own default (8 digits of 7 and 5, to a multiple of 8)
        QCOMPARE(l.cols.size(3), 0);
        QCOMPARE(l.cols.size(5), GridSheet::columnPixels(20, 7));
        QCOMPARE(l.cols.size(6), GridSheet::columnPixels(20, 7));
        QCOMPARE(l.cols.size(7), GridSheet::columnPixels(30, 7));
        QCOMPARE(g.columnWidth(0), 89.0);
        QCOMPARE(g.columnWidth(3), 0.0);
        QCOMPARE(l.rows.size(0), 20);
        QCOMPARE(l.rows.size(1), 40);
        QVERIFY(l.rows.isHidden(2));
        QCOMPARE(l.rows.size(4), 20);
        QCOMPARE(l.merges.count(), 1);
        QCOMPARE(l.frozenRows(), qint64(1));
        QCOMPARE(l.frozenCols(), qint64(2));
        QCOMPARE(l.zoom(), 1.5);
        // the part that scrolls begins at the cell the file says
        QCOMPARE(l.scrollY(), l.rows.position(11) - l.rows.position(1));
        QCOMPARE(l.scrollX(), l.cols.position(4) - l.cols.position(2));
        QCOMPARE(l.cols.count(), qint64(16384));
        QCOMPARE(l.rows.count(), qint64(1048576));
    }

    // A sheet that hides the rows it does not list shows those it lists; a column run that is hostile (a hundred thousand runs on top of one another) costs a sort.
    void rowsTheFileDoesNotListAreHiddenWhenItSaysSoAndColumnRunsAreTrimmed()
    {
        Sheet sheet;
        sheet.format.zeroHeight = true;
        for (quint32 r : {0u, 1u, 4u}) {
            RowSpec s;
            s.row = r;
            sheet.rowSpecs.append(s);
        }
        sheet.view = SheetView();
        for (int i = 0; i < 100000; ++i) {
            ColumnSpan c;
            c.first = 0;
            c.last = quint32(i % 16384);
            c.width = 5 + i % 50;
            sheet.columns.append(c);
        }
        Styles styles;
        styles.fonts = {FontSpec()};
        styles.formats = {CellFormat()};
        GridData d;
        d.styles = &styles;
        d.sheet = &sheet;
        GridSheet g;
        g.setData(d);
        const GridLayout &l = g.layout();
        QVERIFY(!l.rows.isHidden(0) && !l.rows.isHidden(1) && !l.rows.isHidden(4));
        QVERIFY(l.rows.isHidden(2) && l.rows.isHidden(3));
        QCOMPARE(l.rows.lastVisible(), qint64(4));   // (the sheet ends at its last listed row)
        QCOMPARE(l.cols.total() > 0, true);
    }

    // A place with no cell takes the format of its row when the row has one, else its column's; a cell has its own.
    void aPlaceWithNoCellHasItsRowsOrItsColumnsFormat()
    {
        Sheet sheet;
        ColumnSpan c;
        c.first = 2;
        c.last = 3;
        c.style = 5;
        ColumnSpan d;
        d.first = 6;
        d.last = 6;
        d.style = 7;
        sheet.columns = {c, d};
        RowSpec r;
        r.row = 4;
        r.customFormat = true;
        r.style = 9;
        RowSpec nof;   // a row that has a style but does not ask for it
        nof.row = 8;
        nof.style = 11;
        sheet.rowSpecs = {r, nof};
        Styles styles;
        styles.fonts = {FontSpec()};
        styles.formats = {CellFormat()};
        GridData data;
        data.styles = &styles;
        data.sheet = &sheet;
        GridSheet g;
        g.setData(data);
        QCOMPARE(g.styleOf(0, 2, nullptr), 5u);
        QCOMPARE(g.styleOf(0, 3, nullptr), 5u);
        QCOMPARE(g.styleOf(0, 4, nullptr), 0u);
        QCOMPARE(g.styleOf(0, 6, nullptr), 7u);
        QCOMPARE(g.styleOf(4, 2, nullptr), 9u);   // the row's format wins over the column's
        QCOMPARE(g.styleOf(4, 0, nullptr), 9u);
        QCOMPARE(g.styleOf(8, 0, nullptr), 0u);
        const Cell own = numberCell(4, 2, 1, 3);
        QCOMPARE(g.styleOf(4, 2, &own), 3u);
    }

    // The reach of the cells, from the sheet.
    void theUsedExtentIsTheSheets()
    {
        Sheet sheet;
        sheet.extentRows = 20;
        sheet.extentCols = 5;
        Styles styles;
        styles.fonts = {FontSpec()};
        styles.formats = {CellFormat()};
        GridData data;
        data.styles = &styles;
        data.sheet = &sheet;
        GridSheet g;
        g.setData(data);
        quint32 rows = 0, cols = 0;
        g.usedExtent(&rows, &cols);
        QCOMPARE(rows, 20u);
        QCOMPARE(cols, 5u);
    }

    // A cell's text is made once and kept; a change of values or formats makes the texts again, and the cache holds only as many as a screen shows.
    void whatCellsSayIsMadeOnceAndKept()
    {
        Sheet sheet;
        for (quint32 r = 0; r < 50; ++r)
            for (quint32 c = 0; c < 20; ++c) sheet.cells.add(numberCell(r, c, r * 100.0 + c));
        sheet.cells.finish();
        Styles styles;
        styles.fonts = {FontSpec()};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
        GridData data;
        data.styles = &styles;
        data.sheet = &sheet;
        GridSheet g;
        g.setMeter(&kMeter);
        g.setData(data);
        const Cell *c = g.cellAt(3, 4);
        QVERIFY(c);
        QCOMPARE(g.shown(3, 4, *c).text, QStringLiteral("304"));
        QCOMPARE(g.textsMade(), qint64(1));
        QCOMPARE(g.shown(3, 4, *c).text, QStringLiteral("304"));
        QCOMPARE(g.textsMade(), qint64(1));
        for (quint32 r = 0; r < 50; ++r)
            for (quint32 col = 0; col < 20; ++col) g.shown(r, col, *g.cellAt(r, col));
        QCOMPARE(g.textsMade(), qint64(1000));
        QCOMPARE(g.textsHeld(), size_t(1000));
        // all of it asked for again costs nothing
        for (quint32 r = 0; r < 50; ++r)
            for (quint32 col = 0; col < 20; ++col) g.shown(r, col, *g.cellAt(r, col));
        QCOMPARE(g.textsMade(), qint64(1000));
        g.invalidateTexts();
        QCOMPARE(g.textsHeld(), size_t(0));
        g.shown(3, 4, *c);
        QCOMPARE(g.textsMade(), qint64(1001));
        QVERIFY(!g.cellAt(60, 4));
    }

    // The cache is bounded: a hundred thousand cells shown one after another leave no more than it holds.
    void theTextCacheIsBounded()
    {
        Sheet sheet;
        for (quint32 r = 0; r < 100000; ++r) sheet.cells.add(numberCell(r, 0, r));
        sheet.cells.finish();
        Styles styles;
        styles.fonts = {FontSpec()};
        styles.formats = {CellFormat()};
        GridData data;
        data.styles = &styles;
        data.sheet = &sheet;
        GridSheet g;
        g.setMeter(&kMeter);
        g.setData(data);
        for (quint32 r = 0; r < 100000; ++r) g.shown(r, 0, *g.cellAt(r, 0));
        QVERIFY(g.textsHeld() <= GridSheet::kMaxTexts);
        QCOMPARE(g.textsMade(), qint64(100000));
    }

    // A cell shows what the calculation says; until a formula is worked out it shows the result the file stored.
    void valuesComeFromTheCalculationOnceThereIsOne()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c><c r=\"B1\"><f>A1*2</f><v>99</v></c></row>");
        Package package;
        Workbook book;
        QString why;
        QVERIFY(package.readLazy(buildXlsx({worksheet(rows)}), &why) && book.open(package, &why));
        jp::office::sheet::Calculation calc(book);
        QVERIFY(calc.load());
        GridData d;
        d.styles = &book.styles();
        d.strings = &book.sharedStrings();
        d.sheet = &calc.sheet(0);
        d.calc = &calc;
        GridSheet g;
        g.setMeter(&kMeter);
        g.setData(d);
        const Cell *b1 = g.cellAt(0, 1);
        QVERIFY(b1);
        QCOMPARE(g.valueOf(*b1).debug(), QStringLiteral("n:99"));   // (not worked out yet: what the file stored)
        QVERIFY(calc.recalculate().finished);
        g.invalidateTexts();
        QCOMPARE(g.valueOf(*b1).debug(), QStringLiteral("n:10"));
        QCOMPARE(g.shown(0, 1, *b1).text, QStringLiteral("10"));
        // without a calculation, what the file stored
        GridData stored = d;
        stored.calc = nullptr;
        stored.sheet = &calc.sheet(0);
        GridSheet h;
        h.setData(stored);
        QCOMPARE(h.valueOf(*b1).debug(), QStringLiteral("n:99"));
    }
};

JP_TEST_CLASS(GridCellTests)
#include "tst_grid_cells.moc"
