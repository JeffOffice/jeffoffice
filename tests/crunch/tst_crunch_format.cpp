// Tests of how cells look and how they are formatted: numbers in the person's language, conditional formats, the formatting commands and the Format Cells dialog, the sheets of a
// workbook, and the default sizes. Every workbook is built here, and every locale is set explicitly.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// Numbers in four formats:
//         A (General)   B (#,##0.00)   C (0.00%)    D (d.m.yyyy)    E (0.0E+00)
//   1     1.5           1234.5         0.1234       45000           12345
//   2     1234.5        -1234567.891   2.5          45001.5         0.000123
//   3     -0.25         0.5            -0.01        0               1e15
QByteArray numbersBook()
{
    const QByteArray styles = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"#,##0.00\"/><numFmt numFmtId=\"165\" formatCode=\"d.m.yyyy\"/><numFmt numFmtId=\"166\" formatCode=\"0.0E+00\"/>"),
                                        QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\" applyNumberFormat=\"1\"/><xf numFmtId=\"10\" applyNumberFormat=\"1\"/><xf numFmtId=\"165\" applyNumberFormat=\"1\"/><xf numFmtId=\"166\" applyNumberFormat=\"1\"/>"));
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1.5</v></c><c r=\"B1\" s=\"1\"><v>1234.5</v></c><c r=\"C1\" s=\"2\"><v>0.1234</v></c><c r=\"D1\" s=\"3\"><v>45000</v></c><c r=\"E1\" s=\"4\"><v>12345</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>1234.5</v></c><c r=\"B2\" s=\"1\"><v>-1234567.891</v></c><c r=\"C2\" s=\"2\"><v>2.5</v></c><c r=\"D2\" s=\"3\"><v>45001.5</v></c><c r=\"E2\" s=\"4\"><v>0.000123</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\"><v>-0.25</v></c><c r=\"B3\" s=\"1\"><v>0.5</v></c><c r=\"C3\" s=\"2\"><v>-0.01</v></c><c r=\"D3\" s=\"3\"><v>0</v></c><c r=\"E3\" s=\"4\"><v>1e15</v></c></row>");
    return buildXlsx({worksheet(rows, QString(), QStringLiteral("<cols><col min=\"1\" max=\"5\" width=\"16\" customWidth=\"1\"/></cols>"))}, QByteArray(), QString(), QString(), styles);
}

// A workbook with the lists a real file has (a font, two fills, a border), and some cells:
//         A      B      C
//   1     1      2      3
//   2     4      5      6
//   3     7      8      9
QByteArray formatBook()
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
    QString rows;
    for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%2</v></c><c r=\"B%1\"><v>%3</v></c><c r=\"C%1\"><v>%4</v></c></row>").arg(r).arg(r * 3 - 2).arg(r * 3 - 1).arg(r * 3);
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles);
}

// A workbook whose sheet has rules of conditional formatting:
//         A (color scale)   B (over 3, a fill)   C (data bar)   D (icon set)
//   1     10                1                    10             10
//   2     20                2                    20             20
//   3     30                3                    30             30
//   4     40                4                    40             40
//   5     50                5                    50             50
QByteArray ruledBook()
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"),
                                        QStringLiteral("<dxfs count=\"1\"><dxf><font><b/><color rgb=\"FF9C0006\"/></font><fill><patternFill><bgColor rgb=\"FFFFC7CE\"/></patternFill></fill></dxf></dxfs>"));
    QString rows;
    for (int r = 1; r <= 5; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%2</v></c><c r=\"B%1\"><v>%1</v></c><c r=\"C%1\"><v>%2</v></c><c r=\"D%1\"><v>%2</v></c></row>").arg(r).arg(r * 10);
    const QString after = QStringLiteral(
        "<conditionalFormatting sqref=\"A1:A5\"><cfRule type=\"colorScale\" priority=\"1\"><colorScale><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FFFF0000\"/><color rgb=\"FF0000FF\"/></colorScale></cfRule></conditionalFormatting>"
        "<conditionalFormatting sqref=\"B1:B5\"><cfRule type=\"cellIs\" dxfId=\"0\" priority=\"2\" operator=\"greaterThan\"><formula>3</formula></cfRule></conditionalFormatting>"
        "<conditionalFormatting sqref=\"C1:C5\"><cfRule type=\"dataBar\" priority=\"3\"><dataBar><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FF638EC6\"/></dataBar></cfRule></conditionalFormatting>"
        "<conditionalFormatting sqref=\"D1:D5\"><cfRule type=\"iconSet\" priority=\"4\"><iconSet iconSet=\"3TrafficLights1\"><cfvo type=\"percent\" val=\"0\"/><cfvo type=\"percent\" val=\"33\"/><cfvo type=\"percent\" val=\"67\"/></iconSet></cfRule></conditionalFormatting>");
    return buildXlsx({worksheet(rows, after)}, QByteArray(), QString(), QString(), styles);
}

CellRange columns(quint32 first, quint32 last) { return CellRange{0, first, 1048575, last, true}; }
CellRange rows(quint32 first, quint32 last) { return CellRange{first, 0, last, 16383, true}; }

// The format of a cell of the grid, as the grid draws it.
const grid::CellLook &lookAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::Cell *cell = e.view()->sheet().cellAt(r, c);
    return e.view()->sheet().looks().look(e.view()->sheet().styleOf(r, c, cell));
}

quint32 styleAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.view()->sheet().styleOf(r, c, e.view()->sheet().cellAt(r, c));
}

} // namespace

class CrunchFormatTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // Numbers are shown with the decimal mark and the digit grouping of the person's language, in General and in the number formats (which stay as the file has them); a date is by its code.
    // The formula bar and F2 give the number the way it is typed in that language.
    void numbersAreShownInTheLanguageOfTheComputer()
    {
        struct Case {
            QLocale locale;
            QString decimal;   // what the locale writes (the group is the locale's own)
        };
        const QVector<Case> cases = {
            {QLocale(QLocale::English, QLocale::UnitedStates), QStringLiteral(".")},
            {QLocale(QLocale::German, QLocale::Germany), QStringLiteral(",")},
            {QLocale(QLocale::French, QLocale::France), QStringLiteral(",")},
            {QLocale(QLocale::German, QLocale::Switzerland), QStringLiteral(".")},
            {QLocale(QLocale::Japanese, QLocale::Japan), QStringLiteral(".")},
        };
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        for (const Case &c : cases) {
            const QString name = c.locale.name();
            e.win.w.setLocale(c.locale);
            const QString d = c.decimal, g = c.locale.groupSeparator();
            QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("1") + d + QStringLiteral("5"));
            QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("1234") + d + QStringLiteral("5"));
            QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("-0") + d + QStringLiteral("25"));
            QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("1") + g + QStringLiteral("234") + d + QStringLiteral("50"));
            QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("-1") + g + QStringLiteral("234") + g + QStringLiteral("567") + d + QStringLiteral("89"));
            QCOMPARE(e.shownAt(QStringLiteral("B3")), QStringLiteral("0") + d + QStringLiteral("50"));
            QCOMPARE(e.shownAt(QStringLiteral("C1")), QStringLiteral("12") + d + QStringLiteral("34%"));
            QCOMPARE(e.shownAt(QStringLiteral("D1")), QStringLiteral("15.3.2023"));           // (the code, whatever the language)
            QCOMPARE(e.shownAt(QStringLiteral("E1")), QStringLiteral("1") + d + QStringLiteral("2E+04"));
            QCOMPARE(e.shownAt(QStringLiteral("E2")), QStringLiteral("1") + d + QStringLiteral("2E-04"));
            // F2 and the formula bar: as the language types a number
            QCOMPARE(e.win.w.session()->editText(0, 0, 0), QStringLiteral("1") + d + QStringLiteral("5"));
            QCOMPARE(e.win.w.session()->editText(0, 1, 1), QString(QStringLiteral("-1234567") + d + QStringLiteral("891")));
            e.select(QStringLiteral("A1"));
            QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("1") + d + QStringLiteral("5"));
            e.select(QStringLiteral("B2"));
            QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("-1234567") + d + QStringLiteral("891"));
            // what is typed in the language comes back as the same number
            e.select(QStringLiteral("A3"));
            e.enter(QStringLiteral("2") + d + QStringLiteral("75"));
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2.75"));
            QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("2") + d + QStringLiteral("75"));
            e.win.w.undoStack()->undo();
            QVERIFY(e.settle());
            QVERIFY2(e.shownAt(QStringLiteral("A3")) == QStringLiteral("-0") + d + QStringLiteral("25"), qPrintable(name));
        }
        // a copy gives the text as shown
        e.win.w.setLocale(QLocale(QLocale::German, QLocale::Germany));
        QCOMPARE(e.view()->sheet().textOf(*e.view()->sheet().cellAt(1, 1)), QStringLiteral("-1.234.567,89"));
    }

    // A formatting command is one step: the cells have the font and fill, the format is the same for the same change (never made twice), Undo and Redo give it and take it back, and the
    // saved file has the font, the fill, and the cell format, once.
    void formattingIsOneStepAndSavedOnce()
    {
        EditWindow e;
        QVERIFY(e.open(formatBook()));
        crunch::EditSession *s = e.win.w.session();
        const CellRange block = rangeOf(QStringLiteral("B2:C3"));
        FormatChange bold;
        bold.bold = true;
        bold.fontSize = 14;
        bold.fontName = QStringLiteral("Georgia");
        FillSpec yellow;
        yellow.pattern = PatternKind::Solid;
        yellow.fg = ColorSpec::fromRgb(0xFFFF00);
        bold.fill = yellow;
        QVERIFY(s->formatCells(0, block, bold, QStringLiteral("Formatting")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Formatting"));
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold && lookAt(e, QStringLiteral("C3")).bold);
        QVERIFY(!lookAt(e, QStringLiteral("A1")).bold && !lookAt(e, QStringLiteral("B1")).bold);
        QCOMPARE(lookAt(e, QStringLiteral("B2")).family, QStringLiteral("Georgia"));
        QCOMPARE(lookAt(e, QStringLiteral("B2")).pointSize, 14.0);
        QCOMPARE(lookAt(e, QStringLiteral("B2")).fg, QRgb(0xFFFFFF00));
        QVERIFY(lookAt(e, QStringLiteral("B2")).pattern == PatternKind::Solid);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:5"));            // (the values are as they were)
        const quint32 style = styleAt(e, QStringLiteral("B2"));
        QCOMPARE(styleAt(e, QStringLiteral("C3")), style);                            // (one cell format for the cells)
        // the same change on other cells is the same cell format
        QVERIFY(s->formatCells(0, rangeOf(QStringLiteral("A1")), bold, QStringLiteral("Formatting")).ok);
        QCOMPARE(styleAt(e, QStringLiteral("A1")), style);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(!lookAt(e, QStringLiteral("A1")).bold);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(!lookAt(e, QStringLiteral("B2")).bold);
        QCOMPARE(styleAt(e, QStringLiteral("B2")), 0u);
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold);
        // a change that changes nothing is no step
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(s->formatCells(0, block, bold, QStringLiteral("Formatting")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // saved, and read again
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|') + e.win.w.statusText()));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        const grid::Cell *cell = g.cellAt(1, 1);
        QVERIFY(cell);
        const grid::CellLook &look = g.looks().look(cell->style);
        QVERIFY(look.bold);
        QCOMPARE(look.family, QStringLiteral("Georgia"));
        QCOMPARE(look.pointSize, 14.0);
        QCOMPARE(look.fg, QRgb(0xFFFFFF00));
        QCOMPARE(second.w.calculation()->sheet(0).cells.find(2, 2)->style, cell->style);
        QCOMPARE(second.w.calculation()->sheet(0).cells.find(0, 0)->style, 0u);
        QCOMPARE(second.w.calculation()->sheet(0).cells.find(1, 1)->style, second.w.calculation()->sheet(0).cells.find(2, 1)->style);
        // a second save adds nothing the file has: the same change gives the same cell format
        FormatChange again;
        again.bold = true;
        again.fontSize = 14;
        again.fontName = QStringLiteral("Georgia");
        again.fill = yellow;
        QVERIFY(second.w.session()->formatCells(0, rangeOf(QStringLiteral("A3")), again, QStringLiteral("Formatting")).ok);
        QCOMPARE(second.w.calculation()->sheet(0).cells.find(2, 0)->style, cell->style);
    }

    // Whole columns and rows are formatted as the columns' and rows' own (empty cells there have it, and a cell typed in later takes it), the cells in them that have formats of their own are
    // changed too, it is saved as the column's and row's format, and Clear Formats takes it away.
    void wholeColumnsAndRowsHaveTheFormatTheirOwn()
    {
        EditWindow e;
        QVERIFY(e.open(formatBook()));
        crunch::EditSession *s = e.win.w.session();
        FormatChange italic;
        italic.italic = true;
        FillSpec green;
        green.pattern = PatternKind::Solid;
        green.fg = ColorSpec::fromRgb(0x00FF00);
        italic.fill = green;
        QVERIFY(s->formatCells(0, columns(1, 1), italic, QStringLiteral("Formatting")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(lookAt(e, QStringLiteral("B2")).italic);                 // (a cell the column has)
        QVERIFY(lookAt(e, QStringLiteral("B500")).italic);               // (and one it does not)
        QVERIFY(!lookAt(e, QStringLiteral("A500")).italic && !lookAt(e, QStringLiteral("C500")).italic);
        QCOMPARE(lookAt(e, QStringLiteral("B500")).fg, QRgb(0xFF00FF00));
        // a row over it: the row's format is the empty cells' (it is over the column's)
        FormatChange bold;
        bold.bold = true;
        QVERIFY(s->formatCells(0, rows(4, 5), bold, QStringLiteral("Formatting")).ok);
        QVERIFY(lookAt(e, QStringLiteral("F5")).bold && lookAt(e, QStringLiteral("F6")).bold && !lookAt(e, QStringLiteral("F7")).bold);
        QVERIFY(lookAt(e, QStringLiteral("A1")).bold == false);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|') + e.win.w.statusText()));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        QVERIFY(g.looks().look(g.styleOf(499, 1, nullptr)).italic);
        QVERIFY(g.looks().look(g.styleOf(4, 7, nullptr)).bold);
        QVERIFY(g.looks().look(g.styleOf(1, 1, g.cellAt(1, 1))).italic);
        QVERIFY(!g.looks().look(g.styleOf(499, 2, nullptr)).italic);
        // Clear Formats takes them away, with the formats of the cells
        // a number typed in a cell of the column takes the column's format
        e.select(QStringLiteral("B600"));
        e.enter(QStringLiteral("7"));
        QVERIFY(e.settle());
        QVERIFY(lookAt(e, QStringLiteral("B600")).italic);
        QCOMPARE(lookAt(e, QStringLiteral("B600")).fg, QRgb(0xFF00FF00));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(s->clearFormats(0, columns(1, 1)).ok);
        QVERIFY(!lookAt(e, QStringLiteral("B2")).italic && !lookAt(e, QStringLiteral("B500")).italic);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(lookAt(e, QStringLiteral("B2")).italic && lookAt(e, QStringLiteral("B500")).italic);
        QVERIFY(s->clearFormats(0, rangeOf(QStringLiteral("A1:C3"))).ok);
        QVERIFY(!lookAt(e, QStringLiteral("B2")).italic);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:5"));   // (the values stay)
        QVERIFY(s->formatCells(0, CellRange{0, 0, 1048575, 16383, true}, italic, QStringLiteral("Formatting")).ok);   // (the whole sheet is the columns')
    }

    // The borders of a block: the outside edges and the lines between its cells, the cells beside it that share an edge have the line on their side, and a line of no style takes them away.
    void bordersAreDrawnOnTheEdgesOfABlock()
    {
        EditWindow e;
        QVERIFY(e.open(formatBook()));
        crunch::EditSession *s = e.win.w.session();
        BorderSide thin;
        thin.style = BorderStyle::Thin;
        thin.color = ColorSpec::fromRgb(0x336699);
        using E = crunch::EditSession;
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("B2:C3")), E::EdgeLeft | E::EdgeRight | E::EdgeTop | E::EdgeBottom, thin, QStringLiteral("Borders")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(lookAt(e, QStringLiteral("B2")).left.style == BorderStyle::Thin && lookAt(e, QStringLiteral("B2")).top.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("B2")).right.style == BorderStyle::None && lookAt(e, QStringLiteral("B2")).bottom.style == BorderStyle::None);   // (inside)
        QVERIFY(lookAt(e, QStringLiteral("C3")).right.style == BorderStyle::Thin && lookAt(e, QStringLiteral("C3")).bottom.style == BorderStyle::Thin);
        QCOMPARE(lookAt(e, QStringLiteral("B2")).left.color, QRgb(0xFF336699));
        // the cells beside it
        QVERIFY(lookAt(e, QStringLiteral("A2")).right.style == BorderStyle::Thin && lookAt(e, QStringLiteral("A3")).right.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("B1")).bottom.style == BorderStyle::Thin && lookAt(e, QStringLiteral("C1")).bottom.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("D2")).left.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("B4")).top.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("A1")).right.style == BorderStyle::None);
        // the lines between
        BorderSide dashed;
        dashed.style = BorderStyle::Dashed;
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("B2:C3")), E::EdgeInsideHorizontal | E::EdgeInsideVertical, dashed, QStringLiteral("Borders")).ok);
        QVERIFY(lookAt(e, QStringLiteral("B2")).right.style == BorderStyle::Dashed && lookAt(e, QStringLiteral("B2")).bottom.style == BorderStyle::Dashed);
        QVERIFY(lookAt(e, QStringLiteral("C3")).left.style == BorderStyle::Dashed && lookAt(e, QStringLiteral("C3")).top.style == BorderStyle::Dashed);
        QVERIFY(lookAt(e, QStringLiteral("B2")).left.style == BorderStyle::Thin);   // (the outside stays)
        // none takes them away
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("B2:C3")), E::EdgeLeft | E::EdgeRight | E::EdgeTop | E::EdgeBottom | E::EdgeInsideHorizontal | E::EdgeInsideVertical, BorderSide(), QStringLiteral("Borders")).ok);
        for (const char *ref : {"B2", "C3", "A2", "B1", "D2", "B4"}) {
            const grid::CellLook &l = lookAt(e, QString::fromLatin1(ref));
            QVERIFY2(!l.hasBorder(), ref);
        }
        // a diagonal
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("A1")), E::EdgeDiagonalUp, thin, QStringLiteral("Borders")).ok);
        QVERIFY(lookAt(e, QStringLiteral("A1")).diagonalUp && lookAt(e, QStringLiteral("A1")).diagonal.style == BorderStyle::Thin);
        // saved
        QVERIFY(s->setBorders(0, rangeOf(QStringLiteral("B2:C3")), E::EdgeBottom, thin, QStringLiteral("Borders")).ok);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|') + e.win.w.statusText()));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        QVERIFY(g.looks().look(g.cellAt(2, 1)->style).bottom.style == BorderStyle::Thin);
        QVERIFY(g.looks().look(g.cellAt(0, 0)->style).diagonalUp);
    }

    // Cells are merged (the upper left keeps its value; the others are asked about when they hold something, and are emptied), merged and centered, merged across, and unmerged; one step
    // each, undone, and saved.
    void cellsAreMergedAndUnmerged()
    {
        EditWindow e;
        QVERIFY(e.open(formatBook()));
        crunch::EditSession *s = e.win.w.session();
        using K = crunch::EditSession::MergeKind;
        // the others hold numbers: asked first, and nothing changed
        crunch::EditSession::Outcome o = s->mergeCells(0, rangeOf(QStringLiteral("A1:B2")), K::Merge);
        QVERIFY(!o.ok && o.confirm);
        QVERIFY(o.problem.contains(QStringLiteral("upper-left")));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
        o = s->mergeCells(0, rangeOf(QStringLiteral("A1:B2")), K::Merge, true);
        QVERIFY(o.ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 1);
        QVERIFY(e.view()->sheet().layout().merges.at(0, 1) >= 0 && e.view()->sheet().layout().merges.at(1, 0) >= 0);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("-"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:5"));
        // merge and center
        QVERIFY(s->mergeCells(0, rangeOf(QStringLiteral("B3:C3")), K::MergeAndCenter, true).ok);
        QVERIFY(lookAt(e, QStringLiteral("B3")).horizontal == HAlign::Center);
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("-"));
        // across: each row of the block
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(s->mergeCells(0, rangeOf(QStringLiteral("A1:C3")), K::Across, true).ok);
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 3);
        QVERIFY(e.valueAt(QStringLiteral("A2")) == QStringLiteral("n:4") && e.valueAt(QStringLiteral("B2")) == QStringLiteral("-"));
        // unmerge: the merges the block touches go; the values stay empty
        QVERIFY(s->mergeCells(0, rangeOf(QStringLiteral("A2")), K::Unmerge).ok);
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 2);
        // saved: the merged ranges are in the file
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|') + e.win.w.statusText()));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.calculation()->mergesOf(0), e.win.w.calculation()->mergesOf(0));
        // a merge over a merge replaces it
        o = s->mergeCells(0, rangeOf(QStringLiteral("A1:A3")), K::Merge, true);
        QVERIFY(o.ok);
        for (const CellRange &m : e.win.w.calculation()->mergesOf(0)) QVERIFY(m.row1 != m.row2 ? (m.col1 == 0 && m.col2 == 0) : m.col1 > 0);
    }

    // Conditional formats are shown from the file: the color scale, the fill of a rule, the bar, and the icon; they are worked out again when a value changes (the lowest and highest move) and
    // come back with Undo; rows put in above the cells move the rules with them; and they are in the file saved after all that.
    void conditionalFormatsAreShownFollowTheEditsAndAreSaved()
    {
        EditWindow e;
        QVERIFY(e.open(ruledBook()));
        grid::GridSheet &g = e.view()->sheet();
        auto at = [&](const QString &ref) -> const grid::CondCell & {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            return g.conditional().at(r, c);
        };
        auto look = [&](const QString &ref) -> const grid::CellLook & {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            return g.lookOf(r, c, g.cellAt(r, c) ? g.cellAt(r, c)->style : 0);
        };
        QVERIFY(g.hasConditionalFormats());
        QCOMPARE(at(QStringLiteral("A1")).scale, QRgb(0xFFFF0000));
        QCOMPARE(at(QStringLiteral("A5")).scale, QRgb(0xFF0000FF));
        const QRgb quarter = at(QStringLiteral("A2")).scale;   // 20 is a quarter of the way from 10 to 50
        QVERIFY(!look(QStringLiteral("B3")).hasFill());
        QVERIFY(look(QStringLiteral("B4")).hasFill() && look(QStringLiteral("B4")).fg == QRgb(0xFFFFC7CE) && look(QStringLiteral("B4")).bold);
        QCOMPARE(look(QStringLiteral("B4")).textColor, QRgb(0xFF9C0006));
        QVERIFY(at(QStringLiteral("C3")).hasBar);
        QCOMPARE(at(QStringLiteral("D1")).icon, 0);
        QCOMPARE(at(QStringLiteral("D3")).icon, 1);
        QCOMPARE(at(QStringLiteral("D5")).icon, 2);
        // the highest value is typed over: the figures are worked out again, and what is shown is another
        e.select(QStringLiteral("A5"));
        e.enter(QStringLiteral("100"));
        QVERIFY(e.settle());
        QVERIFY(at(QStringLiteral("A2")).scale != quarter);
        QVERIFY(at(QStringLiteral("C5")).bar.to > 0.89);
        e.select(QStringLiteral("C5"));
        e.enter(QStringLiteral("1000"));
        QVERIFY(e.settle());
        QVERIFY(at(QStringLiteral("C4")).bar.to < 0.2);   // (40 against 10 to 1000)
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(at(QStringLiteral("C4")).bar.to > 0.7);
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(at(QStringLiteral("A2")).scale, quarter);
        // a row put in above: the rules cover the cells where they have gone (A2:A6), and the new row has none
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(e.settle());
        QVERIFY(!at(QStringLiteral("A1")).any);
        QCOMPARE(at(QStringLiteral("A2")).scale, QRgb(0xFFFF0000));
        QCOMPARE(at(QStringLiteral("A6")).scale, QRgb(0xFF0000FF));
        QVERIFY(look(QStringLiteral("B5")).hasFill() && !look(QStringLiteral("B4")).hasFill());
        QCOMPARE(g.conditional().iconSetName(g.conditional().at(5, 3).iconSet), QStringLiteral("3TrafficLights1"));
        // saved, and the file has the rules where they are
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|') + e.win.w.statusText()));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const QVector<CondRule> &rules = second.w.calculation()->sheet(0).conditionalFormats;
        QCOMPARE(rules.size(), 4);
        QCOMPARE(rangeName(rules[0].ranges[0]), QStringLiteral("A2:A6"));
        QCOMPARE(rangeName(rules[1].ranges[0]), QStringLiteral("B2:B6"));
        QVERIFY(second.w.view()->sheet().conditional().at(5, 0).scale == QRgb(0xFF0000FF));
    }

    // Default Width and Default Row Height: the columns and rows that have no size of their own take it (the others do not), it is one step each, Undo takes it back, and the file has it.
    void theDefaultSizesAreSetUndoneAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(formatBook()));
        const grid::GridLayout &L = e.view()->sheet().layout();
        const int widthBefore = L.cols.defaultSize(), heightBefore = L.rows.defaultSize();
        CrunchWindow::SizeRequest seen;
        int asked = 0;
        double answer = 20;
        e.win.w.setSizeAsker([&](CrunchWindow::SizeRequest *r) {
            seen = *r;
            ++asked;
            r->value = answer;
            return true;
        });
        // a column and a row of their own
        QVERIFY(e.win.w.session()->resizeColumns(0, 1, 1, [] {
            crunch::EditSession::SizeChange by;
            by.setSize = true;
            by.size = 30;
            return by;
        }(), QStringLiteral("Changing column width")).ok);
        const int ownWidth = L.cols.size(1);
        e.win.w.act(QStringLiteral("format.defaultWidth"))->trigger();
        QCOMPARE(asked, 1);
        QVERIFY(seen.column && seen.standard);
        QVERIFY(L.cols.defaultSize() > widthBefore);                  // (20 characters is wider than the 8 and a bit it was)
        QCOMPARE(L.cols.size(1), ownWidth);                            // (the column that has its own width keeps it)
        QCOMPARE(L.cols.size(5), L.cols.defaultSize());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Default Width"));
        answer = 30;
        e.win.w.act(QStringLiteral("format.defaultHeight"))->trigger();
        QCOMPARE(asked, 2);
        QVERIFY(!seen.column && seen.standard);
        QVERIFY(L.rows.defaultSize() > heightBefore);
        QCOMPARE(e.win.w.undoStack()->count(), 3);
        // asked again, the dialog starts from what the sheet has (in the units a person is shown)
        e.win.w.act(QStringLiteral("format.defaultHeight"))->trigger();
        QCOMPARE(seen.value, 30.0);
        QCOMPARE(e.win.w.undoStack()->count(), 3);   // (the same size is no step)
        // Undo and Redo
        e.win.w.undoStack()->undo();
        QCOMPARE(L.rows.defaultSize(), heightBefore);
        e.win.w.undoStack()->undo();
        QCOMPARE(L.cols.defaultSize(), widthBefore);
        e.win.w.undoStack()->redo();
        e.win.w.undoStack()->redo();
        QVERIFY(L.cols.defaultSize() > widthBefore && L.rows.defaultSize() > heightBefore);
        // saved, and the file's sheet says it
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|')));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const SheetFormat f = second.w.calculation()->sheetFormatOf(0);
        QCOMPARE(f.defaultRowHeight, 30.0);
        QVERIFY(f.customHeight);
        QVERIFY(f.defaultColWidth > 20);   // (the width in the file has the padding)
        QCOMPARE(second.w.view()->sheet().layout().cols.defaultSize(), L.cols.defaultSize());
        QCOMPARE(second.w.view()->sheet().layout().rows.defaultSize(), L.rows.defaultSize());
        // a size that is nothing is refused
        answer = 0;
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("format.defaultWidth"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.told.isEmpty());
    }

    // The sheet with its rules drawn (JC_SCREENSHOT_COND names a file to keep the picture in): the color scale, the cells over three, the bars, and the icons are on the screen.
    void conditionalFormatsAreOnTheScreen()
    {
        EditWindow e;
        QVERIFY(e.open(ruledBook()));
        e.win.w.resize(900, 420);
        e.view()->setZoom(1.5);
        QApplication::processEvents();
        const QImage shot = e.win.w.grab().toImage();
        QVERIFY(!shot.isNull());
        const QPoint origin = e.view()->viewport()->mapTo(&e.win.w, QPoint(0, 0));
        auto at = [&](int row, int col, double fx, double fy) {
            const QRect r = e.view()->cellRect(row, col);
            return QColor(shot.pixel(origin.x() + r.left() + int(r.width() * fx), origin.y() + r.top() + int(r.height() * fy)));
        };
        QCOMPARE(at(0, 0, 0.5, 0.2).rgb(), QRgb(0xFFFF0000));      // the lowest of the scale: red
        QCOMPARE(at(4, 0, 0.5, 0.2).rgb(), QRgb(0xFF0000FF));      // the highest: blue
        QCOMPARE(at(3, 1, 0.9, 0.15).rgb(), QRgb(0xFFFFC7CE));     // 4 is over 3: the fill of the format
        QVERIFY(at(2, 1, 0.9, 0.15).rgb() == QRgb(0xFFFFFFFF));     // 3 is not
        QVERIFY(at(4, 2, 0.5, 0.5).rgb() != QRgb(0xFFFFFFFF));      // the longest bar
        const QString out = qEnvironmentVariable("JC_SCREENSHOT_COND");
        if (!out.isEmpty()) QVERIFY(shot.save(out));
    }
};

JP_TEST_CLASS(CrunchFormatTests)
#include "tst_crunch_format.moc"
