// Tests of the formatting commands of JeffCrunch's window: the buttons of the Home tab (font, size, bold, italic, underlines, colors, borders, alignment, indent, orientation, merging, the
// number format and its buttons, cell styles, the Format Painter, Clear) and the Format Cells dialog's changes made to the cells. Each command is one step of Undo; every language is set
// explicitly; the workbook is built here.
#include "app/crunchwindow.h"
#include "savedok.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"

#include <QAction>
#include <QComboBox>
#include <QElapsedTimer>
#include <QMenu>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

//         A   B   C
//   1     1   2   3
//   2     4   5   6
//   3     7   8   9
// The locale of the typed values is pinned for the tests of them.
struct EnglishUs {
    QLocale was = QLocale();
    EnglishUs() { QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates)); }
    ~EnglishUs() { QLocale::setDefault(was); }
};

// The styles part of uiBook (a workbook with none has no place to keep the look of a rule).
QByteArray uiStyles(const QString &extra = QString())
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>") + extra;
    return stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
}

QByteArray uiBook()
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"), lists);
    QString rows;
    for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%2</v></c><c r=\"B%1\"><v>%3</v></c><c r=\"C%1\"><v>%4</v></c></row>").arg(r).arg(r * 3 - 2).arg(r * 3 - 1).arg(r * 3);
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles);
}

const grid::CellLook &lookAt(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::Cell *cell = e.view()->sheet().cellAt(r, c);
    return e.view()->sheet().looks().look(e.view()->sheet().styleOf(r, c, cell));
}

void selectRange(EditWindow &e, const QString &range)
{
    const CellRange r = rangeOf(range);
    e.view()->selectRange(r.row1, r.col1, r.row2, r.col2);
}

void run(EditWindow &e, const char *action)
{
    QAction *a = e.win.w.act(QString::fromLatin1(action));
    QVERIFY2(a, action);
    QVERIFY2(a->isEnabled(), action);
    a->trigger();
}

QMenu *menuNamed(EditWindow &e, const char *name)
{
    return e.win.w.findChild<QMenu *>(QString::fromLatin1(name));
}

QAction *actionTextOf(QMenu *menu, const QString &text)
{
    for (QAction *a : menu->actions())
        if (a->text() == text) return a;
    return nullptr;
}

} // namespace

class CrunchFormatUiTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // Bold, italic, the underlines, and strikethrough: each command is one step for the cells selected, the state of the cell with the cursor decides whether it puts the style on or
    // takes it off, the buttons show that state, and Undo takes it back.
    void theFontButtonsFormatTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("B2:C3"));
        QVERIFY(!e.win.w.act(QStringLiteral("fmt.bold"))->isChecked());
        run(e, "fmt.bold");
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold && lookAt(e, QStringLiteral("C3")).bold && lookAt(e, QStringLiteral("B3")).bold);
        QVERIFY(!lookAt(e, QStringLiteral("A1")).bold && !lookAt(e, QStringLiteral("B1")).bold);
        QVERIFY(e.win.w.act(QStringLiteral("fmt.bold"))->isChecked());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Bold"));
        run(e, "fmt.italic");
        run(e, "fmt.underline");
        QVERIFY(lookAt(e, QStringLiteral("B2")).italic && lookAt(e, QStringLiteral("B2")).underline == Underline::Single);
        run(e, "fmt.underline.double");
        QVERIFY(lookAt(e, QStringLiteral("C2")).underline == Underline::Double);
        run(e, "fmt.underline.double");
        QVERIFY(lookAt(e, QStringLiteral("C2")).underline == Underline::None);
        run(e, "fmt.strike");
        QVERIFY(lookAt(e, QStringLiteral("C3")).strike);
        // the cell with the cursor decides: select the bold block, then a block from the plain cell A1 on: the command bolds all of it
        selectRange(e, QStringLiteral("A1:B2"));
        QVERIFY(!e.win.w.act(QStringLiteral("fmt.bold"))->isChecked());
        run(e, "fmt.bold");
        QVERIFY(lookAt(e, QStringLiteral("A1")).bold && lookAt(e, QStringLiteral("B2")).bold);
        // undo, one step at a time
        e.win.w.undoStack()->undo();
        QVERIFY(!lookAt(e, QStringLiteral("A1")).bold);
        QVERIFY(lookAt(e, QStringLiteral("C3")).strike);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:5"));
    }

    void theFontBoxesAndColorsFormatTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("B2:C3"));
        auto *font = e.win.w.findChild<FontCombo *>(QStringLiteral("fmt.fontName"));
        auto *size = e.win.w.findChild<SizeCombo *>(QStringLiteral("fmt.fontSize"));
        auto *fill = e.win.w.findChild<ColorButton *>(QStringLiteral("fmt.fillColor"));
        auto *color = e.win.w.findChild<ColorButton *>(QStringLiteral("fmt.fontColor"));
        QVERIFY(font && size && fill && color);
        font->familyChosen(QStringLiteral("Georgia"));
        QCOMPARE(lookAt(e, QStringLiteral("B2")).family, QStringLiteral("Georgia"));
        QCOMPARE(lookAt(e, QStringLiteral("C3")).family, QStringLiteral("Georgia"));
        QCOMPARE(lookAt(e, QStringLiteral("A1")).family, QStringLiteral("Calibri"));
        size->sizeChosen(16);
        QCOMPARE(lookAt(e, QStringLiteral("B3")).pointSize, 16.0);
        run(e, "fmt.growFont");
        QCOMPARE(lookAt(e, QStringLiteral("B3")).pointSize, 18.0);
        run(e, "fmt.shrinkFont");
        run(e, "fmt.shrinkFont");
        QCOMPARE(lookAt(e, QStringLiteral("B3")).pointSize, 14.0);
        fill->colorPicked(ColorRef::rgb(QColor(0x11, 0x22, 0x33)));
        QVERIFY(lookAt(e, QStringLiteral("C2")).pattern == PatternKind::Solid);
        QCOMPARE(lookAt(e, QStringLiteral("C2")).fg, QRgb(0xFF112233));
        color->colorPicked(ColorRef::rgb(QColor(0xAA, 0, 0)));
        QCOMPARE(lookAt(e, QStringLiteral("C2")).textColor, QRgb(0xFFAA0000));
        fill->colorPicked(ColorRef::none());
        QVERIFY(!lookAt(e, QStringLiteral("C2")).hasFill());
        color->colorPicked(ColorRef::none());   // automatic
        QCOMPARE(lookAt(e, QStringLiteral("C2")).textColor, QRgb(0xFF000000));
        // the boxes show the cell with the cursor
        e.select(QStringLiteral("A1"));
        QCOMPARE(lookAt(e, QStringLiteral("A1")).family, QStringLiteral("Calibri"));
        QCOMPARE(e.win.w.undoStack()->count(), 9);
        // saved and read again
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        const grid::CellLook &look = g.looks().look(g.cellAt(1, 1)->style);
        QCOMPARE(look.family, QStringLiteral("Georgia"));
        QCOMPARE(look.pointSize, 14.0);
    }

    // The number buttons, in the language the window is set to; and the number format box shows the format of the cell with the cursor and sets it.
    // The buttons are made to show a cell's format only when the format of the cell with the cursor is not the one they show: moving among cells of one format, and keeping an entry, cost
    // them nothing (counted), and a move to a cell of another format, a format given, and Undo of it do.
    void theButtonsAreOnlyMadeAgainWhenTheFormatChanges()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        e.win.w.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        auto *box = e.win.w.findChild<QComboBox *>(QStringLiteral("fmt.numberFormat"));
        QVERIFY(box);
        QAction *bold = e.win.w.act(QStringLiteral("fmt.bold"));
        e.select(QStringLiteral("B2"));
        const int start = e.win.w.formatStateRuns();
        // cells of the format General with no font changes: nothing is made again
        e.select(QStringLiteral("B3"));
        e.select(QStringLiteral("C2"));
        e.select(QStringLiteral("B2"));
        QCOMPARE(e.win.w.formatStateRuns(), start);
        // an entry kept (the cell's value is changed, its format is not, and the cursor moves down)
        e.enter(QStringLiteral("12"));
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.formatStateRuns(), start);
        QCOMPARE(box->currentIndex(), 0);
        // a format given is shown at once, by the command's own call
        e.select(QStringLiteral("B2"));
        run(e, "fmt.bold");
        QVERIFY(bold->isChecked());
        const int bolded = e.win.w.formatStateRuns();
        QVERIFY(bolded > start);
        // a move to a cell of another format shows it, and a move back shows this one
        e.select(QStringLiteral("B3"));
        QVERIFY(!bold->isChecked());
        QVERIFY(e.win.w.formatStateRuns() > bolded);
        e.select(QStringLiteral("B2"));
        QVERIFY(bold->isChecked());
        // Undo of the format, with the cursor on the cell: the buttons show the format it has now
        e.win.w.undoStack()->undo();
        QVERIFY(!bold->isChecked());
        e.win.w.undoStack()->redo();
        QVERIFY(bold->isChecked());
        // the number box follows the language's marks too: a new language makes it be read again
        const int before = e.win.w.formatStateRuns();
        e.win.w.setLocale(QLocale(QLocale::German, QLocale::Germany));
        QVERIFY(e.win.w.formatStateRuns() > before);
        e.win.w.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    void theNumberButtonsAndBox()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        e.win.w.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        e.select(QStringLiteral("B2"));
        run(e, "fmt.percent");
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("500%"));
        run(e, "fmt.percent");   // (on again: off)
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5"));
        run(e, "fmt.decimalInc");
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5.0"));
        run(e, "fmt.decimalInc");
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5.00"));
        run(e, "fmt.decimalDec");
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5.0"));
        run(e, "fmt.currency");
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("$5.00"));
        run(e, "fmt.comma");
        QVERIFY2(e.shownAt(QStringLiteral("B2")).trimmed() == QStringLiteral("5.00"), qPrintable(e.shownAt(QStringLiteral("B2"))));
        run(e, "fmt.numberGeneral");
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5"));
        // the box
        auto *box = e.win.w.findChild<QComboBox *>(QStringLiteral("fmt.numberFormat"));
        QVERIFY(box);
        QCOMPARE(box->currentIndex(), 0);
        box->setCurrentIndex(9);   // scientific
        box->activated(9);
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("5.00E+00"));
        QCOMPARE(box->currentIndex(), 9);
        e.select(QStringLiteral("A1"));
        QCOMPARE(box->currentIndex(), 0);
        e.select(QStringLiteral("B2"));
        QCOMPARE(box->currentIndex(), 9);
        // a single step each
        QCOMPARE(e.win.w.undoStack()->count(), 9);
        // the Accounting Number Format takes a currency from a list: the symbol in front or after, and the decimals of that money
        e.select(QStringLiteral("C3"));
        run(e, "fmt.accounting.gbp");
        QVERIFY2(e.shownAt(QStringLiteral("C3")).contains(QChar(0x00A3)) && e.shownAt(QStringLiteral("C3")).contains(QStringLiteral("9.00")), qPrintable(e.shownAt(QStringLiteral("C3"))));
        run(e, "fmt.accounting.eur");
        QVERIFY2(e.shownAt(QStringLiteral("C3")).trimmed().endsWith(QChar(0x20AC)), qPrintable(e.shownAt(QStringLiteral("C3"))));
        run(e, "fmt.accounting.jpy");
        QVERIFY2(e.shownAt(QStringLiteral("C3")).contains(QChar(0x00A5)) && !e.shownAt(QStringLiteral("C3")).contains(QLatin1Char('.')), qPrintable(e.shownAt(QStringLiteral("C3"))));
        // the currency is the language's
        e.win.w.setLocale(QLocale(QLocale::German, QLocale::Germany));
        e.select(QStringLiteral("C2"));
        run(e, "fmt.currency");
        const QString euros = e.shownAt(QStringLiteral("C2"));
        QVERIFY2(euros.startsWith(QStringLiteral("6,00")) && euros.contains(QChar(0x20AC)), qPrintable(euros));
        e.win.w.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    void theAlignmentButtons()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("A1:B2"));
        run(e, "fmt.alignLeft");
        QVERIFY(lookAt(e, QStringLiteral("B2")).horizontal == HAlign::Left);
        QVERIFY(e.win.w.act(QStringLiteral("fmt.alignLeft"))->isChecked());
        run(e, "fmt.alignLeft");
        QVERIFY(lookAt(e, QStringLiteral("B2")).horizontal == HAlign::General);
        run(e, "fmt.alignCenter");
        QVERIFY(lookAt(e, QStringLiteral("A1")).horizontal == HAlign::Center);
        run(e, "fmt.alignRight");
        QVERIFY(lookAt(e, QStringLiteral("A2")).horizontal == HAlign::Right);
        run(e, "fmt.alignTop");
        QVERIFY(lookAt(e, QStringLiteral("B1")).vertical == VAlign::Top);
        run(e, "fmt.alignMiddle");
        QVERIFY(lookAt(e, QStringLiteral("B1")).vertical == VAlign::Center);
        run(e, "fmt.alignBottom");
        QVERIFY(lookAt(e, QStringLiteral("B1")).vertical == VAlign::Bottom);
        run(e, "fmt.wrap");
        QVERIFY(lookAt(e, QStringLiteral("B2")).wrap);
        run(e, "fmt.wrap");
        QVERIFY(!lookAt(e, QStringLiteral("B2")).wrap);
        // indent: aligned to the left to be indented, and plain again at none
        e.select(QStringLiteral("C3"));
        run(e, "fmt.indentInc");
        run(e, "fmt.indentInc");
        QVERIFY(lookAt(e, QStringLiteral("C3")).horizontal == HAlign::Left);
        QCOMPARE(lookAt(e, QStringLiteral("C3")).indent, 2);
        run(e, "fmt.indentDec");
        run(e, "fmt.indentDec");
        QCOMPARE(lookAt(e, QStringLiteral("C3")).indent, 0);
        QVERIFY(lookAt(e, QStringLiteral("C3")).horizontal == HAlign::General);
        run(e, "fmt.indentDec");   // (nothing to take off: no step)
        const int steps = e.win.w.undoStack()->count();
        run(e, "fmt.indentDec");
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // orientation
        run(e, "fmt.turn.ccw");
        QCOMPARE(lookAt(e, QStringLiteral("C3")).rotation, 45);
        run(e, "fmt.turn.cw");
        QCOMPARE(lookAt(e, QStringLiteral("C3")).rotation, 135);
        run(e, "fmt.turn.vertical");
        QCOMPARE(lookAt(e, QStringLiteral("C3")).rotation, 255);
        run(e, "fmt.turn.none");
        QCOMPARE(lookAt(e, QStringLiteral("C3")).rotation, 0);
    }

    void mergeAndCenterAsksBeforeDiscarding()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        QStringList asked;
        bool allow = false;
        e.win.w.setConfirmAsker([&](const QString &title, const QString &text) {
            asked << title + QStringLiteral(": ") + text;
            return allow;
        });
        selectRange(e, QStringLiteral("A1:B1"));
        run(e, "fmt.merge");
        QCOMPARE(asked.size(), 1);   // (B1 holds a value)
        QVERIFY(e.win.w.calculation()->mergesOf(0).isEmpty());
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        allow = true;
        run(e, "fmt.merge");
        QCOMPARE(asked.size(), 2);
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 1);
        QVERIFY(lookAt(e, QStringLiteral("A1")).horizontal == HAlign::Center);
        QVERIFY(e.win.w.act(QStringLiteral("fmt.merge"))->isChecked());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // on a merged cell the button takes the merge away
        run(e, "fmt.merge");
        QVERIFY(e.win.w.calculation()->mergesOf(0).isEmpty());
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 1);
        // merge across: each row alone
        e.win.w.undoStack()->undo();
        selectRange(e, QStringLiteral("A3:C3"));
        run(e, "fmt.merge.across");
        QCOMPARE(e.win.w.calculation()->mergesOf(0).size(), 1);
        run(e, "fmt.merge.unmerge");
        QVERIFY(e.win.w.calculation()->mergesOf(0).isEmpty());
    }

    // The Borders menu: each command puts its lines on the edges of the block (in one step), the line style chosen is used, and No Border takes them all away.
    void theBordersMenu()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("B2:C3"));
        run(e, "fmt.border.outside");
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(lookAt(e, QStringLiteral("B2")).left.style == BorderStyle::Thin && lookAt(e, QStringLiteral("B2")).top.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("C3")).right.style == BorderStyle::Thin && lookAt(e, QStringLiteral("C3")).bottom.style == BorderStyle::Thin);
        QVERIFY(lookAt(e, QStringLiteral("B2")).right.style == BorderStyle::None);
        QVERIFY(lookAt(e, QStringLiteral("A2")).right.style == BorderStyle::Thin);   // (the cell beside shares the edge)
        run(e, "fmt.border.all");
        QVERIFY(lookAt(e, QStringLiteral("B2")).right.style == BorderStyle::Thin && lookAt(e, QStringLiteral("B2")).bottom.style == BorderStyle::Thin);
        run(e, "fmt.border.thickOutside");
        QVERIFY(lookAt(e, QStringLiteral("B2")).left.style == BorderStyle::Thick);
        run(e, "fmt.border.none");
        QVERIFY(lookAt(e, QStringLiteral("B2")).left.style == BorderStyle::None && lookAt(e, QStringLiteral("C3")).right.style == BorderStyle::None);
        QVERIFY(lookAt(e, QStringLiteral("B2")).right.style == BorderStyle::None && lookAt(e, QStringLiteral("B2")).bottom.style == BorderStyle::None);
        // two lines in one step
        const int before = e.win.w.undoStack()->count();
        run(e, "fmt.border.topDoubleBottom");
        QCOMPARE(e.win.w.undoStack()->count(), before + 1);
        QVERIFY(lookAt(e, QStringLiteral("B2")).top.style == BorderStyle::Thin && lookAt(e, QStringLiteral("C3")).bottom.style == BorderStyle::Double);
        e.win.w.undoStack()->undo();
        QVERIFY(lookAt(e, QStringLiteral("B2")).top.style == BorderStyle::None && lookAt(e, QStringLiteral("C3")).bottom.style == BorderStyle::None);
        // the line style chosen in the menu is the one the commands use; the button repeats the last command
        QMenu *borders = menuNamed(e, "bordersMenu");
        QVERIFY(borders);
        QMenu *lines = nullptr;
        for (QAction *a : borders->actions())
            if (a->menu() && a->text() == QStringLiteral("Line Style")) lines = a->menu();
        QVERIFY(lines);
        QAction *dashed = actionTextOf(lines, QStringLiteral("Dashed"));
        QVERIFY(dashed);
        dashed->trigger();
        e.select(QStringLiteral("A1"));
        run(e, "fmt.border.bottom");
        QVERIFY(lookAt(e, QStringLiteral("A1")).bottom.style == BorderStyle::Dashed);
        e.select(QStringLiteral("B1"));
        run(e, "fmt.borders");
        QVERIFY(lookAt(e, QStringLiteral("B1")).bottom.style == BorderStyle::Dashed);
        QCOMPARE(e.win.told.size(), 0);
    }

    void cellStylesAreGivenAndSaved()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("B2:B3"));
        QMenu *styles = menuNamed(e, "stylesMenu");
        QVERIFY(styles);
        styles->aboutToShow();
        QAction *good = actionTextOf(styles, QStringLiteral("Good"));
        QVERIFY(good);
        QVERIFY(actionTextOf(styles, QStringLiteral("Heading 1")) && actionTextOf(styles, QStringLiteral("60% - Accent4")) && actionTextOf(styles, QStringLiteral("Percent")));
        good->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(lookAt(e, QStringLiteral("B2")).pattern == PatternKind::Solid);
        QCOMPARE(lookAt(e, QStringLiteral("B2")).fg, QRgb(0xFFC6EFCE));
        QCOMPARE(lookAt(e, QStringLiteral("B3")).textColor, QRgb(0xFF006100));
        QVERIFY(!lookAt(e, QStringLiteral("B1")).hasFill());
        // a second time is the same cell format, and Percent changes the number format of the style
        QAction *percent = actionTextOf(styles, QStringLiteral("Percent"));
        QVERIFY(percent);
        selectRange(e, QStringLiteral("C1"));
        percent->trigger();
        QCOMPARE(e.shownAt(QStringLiteral("C1")), QStringLiteral("300%"));
        e.win.w.undoStack()->undo();
        QCOMPARE(e.shownAt(QStringLiteral("C1")), QStringLiteral("3"));
        // saved: the style is in the file under its name and number
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        const QByteArray part = p.part(QStringLiteral("/xl/styles.xml"));
        QVERIFY2(part.contains("name=\"Good\"") && part.contains("builtinId=\"26\""), part.constData());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        QCOMPARE(g.looks().look(g.cellAt(1, 1)->style).fg, QRgb(0xFFC6EFCE));
    }

    // New Cell Style keeps the look of the cell with the cursor under a name: it is in the gallery, the selected cells are given it, it is saved with the workbook under that name, and a name that
    // is taken or empty is refused with the reason.
    void aNewCellStyleKeepsTheLookOfACell()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        e.select(QStringLiteral("A1"));
        run(e, "fmt.bold");
        run(e, "fmt.numberGeneral");
        const int steps = e.win.w.undoStack()->count();
        QString answer = QStringLiteral("My Total");
        e.win.w.setStyleNameAsker([&answer](QString *name) {
            *name = answer;
            return true;
        });
        selectRange(e, QStringLiteral("A1"));
        QVERIFY(e.win.w.act(QStringLiteral("style.new"))->isEnabled());
        run(e, "style.new");
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QMenu *styles = menuNamed(e, "stylesMenu");
        QVERIFY(styles);
        styles->aboutToShow();
        QAction *mine = actionTextOf(styles, QStringLiteral("My Total"));
        QVERIFY(mine);
        // another cell is given the style from the gallery: bold, as the cell it was made from
        selectRange(e, QStringLiteral("B2"));
        QVERIFY(!lookAt(e, QStringLiteral("B2")).bold);
        mine->trigger();
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold);
        // a taken name (whatever its capitals), and an empty one, are refused
        const int before = e.win.w.undoStack()->count();
        answer = QStringLiteral("my total");
        run(e, "style.new");
        answer = QStringLiteral("Good");
        run(e, "style.new");
        answer = QStringLiteral("   ");
        run(e, "style.new");
        QCOMPARE(e.win.told.size(), 3);
        QCOMPARE(e.win.w.undoStack()->count(), before);
        // saved under its name, with no built-in number
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        const QByteArray part = p.part(QStringLiteral("/xl/styles.xml"));
        QVERIFY2(part.contains("name=\"My Total\""), part.constData());
        QVERIFY(!part.contains("name=\"My Total\" builtinId"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        QVERIFY(g.looks().look(g.cellAt(0, 0)->style).bold);
    }

    // Undo takes a New Cell Style back out whole (the style, and the cell formats made for it), so the same name can be made again and nothing unused is written; a name with a control character
    // is refused.
    void undoOfANewCellStyleTakesTheStyleOut()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        const jp::office::sheet::Workbook *book = e.win.w.session()->workbook();
        const int xfs = int(book->styles().styleXfs.size()), names = int(book->styles().cellStyles.size());
        e.select(QStringLiteral("A1"));
        run(e, "fmt.bold");
        const int steps = e.win.w.undoStack()->count();
        e.win.w.setStyleNameAsker([](QString *name) {
            *name = QStringLiteral("Mine");
            return true;
        });
        const int formatsBold = int(book->styles().formats.size());
        run(e, "style.new");
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(int(book->styles().cellStyles.size()), names + 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(int(book->styles().styleXfs.size()), xfs);
        QCOMPARE(int(book->styles().cellStyles.size()), names);
        QCOMPARE(int(book->styles().formats.size()), formatsBold);
        QVERIFY(book->addedStyles().isEmpty());
        QMenu *styles = menuNamed(e, "stylesMenu");
        styles->aboutToShow();
        QVERIFY(!actionTextOf(styles, QStringLiteral("Mine")));
        // the same name again works, and Redo after Undo brings it back
        run(e, "style.new");
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->redo();
        QCOMPARE(int(book->styles().cellStyles.size()), names + 1);
        QCOMPARE(int(book->addedStyles().size()), 1);
        e.win.w.undoStack()->undo();
        // nothing unused is written
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        QVERIFY(!p.part(QStringLiteral("/xl/styles.xml")).contains("Mine"));
        // control characters are refused
        e.win.told.clear();
        e.win.w.setStyleNameAsker([](QString *name) {
            *name = QStringLiteral("Bad\x01Name");
            return true;
        });
        const int before = e.win.w.undoStack()->count();
        run(e, "style.new");
        QCOMPARE(e.win.told.size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), before);
        QCOMPARE(int(book->styles().cellStyles.size()), names);
    }

    // Conditional formatting from the Home tab: a rule asked for is on the selected cells and drawn at once, goes first, is one step of Undo, is saved as `conditionalFormatting` with its
    // differential format in the styles part, and comes back when the file is opened; Clear Rules takes rules off.
    void conditionalFormattingRulesAreMadeDrawnSavedAndCleared()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        EditWindow e;
        QVERIFY(e.open(uiBook()));                                                  // A1:C3 hold 1 to 9
        CrunchWindow::CondAnswer answered;
        e.win.w.setCondAsker([&answered](CrunchWindow::CondAnswer *a) {
            *a = answered;
            return true;
        });
        auto fillOf = [&](const QString &ref) {
            quint32 r = 0, c = 0;
            parseCellRef(ref, &r, &c);
            const grid::GridSheet &g = e.view()->sheet();
            return g.lookOf(r, c, g.cellAt(r, c) ? g.cellAt(r, c)->style : 0).fg;
        };
        selectRange(e, QStringLiteral("A1:C3"));
        answered.kind = Kind::Greater;
        answered.value1 = QStringLiteral("5");
        answered.preset = 2;                                                        // green
        run(e, "cond.greater");
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 1);
        QCOMPARE(fillOf(QStringLiteral("C3")), QRgb(0xFFC6EFCE));                   // 9 is over 5
        QVERIFY(fillOf(QStringLiteral("A1")) != QRgb(0xFFC6EFCE));                  // 1 is not
        // a second rule goes first
        answered.kind = Kind::Between;
        answered.value1 = QStringLiteral("2");
        answered.value2 = QStringLiteral("3");
        answered.preset = 1;                                                        // yellow
        run(e, "cond.between");
        const QVector<jp::office::sheet::CondRule> rules = e.win.w.session()->condRules(0);
        QCOMPARE(int(rules.size()), 2);
        QCOMPARE(rules[0].priority, 1);
        QVERIFY(rules[0].op == jp::office::sheet::CondRule::Operator::Between && rules[1].op == jp::office::sheet::CondRule::Operator::GreaterThan);
        QCOMPARE(fillOf(QStringLiteral("B1")), QRgb(0xFFFFEB9C));                   // 2 is between 2 and 3
        // text that contains, duplicates, top and bottom, above average
        answered.kind = Kind::Top;
        answered.value1 = QStringLiteral("2");
        answered.preset = 0;
        run(e, "cond.top");
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 3);
        QVERIFY(e.win.w.session()->condRules(0)[0].type == jp::office::sheet::CondRule::Type::Top10 && e.win.w.session()->condRules(0)[0].rank == 2);
        answered.kind = Kind::Contains;
        answered.value1 = QStringLiteral("x");
        run(e, "cond.contains");
        answered.kind = Kind::Duplicate;
        run(e, "cond.duplicate");
        answered.kind = Kind::Below;
        run(e, "cond.below");
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 6);
        // undo takes one rule off
        pressUndo(&e.win.w);
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 5);
        e.win.w.undoStack()->redo();
        // a rule with no value, and one with a text that is a number or not
        e.win.told.clear();
        answered.kind = Kind::Greater;
        answered.value1 = QString();
        run(e, "cond.greater");
        QCOMPARE(e.win.told.size(), 1);
        // saved and opened again
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        const QByteArray sheet = p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QCOMPARE(sheet.count("<conditionalFormatting"), 6);
        QVERIFY(sheet.contains("type=\"cellIs\"") && sheet.contains("operator=\"between\""));
        QVERIFY(sheet.contains("type=\"top10\"") && sheet.contains("rank=\"2\"") && sheet.contains("type=\"duplicateValues\"") && sheet.contains("type=\"aboveAverage\""));
        QVERIFY(sheet.indexOf("<conditionalFormatting") > sheet.indexOf("</sheetData>"));
        QVERIFY(p.part(QStringLiteral("/xl/styles.xml")).contains("<dxfs count=\"3\">"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        QCOMPARE(int(g.data().sheet->conditionalFormats.size()), 6);
        QCOMPARE(g.lookOf(2, 2, g.cellAt(2, 2)->style).fg, QRgb(0xFFFFC7CE));   // (9 is in the top two, whose rule has the highest priority: red, not green)
        // clear rules from selected cells (a rule on cells none of which are selected stays), then from the sheet
        selectRange(e, QStringLiteral("A1:A1"));
        run(e, "cond.clearSelection");
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 6);                  // (A1 is cut out of each rule's cells: the rules stay on the others)
        QVERIFY(e.win.w.session()->condRules(0)[0].ranges.size() > 1);
        e.win.w.undoStack()->undo();
        QCOMPARE(rangeName(e.win.w.session()->condRules(0)[0].ranges[0]), QStringLiteral("A1:C3"));
        selectRange(e, QStringLiteral("E5"));
        QVERIFY(!e.win.w.act(QStringLiteral("cond.clearSelection"))->isEnabled());      // (dimmed: no rule is on E5)
        QVERIFY(e.win.w.act(QStringLiteral("cond.clearSheet"))->isEnabled());
        e.win.w.act(QStringLiteral("cond.clearSelection"))->trigger();                   // (dimmed: it does nothing)
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 6);                  // (none on E5: nothing changes)
        run(e, "cond.clearSheet");
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 0);
        QVERIFY(e.settle());
        QSignalSpy savedAgain(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(savedAgain, 20000), qPrintable(e.win.told.join('|')));
        Package q;
        QVERIFY2(q.readLazyFile(e.path, &why), qPrintable(why));
        QVERIFY(!q.part(QStringLiteral("/xl/worksheets/sheet1.xml")).contains("conditionalFormatting"));
        QVERIFY(!e.win.w.act(QStringLiteral("cond.clearSheet"))->isEnabled());           // (nothing left to clear)
    }

    // ---- conditional formatting: what a typed value may be, how each kind of rule evaluates, clearing by ranges, and what is refused

    // A rule of the sheet that is on one cell with the look of a preset: the cell's fill.
    static QRgb fillAt(EditWindow &e, const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const grid::GridSheet &g = e.view()->sheet();
        return g.lookOf(r, c, g.cellAt(r, c) ? g.cellAt(r, c)->style : 0).fg;
    }
    static void ask(EditWindow &e, CrunchWindow::CondAnswer::Kind kind, const QString &v1 = QString(), const QString &v2 = QString(), int preset = 2)
    {
        CrunchWindow::CondAnswer a;
        a.kind = kind;
        a.value1 = v1;
        a.value2 = v2;
        a.preset = preset;
        e.win.w.setCondAsker([a](CrunchWindow::CondAnswer *out) {
            *out = a;
            return true;
        });
    }

    // Values that cannot be a rule's are refused with the reason (a formula that does not parse, infinity, "not a number").
    void typedValuesThatCannotBeARuleAreRefused()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        EnglishUs english;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("A1:C3"));
        for (const QString &bad : {QStringLiteral("=SUM("), QStringLiteral("inf"), QStringLiteral("nan"), QStringLiteral("=")}) {
            e.win.told.clear();
            ask(e, Kind::Greater, bad);
            run(e, "cond.greater");
            QVERIFY2(e.win.told.size() == 1 && e.win.w.session()->condRules(0).isEmpty(), qPrintable(bad));
        }
    }

    // From and To of Between go in order.
    void betweenPutsItsValuesInOrder()
    {
        EnglishUs english;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("A1:C3"));
        ask(e, CrunchWindow::CondAnswer::Kind::Between, QStringLiteral("9"), QStringLiteral("2"));
        run(e, "cond.between");
        QCOMPARE(e.win.w.session()->condRules(0)[0].formula1, QStringLiteral("2"));
        QCOMPARE(e.win.w.session()->condRules(0)[0].formula2, QStringLiteral("9"));
    }

    // TRUE and a date are values and not texts; other words are texts.
    void equalToReadsTrueAndDatesAsValues()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        EnglishUs english;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("A1:C3"));
        ask(e, Kind::Equal, QStringLiteral("TRUE"));
        run(e, "cond.equal");
        QCOMPARE(e.win.w.session()->condRules(0)[0].formula1, QStringLiteral("TRUE"));
        ask(e, Kind::Equal, QStringLiteral("1/5/2026"));
        run(e, "cond.equal");
        QCOMPARE(e.win.w.session()->condRules(0)[0].formula1, QStringLiteral("46027"));   // (a day number: January 5, 2026)
        ask(e, Kind::Equal, QStringLiteral("hello"));
        run(e, "cond.equal");
        QCOMPARE(e.win.w.session()->condRules(0)[0].formula1, QStringLiteral("\"hello\""));
    }

    // The cells a typed formula names are as seen from the cell with the cursor: the cursor is on C3 and the cells are B2:C3, so "=D5" is "C4" for the first cell.
    void aTypedFormulaIsSeenFromTheCellWithTheCursor()
    {
        EnglishUs english;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        e.view()->selectRange(2, 2, 1, 1);
        QCOMPARE(e.view()->selection().activeRow, qint64(2));
        ask(e, CrunchWindow::CondAnswer::Kind::Greater, QStringLiteral("=D5"));
        run(e, "cond.greater");
        QCOMPARE(e.win.w.session()->condRules(0)[0].formula1, QStringLiteral("C4"));
    }

    // Every kind of rule is true for the cells it should be and no others (checked by what is drawn).
    void everyKindOfRuleEvaluatesPerCell()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        const QRgb green = 0xFFC6EFCE;
        auto marked = [&](EditWindow &e, const QStringList &refs) {
            QStringList on;
            for (const QString &r : refs)
                if (fillAt(e, r) == green) on << r;
            return on.join(QLatin1Char(' '));
        };
        const QStringList cells = {"A1", "A2", "A3", "A4", "B1", "B2", "B3"};
        auto book = [] { return sheetOf({"A1=1", "A2=2", "A3=2", "A4=9", "B1=apple", "B2=banana", "B3=pineapple"}); };
        {
            EditWindow e;
            QVERIFY(e.open(book()));
            selectRange(e, QStringLiteral("A1:A4"));
            ask(e, Kind::Top, QStringLiteral("1"));
            run(e, "cond.top");
            QCOMPARE(marked(e, cells), QStringLiteral("A4"));
        }
        {
            EditWindow e;
            QVERIFY(e.open(book()));
            selectRange(e, QStringLiteral("A1:A4"));
            ask(e, Kind::Duplicate);
            run(e, "cond.duplicate");
            QCOMPARE(marked(e, cells), QStringLiteral("A2 A3"));
        }
        {
            EditWindow e;
            QVERIFY(e.open(book()));
            selectRange(e, QStringLiteral("A1:A4"));
            ask(e, Kind::Below);
            run(e, "cond.below");
            QCOMPARE(marked(e, cells), QStringLiteral("A1 A2 A3"));
        }
        {
            EditWindow e;
            QVERIFY(e.open(book()));
            selectRange(e, QStringLiteral("B1:B3"));
            ask(e, Kind::Contains, QStringLiteral("apple"));
            run(e, "cond.contains");
            QCOMPARE(marked(e, cells), QStringLiteral("B1 B3"));
        }
    }

    // Clear Rules of the selected cells takes the cells out of a rule's ranges (splitting them), and drops a rule only when no cells are left; no overlap changes nothing. Undo and Redo go back
    // and forth.
    void clearingRulesTrimsAndSplitsTheirRanges()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        crunch::EditSession *s = e.win.w.session();
        selectRange(e, QStringLiteral("A1:C3"));
        ask(e, Kind::Greater, QStringLiteral("0"));
        run(e, "cond.greater");
        // the middle cell cleared: the rule keeps the four blocks around it
        QVERIFY(s->clearCondRules(0, rangeOf(QStringLiteral("B2"))).ok);
        QVector<jp::office::sheet::CondRule> rules = s->condRules(0);
        QCOMPARE(int(rules.size()), 1);
        QStringList where;
        for (const auto &r : rules[0].ranges) where << rangeName(r);
        where.sort();
        QCOMPARE(where, QStringList({"A1:C1", "A2", "A3:C3", "C2"}));
        QVERIFY(fillAt(e, QStringLiteral("B2")) != fillAt(e, QStringLiteral("A1")));
        // cells that are not under the rule: nothing is changed, and no step is made
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(s->clearCondRules(0, rangeOf(QStringLiteral("E5:F6"))).ok);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // undo and redo of the clear
        e.win.w.undoStack()->undo();
        QCOMPARE(rangeName(s->condRules(0)[0].ranges[0]), QStringLiteral("A1:C3"));
        e.win.w.undoStack()->redo();
        QCOMPARE(int(s->condRules(0)[0].ranges.size()), 4);
        // every cell cleared: the rule is dropped
        QVERIFY(s->clearCondRules(0, rangeOf(QStringLiteral("A1:C3"))).ok);
        QVERIFY(s->condRules(0).isEmpty());
    }

    // Undo of a rule takes the look the rule made out of the styles (and Redo puts it back where it was); the count of looks is read from where the fixture starts.
    void undoOfARuleTakesItsLookOutOfTheStyles()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        crunch::EditSession *s = e.win.w.session();
        const jp::office::sheet::Workbook *book = s->workbook();
        const int dxfsAtStart = int(book->styles().dxfs.size());
        selectRange(e, QStringLiteral("A1:C3"));
        ask(e, Kind::Greater, QStringLiteral("0"));
        run(e, "cond.greater");
        QCOMPARE(int(book->styles().dxfs.size()), dxfsAtStart + 1);
        e.win.w.undoStack()->undo();
        QVERIFY(s->condRules(0).isEmpty());
        QCOMPARE(int(book->styles().dxfs.size()), dxfsAtStart);
        QVERIFY(book->addedDxfs().isEmpty());
        e.win.w.undoStack()->redo();
        QCOMPARE(int(s->condRules(0).size()), 1);
        QCOMPARE(int(book->styles().dxfs.size()), dxfsAtStart + 1);
        QCOMPARE(s->condRules(0)[0].dxf, qint32(dxfsAtStart));
    }

    // A look the file has that matches a rule's on bold, italic, color, and fill but also has a font size is not reused (the size is not kept in what is read, and the rule would take it too).
    void aFileLookWithMoreThanTheRuleHasIsNotReused()
    {
        const QString dxfs = QStringLiteral("<dxfs count=\"1\"><dxf><font><b/><color rgb=\"FF1F3864\"/><sz val=\"20\"/></font></dxf></dxfs>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))}, QByteArray(), QString(), QString(), uiStyles(dxfs))));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(int(s->workbook()->styles().dxfs.size()), 1);
        selectRange(e, QStringLiteral("A1:A1"));
        ask(e, CrunchWindow::CondAnswer::Kind::Greater, QStringLiteral("1"), QString(), 5);   // (bold, dark blue text)
        run(e, "cond.greater");
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(s->condRules(0)[0].dxf, qint32(1));                                           // (a new look: the file's own is left as it is)
        QCOMPARE(int(s->workbook()->styles().dxfs.size()), 2);
    }

    // The steps of rules are counted with the other steps of Undo: Add and Clear each grow the count by about the rules they hold.
    void theStepsOfRulesAreCountedInTheUndoBytes()
    {
        using Kind = CrunchWindow::CondAnswer::Kind;
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        crunch::EditSession *s = e.win.w.session();
        selectRange(e, QStringLiteral("A1:C3"));
        const qint64 start = s->undoBytes();
        ask(e, Kind::Greater, QStringLiteral("0"));
        run(e, "cond.greater");
        const qint64 afterAdd = s->undoBytes();
        QVERIFY2(afterAdd - start >= qint64(sizeof(jp::office::sheet::CondRule)), qPrintable(QString::number(afterAdd - start)));   // (a rule is held for Undo and for Redo)
        QVERIFY(s->clearCondRules(0, jp::office::sheet::CellRange()).ok);
        QVERIFY(s->undoBytes() - afterAdd >= qint64(sizeof(jp::office::sheet::CondRule)));
    }

    // On a sheet of 5,000 rules, moving the selection stays quick (the buttons of Clear Rules look at a bounded number of rules).
    void movingTheSelectionOnASheetOfManyRulesStaysQuick()
    {
        QString rules;
        for (int i = 0; i < 5000; ++i)
            rules += QStringLiteral("<conditionalFormatting sqref=\"A%1\"><cfRule type=\"cellIs\" priority=\"%1\" operator=\"equal\"><formula>1</formula></cfRule></conditionalFormatting>").arg(i + 1);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), rules)}, QByteArray(), QString(), QString(), uiStyles())));
        QVERIFY(e.settle());
        QCOMPARE(int(e.win.w.session()->condRules(0).size()), 5000);
        QElapsedTimer timer;
        timer.start();
        for (int i = 0; i < 200; ++i) e.view()->selectCell(quint32(10000 + i), quint32(i % 5));
        QVERIFY2(timer.elapsed() < 20000, qPrintable(QString::number(timer.elapsed())));
        QVERIFY(e.win.w.act(QStringLiteral("cond.clearSheet"))->isEnabled());
    }

    // Rules the file has that JeffCrunch does not hold whole are not rewritten: an extension list, the 2010 block, a table's mark, a kind it does not know, more ranges than it reads, a workbook
    // opened from another kind of file, and a sheet with more rules than it changes. Nothing changes and the reason is told.
    void sheetsWhoseRulesAreNotHeldWholeAreLeftAlone()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row>");
        const QString plain = QStringLiteral("<conditionalFormatting sqref=\"A1\"><cfRule type=\"cellIs\" priority=\"3\" operator=\"equal\"><formula>1</formula></cfRule></conditionalFormatting>");
        QString many = QStringLiteral("<conditionalFormatting sqref=\"A1");
        for (int i = 0; i < 1100; ++i) many += QStringLiteral(" B%1").arg(i + 1);
        many += QStringLiteral("\"><cfRule type=\"cellIs\" priority=\"1\" operator=\"equal\"><formula>1</formula></cfRule></conditionalFormatting>");
        QString thousands;
        for (int i = 0; i < 5200; ++i) thousands += QStringLiteral("<conditionalFormatting sqref=\"A%1\"><cfRule type=\"cellIs\" priority=\"%1\" operator=\"equal\"><formula>1</formula></cfRule></conditionalFormatting>").arg(i + 1);
        const QString x14 = QStringLiteral("<extLst><ext uri=\"{78C0D931-6437-407d-A8EE-F0AAD7539E65}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\"><x14:conditionalFormattings>"
                                           "<x14:conditionalFormatting><x14:cfRule type=\"iconSet\" priority=\"2\" id=\"{1}\"/></x14:conditionalFormatting></x14:conditionalFormattings></ext></extLst>");
        const QStringList books = {
            plain + QStringLiteral("<conditionalFormatting sqref=\"B1\"><cfRule type=\"iconSet\" priority=\"1\"><iconSet iconSet=\"3Arrows\"><cfvo type=\"percent\" val=\"0\"/><cfvo type=\"percent\" val=\"33\"/>"
                                   "<cfvo type=\"percent\" val=\"67\"/></iconSet><extLst><ext uri=\"{B025F937-C7B1-47D3-B67F-A62EFF666E3E}\"><x14:id xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">{1}</x14:id></ext></extLst></cfRule></conditionalFormatting>"),
            plain + x14,
            QStringLiteral("<conditionalFormatting sqref=\"A1\" pivot=\"1\"><cfRule type=\"cellIs\" priority=\"1\" operator=\"equal\"><formula>1</formula></cfRule></conditionalFormatting>"),
            plain + QStringLiteral("<conditionalFormatting sqref=\"B1\"><cfRule type=\"somethingNew\" priority=\"1\"/></conditionalFormatting>"),
            many,
            thousands};
        for (int at = 0; at < books.size(); ++at) {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({worksheet(rows, books[at])}, QByteArray(), QString(), QString(), uiStyles())));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            const QVector<jp::office::sheet::CondRule> before = s->condRules(0);
            selectRange(e, QStringLiteral("A1:B1"));
            ask(e, CrunchWindow::CondAnswer::Kind::Greater, QStringLiteral("0"));
            e.win.told.clear();
            run(e, "cond.greater");
            QVERIFY2(e.win.told.size() == 1, qPrintable(QStringLiteral("book %1 was changed").arg(at)));
            QVERIFY(!s->clearCondRules(0, jp::office::sheet::CellRange()).ok);
            QCOMPARE(s->condRules(0).size(), before.size());
            for (qsizetype i = 0; i < before.size(); ++i) QCOMPARE(s->condRules(0)[i].priority, before[i].priority);
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QVERIFY(s->workbook()->addedDxfs().isEmpty());
        }
        // a workbook opened from a text file keeps its rules for the .xlsx it is saved as: no change now
        EditWindow csv;
        QVERIFY(csv.open(QByteArray("a,b\n1,2\n"), QStringLiteral("data.csv")));
        QVERIFY(!csv.win.w.session()->clearCondRules(0, jp::office::sheet::CellRange()).ok);
        CrunchWindow::CondAnswer::Kind greater = CrunchWindow::CondAnswer::Kind::Greater;
        selectRange(csv, QStringLiteral("A1:B2"));
        ask(csv, greater, QStringLiteral("0"));
        csv.win.told.clear();
        run(csv, "cond.greater");
        QCOMPARE(csv.win.told.size(), 1);
    }

    // A sheet whose file has rules of several kinds that JeffCrunch holds whole (a color scale and an icon set, no extension) takes a new rule, and the others come out of the save as they were.
    void aRuleAddedToASheetWithOtherKindsOfRulesKeepsThem()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\"><v>3</v></c></row>");
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"A1:A1\"><cfRule type=\"colorScale\" priority=\"2\"><colorScale><cfvo type=\"min\"/><cfvo type=\"max\"/><color rgb=\"FFF8696B\"/><color rgb=\"FF63BE7B\"/></colorScale></cfRule></conditionalFormatting>"
                                             "<conditionalFormatting sqref=\"B1:B1\"><cfRule type=\"iconSet\" priority=\"1\"><iconSet iconSet=\"3Arrows\"><cfvo type=\"percent\" val=\"0\"/><cfvo type=\"percent\" val=\"33\"/><cfvo type=\"percent\" val=\"67\"/></iconSet></cfRule></conditionalFormatting>"
                                             "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, after)}, QByteArray(), QString(), QString(), uiStyles())));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("C1:C1"));
        ask(e, CrunchWindow::CondAnswer::Kind::Greater, QStringLiteral("1"));
        run(e, "cond.greater");
        QVERIFY2(e.win.w.session()->condRules(0).size() == 3, qPrintable(e.win.told.join('|')));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        const QByteArray xml = p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QCOMPARE(xml.count("<conditionalFormatting"), 3);
        QVERIFY(xml.contains("<colorScale>") && xml.contains("FFF8696B") && xml.contains("<iconSet iconSet=\"3Arrows\">"));
        QVERIFY(xml.contains("priority=\"3\"") && xml.contains("priority=\"2\"") && xml.contains("priority=\"1\""));
        QVERIFY(xml.indexOf("<pageMargins") > xml.lastIndexOf("</conditionalFormatting>"));   // (the schema's order holds)
        QVERIFY(xml.indexOf("<conditionalFormatting") > xml.indexOf("</sheetData>"));
    }

    // A sheet whose elements have a prefix (<x:worksheet>) is written with the rules in the same prefix, and they come back.
    void rulesAreWrittenInThePrefixOfASheetThatHasOne()
    {
        QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))}, QByteArray(), QString(), QString(), uiStyles());
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(book, parts));
        QString xml = QString::fromUtf8(parts[QStringLiteral("xl/worksheets/sheet1.xml")]);
        xml.replace(QRegularExpression(QStringLiteral("<(/?)([A-Za-z])")), QStringLiteral("<\\1x:\\2"));
        xml.replace(QStringLiteral("<x:?xml"), QStringLiteral("<?xml"));
        xml.replace(QStringLiteral("xmlns=\""), QStringLiteral("xmlns:x=\""));
        parts[QStringLiteral("xl/worksheets/sheet1.xml")] = xml.toUtf8();
        jp::ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
        EditWindow e;
        QVERIFY(e.open(z.finish()));
        QVERIFY(e.settle());
        selectRange(e, QStringLiteral("A1:A1"));
        ask(e, CrunchWindow::CondAnswer::Kind::Greater, QStringLiteral("1"));
        run(e, "cond.greater");
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join('|')));
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        const QByteArray out = p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY2(out.contains("<x:conditionalFormatting") && out.contains("<x:cfRule") && out.contains("<x:formula>1</x:formula>"), out.constData());
        QVERIFY(!out.contains("<cfRule"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(int(second.w.view()->sheet().data().sheet->conditionalFormats.size()), 1);
    }

    // The Format Painter: a click takes the format of the cells selected and the next cells selected get it; a double click keeps it on until Esc.
    void theFormatPainter()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        e.select(QStringLiteral("A1"));
        run(e, "fmt.bold");
        run(e, "fmt.alignRight");
        run(e, "fmt.painter");
        QVERIFY(e.win.w.formatPainterActive() && !e.win.w.formatPainterLocked());
        QVERIFY(e.win.w.act(QStringLiteral("fmt.painter"))->isChecked());
        const int before = e.win.w.undoStack()->count();
        e.select(QStringLiteral("B2"));
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold && lookAt(e, QStringLiteral("B2")).horizontal == HAlign::Right);
        QCOMPARE(e.win.w.undoStack()->count(), before + 1);
        QVERIFY(!e.win.w.formatPainterActive());
        QVERIFY(!e.win.w.act(QStringLiteral("fmt.painter"))->isChecked());
        e.select(QStringLiteral("C3"));   // (the painter is done: nothing more is painted)
        QVERIFY(!lookAt(e, QStringLiteral("C3")).bold);
        // locked: a second click soon after the first
        e.select(QStringLiteral("A1"));
        run(e, "fmt.painter");
        run(e, "fmt.painter");
        QVERIFY(e.win.w.formatPainterLocked());
        e.select(QStringLiteral("C1"));
        e.select(QStringLiteral("C3"));
        QVERIFY(lookAt(e, QStringLiteral("C1")).bold && lookAt(e, QStringLiteral("C3")).bold);
        QVERIFY(e.win.w.formatPainterLocked());
        QTest::keyClick(&e.win.w, Qt::Key_Escape);
        QVERIFY(!e.win.w.formatPainterActive());
        e.select(QStringLiteral("B3"));
        QVERIFY(!lookAt(e, QStringLiteral("B3")).bold);
        // a block of cells is painted over a block: the block taken (A1 bold over A2 plain) is repeated across the cells selected
        selectRange(e, QStringLiteral("A1:A2"));
        run(e, "fmt.painter");
        e.view()->selectRange(1, 1, 2, 2);
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold && lookAt(e, QStringLiteral("C2")).bold);
        QVERIFY(!lookAt(e, QStringLiteral("B3")).bold && !lookAt(e, QStringLiteral("C3")).bold);
    }

    void clearTakesAwayFormatsOrAll()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        selectRange(e, QStringLiteral("B2:C3"));
        run(e, "fmt.bold");
        run(e, "fmt.border.outside");
        run(e, "edit.clear.formats");
        QVERIFY(!lookAt(e, QStringLiteral("B2")).bold && lookAt(e, QStringLiteral("B2")).left.style == BorderStyle::None);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:5"));
        run(e, "fmt.bold");
        run(e, "edit.clear.contents");
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("-"));
        run(e, "edit.clear.all");
        QVERIFY(!lookAt(e, QStringLiteral("B2")).bold);
        e.win.w.undoStack()->undo();
        QVERIFY(lookAt(e, QStringLiteral("B2")).bold || e.win.w.undoStack()->index() > 0);
    }

    // Format Cells, as a person uses it: the dialog is made from the cell with the cursor, and what was changed on its tabs is made as one step.
    void theDialogsChangesAreOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        e.win.w.setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QCOMPARE(e.win.w.act(QStringLiteral("format.cells"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_1));
        selectRange(e, QStringLiteral("B2:C3"));
        int asked = 0;
        e.win.w.setFormatAsker([&](const FormatCellsDialog::Input &in, FormatCellsDialog::Result *out) {
            ++asked;
            // the dialog starts from the cell with the cursor
            if (in.view.fontName != QStringLiteral("Calibri") || in.view.fontSize != 11 || in.oneCell || in.sampleNumber != 5) return false;
            FormatCellsDialog d(in);
            d.findChild<QComboBox *>(QStringLiteral("fontName"))->setCurrentText(QStringLiteral("Georgia"));
            d.findChild<QComboBox *>(QStringLiteral("fontStyle"))->setCurrentIndex(2);
            d.findChild<ColorButton *>(QStringLiteral("fillColor"))->colorPicked(ColorRef::rgb(QColor(0xFF, 0xEB, 0x9C)));
            d.findChild<QToolButton *>(QStringLiteral("borderInsideH"))->click();
            d.findChild<QToolButton *>(QStringLiteral("borderTop"))->click();
            d.findChild<QComboBox *>(QStringLiteral("horizontal"))->setCurrentIndex(2);
            *out = d.result();
            return true;
        });
        run(e, "format.cells");
        QCOMPARE(asked, 1);
        QCOMPARE(e.win.w.undoStack()->count(), 1);   // (a macro: one step)
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Format Cells"));
        for (const char *ref : {"B2", "C2", "B3", "C3"}) {
            QVERIFY2(lookAt(e, QString::fromLatin1(ref)).bold, ref);
            QVERIFY2(lookAt(e, QString::fromLatin1(ref)).family == QStringLiteral("Georgia"), ref);
            QVERIFY2(lookAt(e, QString::fromLatin1(ref)).fg == QRgb(0xFFFFEB9C), ref);
            QVERIFY2(lookAt(e, QString::fromLatin1(ref)).horizontal == HAlign::Center, ref);
        }
        QVERIFY(lookAt(e, QStringLiteral("B2")).top.style == BorderStyle::Thin && lookAt(e, QStringLiteral("C2")).bottom.style == BorderStyle::Thin);   // (the line between rows, and the top)
        QVERIFY(lookAt(e, QStringLiteral("B3")).top.style == BorderStyle::Thin);
        QVERIFY(!lookAt(e, QStringLiteral("A1")).bold);
        e.win.w.undoStack()->undo();
        QVERIFY(!lookAt(e, QStringLiteral("B2")).bold && !lookAt(e, QStringLiteral("B2")).hasFill() && lookAt(e, QStringLiteral("B2")).top.style == BorderStyle::None);
        e.win.w.undoStack()->redo();
        QVERIFY(lookAt(e, QStringLiteral("C3")).bold);
        // a dialog that is given up changes nothing; one that changed nothing makes no step
        e.win.w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *) { return false; });
        run(e, "format.cells");
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.win.w.setFormatAsker([](const FormatCellsDialog::Input &in, FormatCellsDialog::Result *out) {
            FormatCellsDialog d(in);
            *out = d.result();
            return true;
        });
        run(e, "format.cells");
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.told.size(), 0);
    }

    void theCommandsAreOffUntilTheWorkbookCanBeChanged()
    {
        // (the commands are off while the workbook cannot be changed: a save is going on, or nothing is open)
        Window empty;
        for (const char *id : {"fmt.bold", "fmt.alignLeft", "format.cells", "fmt.painter", "edit.clear"}) QVERIFY2(!empty.w.act(QString::fromLatin1(id))->isEnabled(), id);
        EditWindow e;
        QVERIFY(e.open(uiBook()));
        for (const char *id : {"fmt.bold", "fmt.alignLeft", "format.cells", "fmt.painter", "edit.clear"}) QVERIFY2(e.win.w.act(QString::fromLatin1(id))->isEnabled(), id);
        QCOMPARE(e.win.w.ribbonErrors(), QString());
    }
};

JP_TEST_CLASS(CrunchFormatUiTests)
#include "tst_crunch_formatui.moc"
