// Pointing at cells while a formula is typed: a click or a drag writes the address, a second click replaces it, Ctrl+click adds one, the arrow keys move a pointer, F2 and F4, another
// sheet's tab, the headers, Esc, and the colored outlines of the references. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/sheettabs.h"
#include "app/functionargumentsdialog.h"
#include "app/insertfunctiondialog.h"
#include "core/appinfo.h"
#include "core/formularefs.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "findbooks.h"
#include "app/theme.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "testrunner.h"

#include <QApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

QByteArray twoSheets() { return buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>10</v></c></row>")), worksheet(QStringLiteral("<row r=\"3\"><c r=\"B3\"><v>5</v></c></row>"))}, QByteArray(), QString(), QString(), QByteArray(), {QStringLiteral("Sheet1"), QStringLiteral("My Sheet")}); }

QPoint cellPoint(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const QRect cell = e.view()->cellRect(r, c);
    return QPoint(cell.left() + 6, cell.top() + 4);
}

void click(EditWindow &e, const QString &ref, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, m, cellPoint(e, ref)); }

void drag(EditWindow &e, const QPoint &from, const QPoint &to)
{
    QWidget *vp = e.view()->viewport();
    QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
    QTest::mouseMove(vp, (from + to) / 2);
    QTest::mouseMove(vp, to);
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
}

QByteArray mergedBook() { return buildXlsx({worksheet(QStringLiteral("<row r=\"2\"><c r=\"B2\"><v>1</v></c></row>"), QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"B2:C3\"/></mergeCells>"))}); }

QByteArray salesBook()
{
    auto t = [](const QString &ref, const QString &v) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, v); };
    auto n = [](const QString &ref, int v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v); };
    return buildXlsx({worksheet(QStringLiteral("<row r=\"1\">") + t(QStringLiteral("A1"), QStringLiteral("Region")) + t(QStringLiteral("B1"), QStringLiteral("Q1")) + QStringLiteral("</row><row r=\"2\">") + t(QStringLiteral("A2"), QStringLiteral("East")) + n(QStringLiteral("B2"), 1)
                                 + QStringLiteral("</row><row r=\"3\">") + t(QStringLiteral("A3"), QStringLiteral("West")) + n(QStringLiteral("B3"), 2) + QStringLiteral("</row><row r=\"4\">") + t(QStringLiteral("A4"), QStringLiteral("North")) + n(QStringLiteral("B4"), 3) + QStringLiteral("</row>"))});
}

QPoint columnHeader(EditWindow &e, const QString &ref)
{
    const QPoint c = cellPoint(e, ref + QStringLiteral("1"));
    return QPoint(c.x(), e.view()->sheet().layout().headerHeight() / 2);
}

QPoint rowHeader(EditWindow &e, int row)
{
    const QPoint c = cellPoint(e, QStringLiteral("A%1").arg(row));
    return QPoint(e.view()->sheet().layout().headerWidth() / 2, c.y());
}

} // namespace

class CrunchPointTests : public QObject {
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

    // The references of a formula: where they are, not the names of functions or what is in quotes or brackets, and F4's four forms.
    void referencesAreFoundInTheText()
    {
        using crunch::FormulaRef;
        const auto refs = crunch::findReferences(QStringLiteral("=SUM(A1:B2,'My Sheet'!$C$3)+LOG10(4)+\"D4\"+Sheet2!E:F+3:5+Sales[Q1]+A1B"));
        QCOMPARE(int(refs.size()), 4);
        QCOMPARE(refs[0].kind, FormulaRef::Kind::Range);
        QCOMPARE(refs[0].start, 5);
        QCOMPARE(refs[0].length, 5);
        QCOMPARE(refs[1].sheet, QStringLiteral("My Sheet"));
        QCOMPARE(refs[1].row1, quint32(2));
        QCOMPARE(refs[2].kind, FormulaRef::Kind::Columns);
        QCOMPARE(refs[2].col1, quint32(4));
        QCOMPARE(refs[2].col2, quint32(5));
        QCOMPARE(refs[3].kind, FormulaRef::Kind::Rows);
        QCOMPARE(refs[3].row2, quint32(4));
        QCOMPARE(crunch::nextFixedForm(QStringLiteral("A1")), QStringLiteral("$A$1"));
        QCOMPARE(crunch::nextFixedForm(QStringLiteral("$A$1")), QStringLiteral("A$1"));
        QCOMPARE(crunch::nextFixedForm(QStringLiteral("A$1")), QStringLiteral("$A1"));
        QCOMPARE(crunch::nextFixedForm(QStringLiteral("$A1")), QStringLiteral("A1"));
        QCOMPARE(crunch::nextFixedForm(QStringLiteral("B2:C3")), QStringLiteral("$B$2:$C$3"));
        QCOMPARE(crunch::nextFixedForm(QStringLiteral("D:E")), QStringLiteral("$D:$E"));
        QCOMPARE(crunch::referenceText(FormulaRef::Kind::Range, 4, 3, 1, 1), QStringLiteral("B2:D5"));
        QCOMPARE(crunch::referenceText(FormulaRef::Kind::Columns, 0, 1, 0, 3), QStringLiteral("B:D"));
        QCOMPARE(crunch::referenceText(FormulaRef::Kind::Rows, 1, 0, 4, 0), QStringLiteral("2:5"));
    }

    // A click on a cell writes its address, a second click before anything else is typed replaces it, a click after an operator writes another, and the selection stays where it was.
    void aClickWritesTheAddressAndASecondClickReplacesIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("="));
        click(e, QStringLiteral("B2"));
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=B2"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=B2"));
        click(e, QStringLiteral("D3"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=D3"));
        QTest::keyClicks(w.cellEditor(), QStringLiteral("+"));
        click(e, QStringLiteral("A1"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=D3+A1"));
        QCOMPARE(e.selected(), QStringLiteral("F1"));
        QVERIFY(w.cellEditor()->hasFocus());
        e.key(Qt::Key_Escape);
    }

    // A drag writes the block (whichever corner it begins at); Ctrl+click adds another reference after the list separator.
    void aDragWritesABlockAndCtrlClickAddsAnother()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM("));
        drag(e, cellPoint(e, QStringLiteral("B2")), cellPoint(e, QStringLiteral("D5")));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(B2:D5"));
        click(e, QStringLiteral("A1"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A1"));
        drag(e, cellPoint(e, QStringLiteral("D5")), cellPoint(e, QStringLiteral("B2")));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(B2:D5"));
        click(e, QStringLiteral("A1"));
        click(e, QStringLiteral("A3"), Qt::ControlModifier);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A1,A3"));
        click(e, QStringLiteral("A4"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A1,A4"));
        e.key(Qt::Key_Escape);
    }

    // The arrow keys move a pointer from the formula's cell, and Shift extends it to a block.
    void arrowsMoveAPointerAndShiftExtendsIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("="));
        e.key(Qt::Key_Down);
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=F2"));
        e.key(Qt::Key_Right);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=G2"));
        e.key(Qt::Key_Down, Qt::ShiftModifier);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=G2:G3"));
        e.key(Qt::Key_Down, Qt::ShiftModifier);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=G2:G4"));
        e.key(Qt::Key_Left);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=F4"));
        QTest::keyClicks(w.cellEditor(), QStringLiteral("+"));
        e.key(Qt::Key_Down);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=F4+F2"));   // (a new reference starts from the formula's cell again)
        // after a whole operand an arrow ends the entry, as it does for any typed text
        e.key(Qt::Key_Return);
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=1"));
        e.key(Qt::Key_Down);
        QVERIFY(!w.isEditing());
        QCOMPARE(e.selected(), QStringLiteral("F2"));
    }

    // F2 changes between pointing and editing the text, and the status bar says which: Point, Enter, Edit.
    void f2ChangesBetweenPointingAndEditing()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("="));
        QCOMPARE(w.statusText(), QStringLiteral("Point"));
        e.key(Qt::Key_F2);
        QCOMPARE(w.statusText(), QStringLiteral("Edit"));
        e.key(Qt::Key_Down);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("="));   // (the arrow does not point in the text's mode)
        e.key(Qt::Key_F2);
        QCOMPARE(w.statusText(), QStringLiteral("Point"));
        e.key(Qt::Key_Down);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=F2"));
        QTest::keyClicks(w.cellEditor(), QStringLiteral("+1"));
        QCOMPARE(w.statusText(), QStringLiteral("Enter"));
        e.key(Qt::Key_Escape);
        QCOMPARE(w.statusText(), QStringLiteral("Ready"));
        // the entry of text has no pointing to show
        e.type(QStringLiteral("ab"));
        QCOMPARE(w.statusText(), QStringLiteral("Enter"));
        e.key(Qt::Key_Escape);
    }

    // F4 goes round the four forms of the reference at or before the cursor.
    void f4CyclesTheFixedParts()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=A1+B2"));
        for (const char *want : {"=A1+$B$2", "=A1+B$2", "=A1+$B2", "=A1+B2"}) {
            e.key(Qt::Key_F4);
            QCOMPARE(w.cellEditor()->text(), QString::fromLatin1(want));
        }
        w.cellEditor()->setCursorPosition(2);
        e.key(Qt::Key_F4);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=$A$1+B2"));
        e.key(Qt::Key_Escape);
        e.type(QStringLiteral("=SUM("));
        drag(e, cellPoint(e, QStringLiteral("A1")), cellPoint(e, QStringLiteral("B2")));
        e.key(Qt::Key_F4);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM($A$1:$B$2"));
        click(e, QStringLiteral("C3"));   // (the F4 form is part of the reference that was written: a click replaces it)
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(C3"));
        e.key(Qt::Key_Escape);
    }

    // A click on another sheet's tab goes on pointing there; the name is written with the cell, in quotes when it needs them; Enter goes back and puts the formula in its cell.
    void anotherSheetCanBePointedAt()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("D1"));
        e.type(QStringLiteral("="));
        w.sheetTabs()->setCurrentIndex(1);
        QCOMPARE(w.activeSheet(), 1);
        QVERIFY(w.isEditing());
        click(e, QStringLiteral("B3"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("='My Sheet'!B3"));
        QTest::keyClicks(w.formulaBar(), QStringLiteral("+"));
        QTest::keyClick(w.formulaBar(), Qt::Key_Down);
        QVERIFY(w.formulaBar()->text().startsWith(QStringLiteral("='My Sheet'!B3+'My Sheet'!")));
        click(e, QStringLiteral("B3"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("='My Sheet'!B3+'My Sheet'!B3"));
        w.sheetTabs()->setCurrentIndex(0);
        QVERIFY(w.isEditing());
        click(e, QStringLiteral("A2"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("='My Sheet'!B3+A2"));   // (back on the first sheet the click replaces the reference just written, with no sheet's name)
    }

    void enterOnAnotherSheetGoesBackAndCommits()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("D1"));
        e.type(QStringLiteral("=2*"));
        w.sheetTabs()->setCurrentIndex(1);
        click(e, QStringLiteral("B3"));
        QVERIFY(w.isEditing());
        QTest::keyClick(w.formulaBar(), Qt::Key_Return);
        QVERIFY(!w.isEditing());
        QCOMPARE(w.activeSheet(), 0);
        QCOMPARE(w.sheetTabs()->currentIndex(), 0);
        QCOMPARE(w.session()->editText(0, 0, 3), QStringLiteral("=2*'My Sheet'!B3"));
        QCOMPARE(e.selected(), QStringLiteral("D2"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:10"));
    }

    // Esc gives up everything: the entry, the sheet that was shown, and the outlines.
    void escRestoresTheSheetAndTheCell()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->index();
        e.select(QStringLiteral("D1"));
        e.type(QStringLiteral("=SUM("));
        w.sheetTabs()->setCurrentIndex(1);
        click(e, QStringLiteral("B3"));
        QTest::keyClick(w.formulaBar(), Qt::Key_Escape);
        QVERIFY(!w.isEditing());
        QCOMPARE(w.activeSheet(), 0);
        QCOMPARE(w.session()->editText(0, 0, 3), QString());
        QCOMPARE(w.undoStack()->index(), steps);
        QVERIFY(e.view()->pointMarks().empty());
        QCOMPARE(e.selected(), QStringLiteral("D1"));
    }

    // A click on a column header writes A:A, a row header 3:3; dragging along headers gives B:D and 2:5.
    void headersWriteWholeColumnsAndRows()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM("));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, columnHeader(e, QStringLiteral("A")));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(A:A"));
        drag(e, columnHeader(e, QStringLiteral("B")), columnHeader(e, QStringLiteral("D")));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(B:D"));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, rowHeader(e, 3));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(3:3"));
        drag(e, rowHeader(e, 2), rowHeader(e, 5));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(2:5"));
        QCOMPARE(e.selected(), QStringLiteral("F1"));
        e.key(Qt::Key_Escape);
    }

    // The entry is one step of Undo, and what is pointed at is calculated.
    void thePointedFormulaIsOneUndoStep()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->index();
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM("));
        drag(e, cellPoint(e, QStringLiteral("A1")), cellPoint(e, QStringLiteral("A2")));
        QTest::keyClicks(w.cellEditor(), QStringLiteral(")"));
        e.key(Qt::Key_Return);
        QCOMPARE(w.session()->editText(0, 0, 5), QStringLiteral("=SUM(A1:A2)"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:11"));
        QCOMPARE(w.undoStack()->index(), steps + 1);
        pressUndo(e.view());
        QCOMPARE(w.session()->editText(0, 0, 5), QString());
    }

    // Where a reference may not go, a click is not pointing: after a whole operand it ends the entry and selects the cell; in a text in quotes it writes nothing.
    void noPointingAfterAnOperandOrInText()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=A1"));
        click(e, QStringLiteral("D4"));
        QVERIFY(!w.isEditing());
        QCOMPARE(w.session()->editText(0, 0, 5), QStringLiteral("=A1"));
        QCOMPARE(e.selected(), QStringLiteral("D4"));
        e.select(QStringLiteral("F2"));
        e.type(QStringLiteral("=\"a"));
        click(e, QStringLiteral("D4"));
        QVERIFY(w.isEditing());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=\"a"));
        e.key(Qt::Key_Escape);
        // text that is not a formula is not pointed at either
        e.select(QStringLiteral("F3"));
        e.type(QStringLiteral("x"));
        click(e, QStringLiteral("D4"));
        QVERIFY(!w.isEditing());
        QCOMPARE(w.session()->editText(0, 2, 5), QStringLiteral("x"));
    }


    // A merged range is one cell to point at: a click anywhere in it writes the whole range.
    void aMergedCellIsPointedAtAsItsWholeRange()
    {
        EditWindow e;
        QVERIFY(e.open(mergedBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM("));
        click(e, QStringLiteral("C3"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(B2:C3"));
        click(e, QStringLiteral("B2"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM(B2:C3"));
        e.key(Qt::Key_Escape);
    }

    // Ctrl and an arrow jump to the edge of the data as they do on the sheet; with Shift they extend.
    void ctrlArrowsJumpToTheEdgeOfTheData()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A5"));
        e.type(QStringLiteral("="));
        e.key(Qt::Key_Up, Qt::ControlModifier);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=A3"));
        e.key(Qt::Key_Up, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=A1:A3"));
        e.key(Qt::Key_Right, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=A1:C3"));   // (the walk starts at the moving corner, A1, and A3 stays the other corner)
        e.key(Qt::Key_Escape);
    }

    // The cell editor or the formula bar, whichever the entry is in, takes a click on a table's cell; a second click replaces the reference it wrote.
    void tableReferencesAreWrittenToTheRightLineAndReplaced()
    {
        EditWindow e;
        QVERIFY(e.open(salesBook()));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.session()->makeTable(0, [] { jp::office::sheet::CellRange r; parseRangeRef(QStringLiteral("A1:B4"), &r); return r; }(), true, QString()).ok);
        QVERIFY(e.settle());
        e.select(QStringLiteral("F3"));
        e.type(QStringLiteral("="));
        click(e, QStringLiteral("B3"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Table1[@Q1]"));
        click(e, QStringLiteral("B1"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Table1[[#Headers],[Q1]]"));
        click(e, QStringLiteral("D4"));   // (a cell that is in no table replaces it too)
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=D4"));
        click(e, QStringLiteral("B3"));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=Table1[@Q1]"));
        e.key(Qt::Key_Escape);
        // an entry begun in the formula bar
        e.select(QStringLiteral("F3"));
        w.formulaBar()->setFocus();
        QTest::keyClicks(w.formulaBar(), QStringLiteral("="));
        QVERIFY(w.isEditing());
        click(e, QStringLiteral("B3"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=Table1[@Q1]"));
        click(e, QStringLiteral("A4"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=A4"));
        QTest::keyClick(w.formulaBar(), Qt::Key_Escape);
    }

    // A highlight's border moves its reference and its corner square stretches it, the text following as the mouse moves; the dollar signs stay; the entry is one step of Undo.
    void draggingAHighlightMovesAndResizesItsReference()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        const int steps = w.undoStack()->index();
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=SUM($B$2:C3)"));
        QCOMPARE(int(e.view()->pointMarks().size()), 1);
        QWidget *vp = e.view()->viewport();
        const QRect b2 = e.view()->cellRect(1, 1);
        // the left border of the block, dragged one column and one row on
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(b2.left() + 1, b2.center().y()));
        QTest::mouseMove(vp, QPoint(b2.left() + 1, b2.center().y()) + QPoint(b2.width(), b2.height()) / 2);
        QTest::mouseMove(vp, QPoint(b2.left() + 1 + b2.width(), b2.center().y() + b2.height()));
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM($C$3:D4)"));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(b2.left() + 1 + b2.width(), b2.center().y() + b2.height()));
        QVERIFY(w.isEditing());
        QVERIFY(w.cellEditor()->hasFocus());
        QCOMPARE(e.selected(), QStringLiteral("F1"));
        // the corner square stretched to E6
        const QRect d4 = e.view()->cellRect(3, 3);
        const QPoint corner = d4.bottomRight() - QPoint(1, 1);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, corner);
        const QPoint to = e.view()->cellRect(5, 4).center();
        QTest::mouseMove(vp, to);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=SUM($C$3:E6)"));
        QCOMPARE(w.formulaBar()->text(), QStringLiteral("=SUM($C$3:E6)"));
        e.key(Qt::Key_Return);
        QCOMPARE(w.session()->editText(0, 0, 5), QStringLiteral("=SUM($C$3:E6)"));
        QCOMPARE(w.undoStack()->index(), steps + 1);
    }

    // The arrows step as the cell cursor does: over hidden rows and columns, and over a merged range in one step.
    void arrowsSkipHiddenLinesAndStepOverMergedRanges()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\" hidden=\"1\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row>"),
                                            QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"B3:C4\"/></mergeCells>"))})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A1"));
        e.type(QStringLiteral("="));
        e.key(Qt::Key_Down);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=A3"));   // (A2 is in a hidden row)
        e.key(Qt::Key_Escape);
        e.select(QStringLiteral("E3"));
        e.type(QStringLiteral("="));
        e.key(Qt::Key_Left);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=D3"));
        e.key(Qt::Key_Left);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=B3"));   // (the merged range B3:C4 is one step)
        e.key(Qt::Key_Right);
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=D3"));
        e.key(Qt::Key_Escape);
    }

    void refittingKeepsTheFixedParts()
    {
        QCOMPARE(crunch::refitBody(QStringLiteral("$B$2:C3"), QStringLiteral("C3:D4")), QStringLiteral("$C$3:D4"));
        QCOMPARE(crunch::refitBody(QStringLiteral("B2"), QStringLiteral("B2:C3")), QStringLiteral("B2:C3"));
        QCOMPARE(crunch::refitBody(QStringLiteral("A$1"), QStringLiteral("D5")), QStringLiteral("D$5"));
    }

    // The pick strip of the Function Arguments dialog writes a whole column or row as A:A or 3:3.
    void theFunctionArgumentsStripWritesWholeColumnsAndRows()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        w.setFunctionAsker([](QString *name) {
            *name = QStringLiteral("SUM");
            return true;
        });
        QStringList seen;
        w.setArgumentsAsker([&](FunctionArgumentsDialog *d) {
            d->show();
            d->pickButton(0)->click();
            e.view()->selectColumns(0, 0);
            seen << d->edit(0)->text();
            e.view()->selectColumns(1, 3);
            seen << d->edit(0)->text();
            e.view()->selectRows(2, 2);
            seen << d->edit(0)->text();
            e.view()->selectRows(1, 4);
            seen << d->edit(0)->text();
            e.view()->selectRange(1, 1, 3, 3);
            seen << d->edit(0)->text();
            d->endPicking();
            return false;
        });
        w.act(QStringLiteral("formulas.insertFunction"))->trigger();
        QCOMPARE(seen, QStringList({QStringLiteral("A:A"), QStringLiteral("B:D"), QStringLiteral("3:3"), QStringLiteral("2:5"), QStringLiteral("B2:D4")}));
    }

    // Each reference of the formula is outlined in a color of its own, and its text has the same color.
    void everyReferenceHasAColoredOutline()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("F1"));
        e.type(QStringLiteral("=A1+SUM(B2:C3)*D4"));
        const auto &marks = e.view()->pointMarks();
        QCOMPARE(int(marks.size()), 3);
        QCOMPARE(marks[0].range.row1, qint64(0));
        QCOMPARE(marks[1].range.row2, qint64(2));
        QCOMPARE(marks[1].range.col2, qint64(2));
        QCOMPARE(marks[2].range.row1, qint64(3));
        QVERIFY(marks[0].color != marks[1].color && marks[1].color != marks[2].color && marks[0].color != marks[2].color);
        QVERIFY(marks[0].color.isValid());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("=A1+SUM(B2:C3)*D4"));   // (the colors leave the text and the cursor alone)
        QCOMPARE(w.cellEditor()->cursorPosition(), 17);
        saveShot(&e.win.w, QStringLiteral("point-light"));
        applyUiTheme(2);
        saveShot(&e.win.w, QStringLiteral("point-dark"));
        applyUiTheme(1);
        e.key(Qt::Key_Escape);
        QVERIFY(e.view()->pointMarks().empty());
    }
};

JP_TEST_CLASS(CrunchPointTests)
#include "tst_crunch_point.moc"
