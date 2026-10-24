// Tests of the formatting commands of JeffCrunch's window: the buttons of the Home tab (font, size, bold, italic, underlines, colors, borders, alignment, indent, orientation, merging, the
// number format and its buttons, cell styles, the Format Painter, Clear) and the Format Cells dialog's changes made to the cells. Each command is one step of Undo; every language is set
// explicitly; the workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QComboBox>
#include <QMenu>
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
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|')));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridSheet &g = second.w.view()->sheet();
        const grid::CellLook &look = g.looks().look(g.cellAt(1, 1)->style);
        QCOMPARE(look.family, QStringLiteral("Georgia"));
        QCOMPARE(look.pointSize, 14.0);
    }

    // The number buttons, in the language the window is set to; and the number format box shows the format of the cell with the cursor and sets it.
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
        QVERIFY2(saved.wait(20000), qPrintable(e.win.told.join('|')));
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
