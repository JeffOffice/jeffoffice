// Tests of editing in JeffCrunch's window: typing over a cell, in it (F2, a double click), and in the formula bar; the keys that finish or give up an entry; what typed numbers,
// percents, money, dates, and times are given as values and formats; a formula that is not one; Delete; what depends on a cell worked out again by the worker; Undo and Redo; and saving
// (Save, Save As, a save that fails, the question when a window with changes is closed). Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "grid/gridview.h"
#include "io/zipindex.h"

#include <QAction>
#include <QApplication>
#include <QFileInfo>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QThread>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// The sheet most of these tests edit:
//        A            B              C          D
//   1    1            =A1*2 (999)    hello
//   2    10           =SUM(A1:A2)    "          2024-01-02 shown as a date (a number with a date format)
//   3    0.25 (0%)    =A3+1
// with the styles: 1 percent, 2 a date, 3 text.
QByteArray editBook(const QByteArray &extraRows = QByteArray())
{
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"9\" applyNumberFormat=\"1\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/><xf numFmtId=\"49\" applyNumberFormat=\"1\"/>"));
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>999</v></c><c r=\"C1\" t=\"s\"><v>0</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>10</v></c><c r=\"B2\"><f>SUM(A1:A2)</f><v>999</v></c><c r=\"D2\" s=\"2\"><v>45293</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\" s=\"1\"><v>0.25</v></c><c r=\"B3\"><f>A3+1</f><v>999</v></c><c r=\"C3\" s=\"3\"><v>7</v></c></row>")
        + QString::fromUtf8(extraRows);
    return buildXlsx({worksheet(rows)}, sharedStringsXml({QStringLiteral("<t>hello</t>")}), QString(), QString(), styles);
}

struct EditWindow {
    Folder folder;
    Window win;
    QString path;
    bool open(const QByteArray &bytes, const QString &name = QStringLiteral("book.xlsx"))
    {
        path = folder.write(name, bytes);
        return win.open(path);
    }
    grid::GridView *view() { return win.w.view(); }
    void select(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        view()->selectCell(r, c);
    }
    QString selected() { return view()->selectionName(); }
    // The value the grid shows for the cell, and the text it shows.
    QString valueAt(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const grid::Cell *cell = view()->sheet().cellAt(r, c);
        return cell ? view()->sheet().valueOf(*cell).debug() : QStringLiteral("-");
    }
    // The text of the cell in its format, whole (the cell may be too narrow to show it).
    QString shownAt(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        const grid::Cell *cell = view()->sheet().cellAt(r, c);
        return cell ? view()->sheet().textOf(*cell) : QString();
    }
    // Types text into the selected cell as a person does: the first character at the grid (which starts the entry), the rest into the cell's line.
    void type(const QString &text)
    {
        QTest::keyClick(view(), text.at(0).toLatin1());
        if (text.size() > 1) QTest::keyClicks(win.w.cellEditor(), text.mid(1));
    }
    void key(Qt::Key k, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::keyClick(win.w.cellEditor(), k, m); }
    // Waits until the worker has worked out what the edits made.
    bool settle() { return QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 20000); }
    void enter(const QString &text)
    {
        type(text);
        key(Qt::Key_Return);
    }
};

} // namespace

class CrunchEditTests : public QObject {
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

    // ---- typing

    // A key that prints, with a cell selected, starts an entry in that cell: the cell's line is over the cell with the character in it, and the formula bar shows it too; Enter puts it in the
    // cell and goes down.
    void typingOverACellPutsWhatWasTypedInIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("A5"));
        QVERIFY(!e.win.w.isEditing());
        QTest::keyClick(e.view(), Qt::Key_4);
        QVERIFY(e.win.w.isEditing());
        CellEditor *editor = e.win.w.cellEditor();
        QVERIFY(editor->isVisible());
        QCOMPARE(editor->text(), QStringLiteral("4"));
        QCOMPARE(editor->geometry().topLeft(), e.view()->cellRect(4, 0).topLeft());
        QVERIFY(editor->hasFocus() || editor->isVisible());
        QTest::keyClick(editor, Qt::Key_2);
        QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("42"));
        QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("-"));      // (not in the cell until it is finished)
        e.key(Qt::Key_Return);
        QVERIFY(!e.win.w.isEditing());
        QVERIFY(!editor->isVisible());
        QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("n:42"));
        QCOMPARE(e.shownAt(QStringLiteral("A5")), QStringLiteral("42"));
        QCOMPARE(e.selected(), QStringLiteral("A6"));
        QCOMPARE(e.win.w.formulaBar()->text(), QString());   // (the formula bar shows the cell the selection is on)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Typing in A5"));
        // text, replacing what the cell held
        e.select(QStringLiteral("C1"));
        e.enter(QStringLiteral("world"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:world"));
        // a space and Backspace start entries too: the second clears the cell
        e.select(QStringLiteral("A2"));
        QTest::keyClick(e.view(), Qt::Key_Backspace);
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(editor->text(), QString());
        e.key(Qt::Key_Return);
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
    }

    // Enter and Shift+Enter finish and go down and up; Tab and Shift+Tab right and left; arrows, while typing over a cell, finish and move on; Escape gives the entry up and the cell is as
    // it was; Tab, Tab, Enter goes down to the column the Tabs began in.
    void theKeysThatFinishAnEntry()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("F5"));
        e.type(QStringLiteral("1"));
        e.key(Qt::Key_Return, Qt::ShiftModifier);
        QCOMPARE(e.selected(), QStringLiteral("F4"));
        e.type(QStringLiteral("2"));
        e.key(Qt::Key_Tab);
        QCOMPARE(e.selected(), QStringLiteral("G4"));
        e.type(QStringLiteral("3"));
        e.key(Qt::Key_Tab);
        QCOMPARE(e.selected(), QStringLiteral("H4"));
        e.type(QStringLiteral("4"));
        e.key(Qt::Key_Return);                        // the Tabs began in F: Enter goes down to F5
        QCOMPARE(e.selected(), QStringLiteral("F5"));
        e.type(QStringLiteral("5"));
        e.key(Qt::Key_Backtab, Qt::ShiftModifier);
        QCOMPARE(e.selected(), QStringLiteral("E5"));
        e.type(QStringLiteral("6"));
        e.key(Qt::Key_Down);
        QCOMPARE(e.selected(), QStringLiteral("E6"));
        e.type(QStringLiteral("7"));
        e.key(Qt::Key_Left);
        QCOMPARE(e.selected(), QStringLiteral("D6"));
        e.type(QStringLiteral("8"));
        e.key(Qt::Key_Up);
        QCOMPARE(e.selected(), QStringLiteral("D5"));
        e.type(QStringLiteral("9"));
        e.key(Qt::Key_Right);
        QCOMPARE(e.selected(), QStringLiteral("E5"));
        QCOMPARE(e.valueAt(QStringLiteral("F4")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("G4")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("H4")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("n:5"));   // (the 1 typed there first was typed over)
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("E6")), QStringLiteral("n:7"));
        QCOMPARE(e.win.w.undoStack()->count(), 9);
        // Escape
        e.select(QStringLiteral("A1"));
        e.type(QStringLiteral("9"));
        QTest::keyClicks(e.win.w.cellEditor(), "99");
        e.key(Qt::Key_Escape);
        QVERIFY(!e.win.w.isEditing());
        QVERIFY(!e.win.w.cellEditor()->isVisible());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.selected(), QStringLiteral("A1"));
        QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("1"));
        QCOMPARE(e.win.w.undoStack()->count(), 9);
        // with several cells selected, Enter goes through them
        e.view()->selectRange(10, 0, 11, 1);
        QCOMPARE(e.selected(), QStringLiteral("A11"));
        e.enter(QStringLiteral("1"));
        QCOMPARE(e.selected(), QStringLiteral("A12"));
        e.enter(QStringLiteral("2"));
        QCOMPARE(e.selected(), QStringLiteral("B11"));
    }

    // F2 and a double click edit what the cell holds, in the cell, with the cursor at the end; the arrows then move in the text (they do not finish the entry), and F2 changes that.
    void f2AndADoubleClickEditWhatTheCellHolds()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("B1"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY(e.win.w.isEditing());
        CellEditor *editor = e.win.w.cellEditor();
        QCOMPARE(editor->text(), QStringLiteral("=A1*2"));
        QCOMPARE(editor->cursorPosition(), 5);
        QVERIFY(editor->mode() == CellEditor::Mode::Editing);
        QTest::keyClick(editor, Qt::Key_Left);
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.selected(), QStringLiteral("B1"));
        QCOMPARE(editor->cursorPosition(), 4);
        QTest::keyClicks(editor, "+1");
        QCOMPARE(editor->text(), QStringLiteral("=A1*+12"));
        QTest::keyClick(editor, Qt::Key_Escape);
        QVERIFY(!e.win.w.isEditing());
        // the double click, on a cell
        const QRect r = e.view()->cellRect(2, 0);   // A3: a percent
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
        QCOMPARE(e.selected(), QStringLiteral("A3"));
        QVERIFY(!e.win.w.isEditing());
        QTest::mouseDClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.selected(), QStringLiteral("A3"));
        QCOMPARE(editor->text(), QStringLiteral("25%"));
        QTest::keyClick(editor, Qt::Key_Escape);
        // F2 toggles what an arrow does
        e.select(QStringLiteral("A1"));
        e.type(QStringLiteral("5"));
        QVERIFY(editor->mode() == CellEditor::Mode::Typing);
        QTest::keyClick(editor, Qt::Key_F2);
        QVERIFY(editor->mode() == CellEditor::Mode::Editing);
        QTest::keyClick(editor, Qt::Key_Down);
        QVERIFY(e.win.w.isEditing());
        QTest::keyClick(editor, Qt::Key_Escape);
        // a text that would be read as a number is edited with an apostrophe
        e.select(QStringLiteral("C3"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QCOMPARE(editor->text(), QStringLiteral("7"));   // (a number in a Text-formatted cell is a number: C3 holds 7 as a number)
        QTest::keyClick(editor, Qt::Key_Escape);
    }

    // The formula bar is where an entry can be made too: typing in it starts one, Enter finishes it and goes down, Escape gives it up.
    void theFormulaBarEdits()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("A2"));
        QLineEdit *bar = e.win.w.formulaBar();
        QVERIFY(!bar->isReadOnly());
        QCOMPARE(bar->text(), QStringLiteral("10"));
        bar->setFocus();
        bar->selectAll();
        QTest::keyClicks(bar, "77");
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("77"));
        QTest::keyClick(bar, Qt::Key_Return);
        QVERIFY(!e.win.w.isEditing());
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:77"));
        QCOMPARE(e.selected(), QStringLiteral("A3"));
        // Escape
        bar->setFocus();
        bar->selectAll();
        QTest::keyClicks(bar, "5");
        QVERIFY(e.win.w.isEditing());
        QTest::keyClick(bar, Qt::Key_Escape);
        QVERIFY(!e.win.w.isEditing());
        QCOMPARE(bar->text(), QStringLiteral("25%"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:0.25"));
        // what is typed in the cell shows in the bar, and what is typed in the bar in the cell
        e.select(QStringLiteral("E1"));
        e.type(QStringLiteral("a"));
        QCOMPARE(bar->text(), QStringLiteral("a"));
        e.key(Qt::Key_Escape);
    }

    // F2 and then Enter (or a click on another cell) with nothing changed puts nothing in: a step is not made, the file has nothing to save, and a formula with a function this program lacks
    // keeps the result the file stored (it was written over as this program reads it back: the prefix the format needs was dropped, and the cell had #NAME?).
    void anEntryThatWasNotChangedCommitsNothing()
    {
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>_xludf.MYFUNC(B1)</f><v>77</v></c><c r=\"B1\"><v>5</v></c><c r=\"C1\"><f>_xlfn.IMAGE(\"x\")</f><v>88</v></c>"
                                                                  "<c r=\"D1\" s=\"1\"><v>45293</v></c><c r=\"E1\"><f>B1*2</f><v>10</v></c></row>"))},
                                           QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/>")));
        EditWindow e;
        QVERIFY(e.open(book));
        for (const QString &ref : {QStringLiteral("A1"), QStringLiteral("C1"), QStringLiteral("D1"), QStringLiteral("E1")}) {
            e.select(ref);
            QTest::keyClick(e.view(), Qt::Key_F2);
            QVERIFY2(e.win.w.isEditing(), qPrintable(ref));
            e.key(Qt::Key_Return);
            QVERIFY2(!e.win.w.isEditing(), qPrintable(ref));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QVERIFY(!e.win.w.isModified());
        }
        // (and a click on another cell, with the formula bar's text as it was)
        e.select(QStringLiteral("C1"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY(e.win.w.isEditing());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->cellRect(0, 1).center());
        QVERIFY(!e.win.w.isEditing());
        QCOMPARE(e.selected(), QStringLiteral("B1"));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:77"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:88"));
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:10"));
        // a change is a change
        e.select(QStringLiteral("E1"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QTest::keyClicks(e.win.w.cellEditor(), QStringLiteral("+1"));
        e.key(Qt::Key_Return);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
    }

    // A formula typed with a function the format prefixes (that this program lacks) is kept as a file has it: with the prefix, so that the other programs know the function; the cell
    // shows #NAME? here, as it says.
    void aFormulaTypedWithAPrefixedFunctionKeepsItsPrefixInTheFile()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("F1"));
        e.enter(QStringLiteral("=image(\"x\")"));
        e.select(QStringLiteral("F2"));
        e.enter(QStringLiteral("=let(x,A1,x*2)"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("e:#NAME?"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QCOMPARE(savedFormula(e.path, QStringLiteral("F1")), QStringLiteral("_xlfn.IMAGE(\"x\")"));
        QCOMPARE(savedFormula(e.path, QStringLiteral("F2")), QStringLiteral("_xlfn.LET(_xlpm.x,A1,_xlpm.x*2)"));
        // and F2 shows it as it was typed
        Window second;
        QVERIFY(second.open(e.path));
        second.w.view()->selectCell(1, 5);
        QCOMPARE(second.w.formulaBar()->text(), QStringLiteral("=LET(x,A1,x*2)"));
    }

    // ---- what is typed becomes

    void typedNumbersGetTheFormatThatShowsThemAsTyped()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("F1"));
        e.enter(QStringLiteral("5%"));
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:0.05"));
        QCOMPARE(e.shownAt(QStringLiteral("F1")), QStringLiteral("5%"));
        e.enter(QStringLiteral("12.5%"));
        QCOMPARE(e.valueAt(QStringLiteral("F2")), QStringLiteral("n:0.125"));
        QCOMPARE(e.shownAt(QStringLiteral("F2")), QStringLiteral("12.5%"));
        e.enter(QStringLiteral("$1,234.5"));
        QCOMPARE(e.valueAt(QStringLiteral("F3")), QStringLiteral("n:1234.5"));
        QCOMPARE(e.shownAt(QStringLiteral("F3")), QStringLiteral("$1,234.50"));
        e.enter(QStringLiteral("3/4/2025"));
        QCOMPARE(e.valueAt(QStringLiteral("F4")), QStringLiteral("n:45720"));
        QCOMPARE(e.shownAt(QStringLiteral("F4")), QStringLiteral("3/4/2025"));
        e.enter(QStringLiteral("10:30 PM"));
        QCOMPARE(e.valueAt(QStringLiteral("F5")), QStringLiteral("n:0.9375"));
        QCOMPARE(e.shownAt(QStringLiteral("F5")), QStringLiteral("10:30 PM"));
        e.enter(QStringLiteral("1,000"));
        QCOMPARE(e.shownAt(QStringLiteral("F6")), QStringLiteral("1,000"));
        e.enter(QStringLiteral("TRUE"));
        QCOMPARE(e.valueAt(QStringLiteral("F7")), QStringLiteral("b:1"));
        e.enter(QStringLiteral("#N/A"));
        QCOMPARE(e.valueAt(QStringLiteral("F8")), QStringLiteral("e:#N/A"));
        e.enter(QStringLiteral("1.5"));
        QCOMPARE(e.shownAt(QStringLiteral("F9")), QStringLiteral("1.5"));
        // each is the cell's text again when edited
        e.select(QStringLiteral("F1"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("5%"));
        QTest::keyClick(e.win.w.cellEditor(), Qt::Key_Escape);
        e.select(QStringLiteral("F4"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("3/4/2025"));
        QTest::keyClick(e.win.w.cellEditor(), Qt::Key_Escape);
        // a number typed into a cell that has a format of its own keeps it
        e.select(QStringLiteral("A3"));
        e.enter(QStringLiteral("0.5"));
        QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("50%"));
        // and one into a cell formatted as text stays text
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("42"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("s:42"));
    }

    void aFormulaIsParsedAndAFaultyOneKeepsWhatWasTyped()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("E1"));
        e.type(QStringLiteral("=(1+2"));
        e.key(Qt::Key_Return);
        QVERIFY(e.win.w.isEditing());                                 // still there
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=(1+2"));
        QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=(1+2"));
        QCOMPARE(e.selected(), QStringLiteral("E1"));
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].startsWith(QLatin1String("Formula: ")) && e.win.told[0].contains(QLatin1String("can't be read")), qPrintable(e.win.told[0]));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("-"));
        // fixed, it is accepted
        e.win.w.cellEditor()->setMode(CellEditor::Mode::Editing);
        QTest::keyClicks(e.win.w.cellEditor(), ")");
        e.key(Qt::Key_Return);
        QVERIFY(!e.win.w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:3"));
        QCOMPARE(e.win.w.formulaBar()->text(), QString());
        // a text longer than a cell holds is refused with the numbers in the message
        e.select(QStringLiteral("E2"));
        e.type(QStringLiteral("x"));
        e.win.w.cellEditor()->setText(QString(32768, QLatin1Char('x')));
        e.key(Qt::Key_Return);
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.told.size(), 2);
        QVERIFY2(e.win.told[1].contains(QLatin1String("32,767")) && e.win.told[1].contains(QLatin1String("32,768")), qPrintable(e.win.told[1]));
        e.win.w.cellEditor()->setText(QString(32767, QLatin1Char('x')));
        e.key(Qt::Key_Return);
        QVERIFY(!e.win.w.isEditing());
        QCOMPARE(e.win.w.undoStack()->count(), 2);
    }

    void deleteClearsContentsAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.view()->selectRange(0, 0, 2, 2);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        for (const char *ref : {"A1", "B1", "C1", "A2", "B2", "A3", "B3", "C3"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:45293"));   // (outside the range)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(e.win.w.undoStack()->undoText().startsWith(QLatin1String("Clearing ")));
        // the formats stay: the percent cell shows a percent when something is typed in it
        e.select(QStringLiteral("A3"));
        e.enter(QStringLiteral("0.5"));
        QCOMPARE(e.shownAt(QStringLiteral("A3")), QStringLiteral("50%"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:hello"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:0.25"));
        // a cell with nothing to clear is not a step
        e.select(QStringLiteral("Q9"));
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(e.win.w.undoStack()->count(), 2);   // (the typing and the clearing, both undone and there to redo)
    }

    // ---- what depends on a cell

    // What reads a cell is worked out again by the worker, and the results come back in batches; the window says it is calculating, and says Ready when they are all there. The thread
    // that works them out is not the window's.
    void whatDependsOnACellIsWorkedOutAgain()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:11"));
        QSignalSpy done(&e.win.w, &CrunchWindow::calculationFinished);
        const qint64 batches = e.win.w.batchesTaken();
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("100"));
        QVERIFY(e.win.w.isCalculating());
        QVERIFY(done.count() > 0 || done.wait(20000));
        QVERIFY(!e.win.w.isCalculating());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:200"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:110"));
        QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("200"));
        QVERIFY(e.win.w.batchesTaken() > batches);
        QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
        QVERIFY(e.win.w.loaded()->calcThread.load() != quintptr(QThread::currentThreadId()));
        // a formula typed into a cell is worked out, and what reads that
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("=B1+B2"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:310"));
        QCOMPARE(e.win.w.formulaBar()->text(), QString());
        e.select(QStringLiteral("E1"));
        QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=B1+B2"));
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("1"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:301"));   // B1 200 + B2 101
        // a function added after the format's first release is written with the prefix the file needs, and shown without it
        e.select(QStringLiteral("E2"));
        e.enter(QStringLiteral("=stdev.s(A1:A3)"));
        QVERIFY(e.settle());
        e.select(QStringLiteral("E2"));
        QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=STDEV.S(A1:A3)"));
        const Calculation *calc = e.win.w.calculation();
        QVERIFY(calc);
        QCOMPARE(calc->content(0, 1, 4).formula, QStringLiteral("_xlfn.STDEV.S(A1:A3)"));
    }

    // An edit made while the workbook is still being calculated is absorbed: what was going on is worked out again with it in, and the last values are those of the edited workbook.
    void anEditDuringTheCalculationIsAbsorbed()
    {
        const int n = 60000;
        QString rows;
        rows.reserve(n * 90);
        for (int r = 1; r <= n; ++r)
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
        Folder folder;
        Window win;
        QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
        win.w.setWorkerPace(20000);   // (the calculation takes the time the test needs)
        QVERIFY(win.w.openFile(folder.write(QStringLiteral("big.xlsx"), buildXlsx({worksheet(rows)}))));
        QVERIFY(loaded.wait(60000));
        QVERIFY(win.w.isCalculating());
        // an entry in D1 while it goes
        win.w.view()->selectCell(0, 3);
        QTest::keyClick(win.w.view(), Qt::Key_5);
        QTest::keyClick(win.w.cellEditor(), Qt::Key_Return);
        QVERIFY(!win.w.isEditing());
        QVERIFY(win.w.isCalculating());
        win.w.setWorkerPace(0);
        QVERIFY(QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 60000));
        const grid::GridSheet &gs = win.w.view()->sheet();
        for (const quint32 r : {0u, 1u, 15000u, quint32(n - 1)}) {
            const grid::Cell *c = gs.cellAt(r, 1);
            QVERIFY(c);
            QCOMPARE(gs.valueOf(*c).debug(), QStringLiteral("n:%1").arg((r + 1) * 2 + 5));
        }
    }

    // Counted: an edit made with the worker held does not ask the graph who reads the cell, marks nothing, and registers no formula but the one typed; the worker does that after, and the
    // results come back. (The cell is read by twenty thousand formulas: were the window to look for them, the graph's work would show it.)
    void anEditCostsTheWindowOnlyWhatPuttingTheCellInCosts()
    {
        const int n = 20000;
        QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>");
        for (int r = 2; r <= n + 1; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"B%1\"><f>$A$1+%1</f><v>-1</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        qint64 queried0 = -1, queried1 = -1, dirty1 = -1, registered0 = -1, registered1 = -1, holds = 0;
        e.win.w.session()->setHoldHook([&](Calculation *calc, bool after) {
            ++holds;
            if (!after) {
                queried0 = calc->graph().queryWork();
                registered0 = calc->registrations();
            } else {
                queried1 = calc->graph().queryWork();
                dirty1 = calc->dirtyCount();
                registered1 = calc->registrations();
            }
        });
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("100"));
        QCOMPARE(holds, qint64(2));
        QCOMPARE(queried1, queried0);        // the graph was not asked
        QCOMPARE(dirty1, qint64(0));         // nothing was marked
        QCOMPARE(registered1, registered0);  // and no formula told it what it reads
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:102"));
        QCOMPARE(e.valueAt(QStringLiteral("B20001")), QStringLiteral("n:20101"));
        // a formula typed registers once
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("=A1*2"));
        QCOMPARE(registered1 - registered0, qint64(1));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:200"));
    }

    // The worker stays when the formulas are worked out, and goes with the window.
    void theWorkerStaysAndGoesWithTheWindow()
    {
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            QVERIFY(e.win.w.loaded()->calcIdle.load());
            QVERIFY(crunch::WorkbookLoader::workersRunning() >= 1);
            QVERIFY(!e.win.w.loaded()->workerEnded.load());
        }
        QTRY_COMPARE(crunch::WorkbookLoader::workersRunning(), 0);
    }

    // A calculation stopped with Esc, and then a cell typed in: the worker takes up what is left and the new change, and ends with every formula worked out.
    void anEditAfterEscResumesTheCalculation()
    {
        const int n = 60000;
        QString rows;
        rows.reserve(n * 90);
        for (int r = 1; r <= n; ++r)
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
        Folder folder;
        Window win;
        QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
        win.w.setWorkerPace(20000);
        QVERIFY(win.w.openFile(folder.write(QStringLiteral("big.xlsx"), buildXlsx({worksheet(rows)}))));
        QVERIFY(loaded.wait(60000));
        QTRY_VERIFY(win.w.batchesTaken() >= 1);
        QTest::keyClick(&win.w, Qt::Key_Escape);
        QVERIFY(!win.w.isCalculating());
        QTRY_VERIFY(win.w.loaded()->calcIdle.load());
        win.w.setWorkerPace(0);
        win.w.view()->selectCell(0, 3);
        QTest::keyClick(win.w.view(), Qt::Key_9);
        QTest::keyClick(win.w.cellEditor(), Qt::Key_Return);
        QVERIFY(win.w.isCalculating());
        QVERIFY(QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 60000));
        const grid::GridSheet &gs = win.w.view()->sheet();
        for (const quint32 r : {0u, 30000u, quint32(n - 1)}) QCOMPARE(gs.valueOf(*gs.cellAt(r, 1)).debug(), QStringLiteral("n:%1").arg((r + 1) * 2 + 9));
    }

    // Edits one after the other, before the worker has worked out the first: all of them are worked out, and the window says it is calculating until the last is.
    void editsOneAfterTheOtherAreAllWorkedOut()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.setWorkerPace(5000);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("100"));
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("200"));
        e.select(QStringLiteral("A3"));
        e.enter(QStringLiteral("300"));
        QVERIFY(e.win.w.isCalculating());
        e.win.w.setWorkerPace(0);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:200"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:300"));      // SUM(A1:A2)
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:301"));
        QCOMPARE(e.win.w.undoStack()->count(), 3);
    }

    // Nothing can be changed until the workbook is read whole: the person is told so in the status line, with no message box in the way.
    void anEditIsRefusedWhileTheWorkbookIsBeingOpened()
    {
        QString rows;
        for (int r = 1; r <= 30000; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%1</v></c></row>").arg(r);
        Folder folder;
        Window win;
        QSignalSpy first(&win.w, &CrunchWindow::firstScreenShown), loaded(&win.w, &CrunchWindow::loadFinished);
        win.w.setWorkerPace(20000);
        QVERIFY(win.w.openFile(folder.write(QStringLiteral("slow.xlsx"), buildXlsx({worksheet(rows)}))));
        QVERIFY(first.wait(60000));
        QVERIFY(win.w.isLoading());
        QTest::keyClick(win.w.view(), Qt::Key_7);
        QVERIFY(!win.w.isEditing());
        QVERIFY(win.w.statusText().contains(QLatin1String("still being opened")));
        QCOMPARE(win.told.size(), 0);
        QVERIFY(win.w.session() == nullptr);
        win.w.setWorkerPace(0);
        QVERIFY(loaded.wait(60000));
        QVERIFY(win.w.session() != nullptr);
    }

    // A cell of an array is not changed alone.
    void aCellOfAnArrayIsNotChanged()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C2\">A1:B1*2</f><v>2</v></c></row>"))})));
        e.select(QStringLiteral("C1"));
        QTest::keyClick(e.view(), Qt::Key_9);
        QVERIFY(!e.win.w.isEditing());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].contains(QLatin1String("array")), qPrintable(e.win.told[0]));
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(e.win.told.size(), 2);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    // ---- Undo and Redo

    // A typed cell is one step. Undo puts back the value, the formula, the format, and what depends on the cell; Redo does it again; the keys work, and the command dims when there is nothing.
    void undoPutsBackValuesFormulasFormatsAndDependents()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QAction *undo = e.win.w.act(QStringLiteral("edit.undo")), *redo = e.win.w.act(QStringLiteral("edit.redo"));
        QVERIFY(undo->isEnabled() == false && redo->isEnabled() == false);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("50"));           // a number over a number
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("typed"));        // text over a formula
        e.select(QStringLiteral("D2"));
        e.enter(QStringLiteral("7"));            // a number over a date (keeps its format)
        e.select(QStringLiteral("F1"));
        e.enter(QStringLiteral("25%"));          // a new format
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 4);
        QVERIFY(undo->isEnabled());
        QVERIFY(!redo->isEnabled());
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:60"));    // A1 50 + A2 10
        QTest::keyClick(&e.win.w, Qt::Key_Z, Qt::ControlModifier);            // undo "25%"
        QVERIFY(redo->isEnabled());
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("-"));
        QCOMPARE(e.selected(), QStringLiteral("F1"));                          // (the cell it changed is shown)
        QTest::keyClick(&e.win.w, Qt::Key_Z, Qt::ControlModifier);            // 7 over the date
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:45293"));
        QCOMPARE(e.shownAt(QStringLiteral("D2")), QStringLiteral("1/2/2024"));
        QTest::keyClick(&e.win.w, Qt::Key_Z, Qt::ControlModifier);            // the text over the formula
        QCOMPARE(e.win.w.formulaBar()->text(), QStringLiteral("=A1*2"));
        QTest::keyClick(&e.win.w, Qt::Key_Z, Qt::ControlModifier);            // 50 over 1
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:11"));
        QVERIFY(!undo->isEnabled());
        QCOMPARE(e.win.w.undoStack()->index(), 0);
        QVERIFY(e.win.w.undoStack()->isClean());
        // Redo with Ctrl+Y puts them back one by one
        QTest::keyClick(&e.win.w, Qt::Key_Y, Qt::ControlModifier);
        QTest::keyClick(&e.win.w, Qt::Key_Y, Qt::ControlModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:50"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("s:typed"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:60"));
        QTest::keyClick(&e.win.w, Qt::Key_Y, Qt::ControlModifier);
        QTest::keyClick(&e.win.w, Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(e.shownAt(QStringLiteral("F1")), QStringLiteral("25%"));
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:7"));
        QVERIFY(!redo->isEnabled());
        // a new edit after an undo drops what could have been redone
        QTest::keyClick(&e.win.w, Qt::Key_Z, Qt::ControlModifier);
        QVERIFY(redo->isEnabled());
        e.select(QStringLiteral("H1"));
        e.enter(QStringLiteral("1"));
        QVERIFY(!redo->isEnabled());
        QCOMPARE(e.win.w.undoStack()->count(), 4);
    }

    // Undo keeps what changed, not the workbook: a step holds the cells it changed, as they were and as they are; the stack holds at most its limit of steps.
    void undoHoldsWhatChangedAndNoMore()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        for (int i = 0; i < 1200; ++i) QVERIFY(e.win.w.session()->type(0, quint32(i % 50 + 10), 5, QString::number(i)).ok);
        QCOMPARE(e.win.w.undoStack()->count(), crunch::kUndoSteps);
    }

    // Undo back to where the file was says the file lacks nothing, but the results that follow the undo are still to come, and a Redo after them (or the same number typed again) must be
    // saved with the results it gives: the file has the old ones. (What was kept of the engine's value before the edit was lost with the undo, and the old edited value was taken for it.)
    void anUndoAndARedoStillSaveTheResultsOfTheRedo()
    {
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>2</v></c><c r=\"C1\"><f>B1+1</f><v>3</v></c></row>"))});
        for (int variant = 0; variant < 4; ++variant) {
            EditWindow e;
            QVERIFY(e.open(book));
            e.select(QStringLiteral("A1"));
            e.enter(QStringLiteral("5"));
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:11"));
            e.win.w.act(QStringLiteral("edit.undo"))->trigger();
            if (variant == 0 || variant == 2) QVERIFY(e.settle());            // (the results of the undo come before the redo, or they do not)
            if (variant < 2) {
                e.win.w.act(QStringLiteral("edit.redo"))->trigger();
            } else {
                e.select(QStringLiteral("A1"));
                e.enter(QStringLiteral("5"));                                   // the same number typed again
            }
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:10"));
            QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:11"));
            QVERIFY(e.win.w.isModified());
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY(saved.wait(20000));
            QVERIFY2(saved.first().at(1).toBool(), qPrintable(QString::number(variant)));
            QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:5"));
            QVERIFY2(savedCell(e.path, QStringLiteral("B1")) == QStringLiteral("n:10"), qPrintable(QStringLiteral("variant %1: B1 is %2").arg(variant).arg(savedCell(e.path, QStringLiteral("B1")))));
            QVERIFY2(savedCell(e.path, QStringLiteral("C1")) == QStringLiteral("n:11"), qPrintable(QStringLiteral("variant %1: C1 is %2").arg(variant).arg(savedCell(e.path, QStringLiteral("C1")))));
            // and it opens with the same values
            Window second;
            QVERIFY(second.open(e.path));
            QCOMPARE(second.w.view()->sheet().valueOf(*second.w.view()->sheet().cellAt(0, 2)).debug(), QStringLiteral("n:11"));
        }
    }

    // A cell being edited in the window of a workbook that looks like a real one: its line is over the cell with the cell's formula, the formula bar shows the same, and the line has the accent
    // round it. The picture is saved when JC_SCREENSHOT_EDIT names a file.
    void aCellBeingEditedShowsItsFormulaInTheCellAndTheBar()
    {
        Folder folder;
        Window win;
        win.w.resize(1100, 640);
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        win.w.view()->setZoom(1.0);
        win.w.view()->selectCell(8, 2);   // C9: the total of the second month
        QTest::keyClick(win.w.view(), Qt::Key_F2);
        QVERIFY(win.w.isEditing());
        QApplication::processEvents();
        CellEditor *editor = win.w.cellEditor();
        QVERIFY(editor->isVisible());
        QCOMPARE(editor->text(), QStringLiteral("=SUM(C3:C8)"));
        QCOMPARE(win.w.formulaBar()->text(), QStringLiteral("=SUM(C3:C8)"));
        QCOMPARE(win.w.nameBox()->text(), QStringLiteral("C9"));
        QImage shot = win.w.grab().toImage();
        QVERIFY(!shot.isNull());
        // the line is over C9: the accent runs round it
        const QPoint at = editor->mapTo(&win.w, QPoint(0, 0));
        QCOMPARE(QColor(shot.pixel(at.x() + 1, at.y() + editor->height() / 2)), QColor(0x13, 0x77, 0x3A));
        QCOMPARE(QColor(shot.pixel(at.x() + editor->width() / 2, at.y() + 1)), QColor(0x13, 0x77, 0x3A));
        QCOMPARE(editor->geometry().size(), win.w.view()->cellRect(8, 2).size() + QSize(editor->width() - win.w.view()->cellRect(8, 2).width(), 0));
        const QString out = qEnvironmentVariable("JC_SCREENSHOT_EDIT");
        if (!out.isEmpty()) QVERIFY(shot.save(out));
        QTest::keyClick(editor, Qt::Key_Escape);
    }

    // ---- saving

    // Save writes the workbook as it was read with the edit in it: the edited cell, the formulas that read it with their new results, the new text and format; every part that did not
    // change is the old file's, byte for byte; and the calculation chain is left out.
    void saveWritesWhatChangedAndKeepsTheRest()
    {
        const QByteArray original = editBookWithParts();
        EditWindow e;
        QVERIFY(e.open(original));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("100"));
        e.select(QStringLiteral("F1"));
        e.enter(QStringLiteral("25%"));
        e.select(QStringLiteral("F2"));
        e.enter(QStringLiteral("new text"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.isModified());
        QVERIFY(e.win.w.windowTitle().contains(QLatin1String("[*]")) || e.win.w.isWindowModified());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QCOMPARE(saved.first().at(1).toBool(), true);
        QVERIFY(!e.win.w.isModified());
        QVERIFY(!e.win.w.isSaving());
        QCOMPARE(e.win.told.size(), 0);
        // the file
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray now = f.readAll();
        QVERIFY(now != original);
        const auto before = rawParts(original), after = rawParts(now);
        QVERIFY(!after.contains(QStringLiteral("xl/calcChain.xml")));
        for (auto it = before.constBegin(); it != before.constEnd(); ++it) {
            if (it.key() == QLatin1String("xl/calcChain.xml")) continue;
            QVERIFY2(after.contains(it.key()), qPrintable(it.key()));
            const bool changed = it.key() == QLatin1String("xl/worksheets/sheet1.xml") || it.key() == QLatin1String("xl/sharedStrings.xml") || it.key() == QLatin1String("xl/styles.xml") || it.key() == QLatin1String("[Content_Types].xml")
                || it.key() == QLatin1String("xl/_rels/workbook.xml.rels");
            if (!changed) QVERIFY2(after.value(it.key()) == it.value(), qPrintable(it.key()));
        }
        // read back: the cells, the results, the text, the format
        Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        Workbook b;
        QVERIFY2(b.open(p, &why), qPrintable(why));
        Sheet s;
        QVERIFY(b.readSheet(0, &s));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:100"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:200"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B2")), QStringLiteral("n:110"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B3")), QStringLiteral("n:999"));    // (nothing that was changed is read by it: it is as the file had it)
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("F1")), QStringLiteral("n:0.25"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("F2")), QStringLiteral("s:new text"));
        QCOMPARE(b.styles().numFmtId(at(s, QStringLiteral("F1"))->style), 9);
        QCOMPARE(b.sharedStrings().strings.size(), 2);
        // the formula is still a formula
        QCOMPARE(s.formulaText(*at(s, QStringLiteral("B1"))), QStringLiteral("A1*2"));
        // and it opens in this program, with the same values
        Window second;
        QVERIFY(second.open(e.path));
        QCOMPARE(second.w.view()->sheet().valueOf(*second.w.view()->sheet().cellAt(0, 1)).debug(), QStringLiteral("n:200"));
    }

    // The next save writes what changed since the last, over the file the last wrote; and a workbook that is back where it was saved has nothing to save.
    void aSecondSaveWritesWhatChangedSinceTheFirst()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("6"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.isModified());
        QVERIFY(e.win.w.save());
        QVERIFY(saved.count() == 2 || saved.wait(20000));
        QVERIFY(!e.win.w.isModified());
        {
            Package p;
            QVERIFY(p.readLazyFile(e.path));
            Workbook b;
            QVERIFY(b.open(p));
            Sheet s;
            QVERIFY(b.readSheet(0, &s));
            QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:5"));
            QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A2")), QStringLiteral("n:6"));
            QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B2")), QStringLiteral("n:11"));
        }
        // an edit and its undo is a workbook as it was saved: nothing to save, and Save writes nothing
        const QByteArray bytes = [&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }();
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("9"));
        QVERIFY(e.win.w.isModified());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(QTest::qWaitFor([&] { return !e.win.w.isCalculating(); }, 20000));
        QVERIFY(!e.win.w.isModified());
        QVERIFY(e.win.w.save());
        QCOMPARE(saved.count(), 3);   // (said at once)
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(f.readAll() == bytes);
        QVERIFY(e.win.w.statusText().contains(QLatin1String("Nothing to save")));
    }

    void saveAsWritesAnotherFileAndTheWindowIsThatFile()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("3"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        const QString other = e.folder.dir.filePath(QStringLiteral("copy"));   // (no ending: it is given one)
        QVERIFY(e.win.w.saveAs(other));
        QVERIFY(saved.wait(20000));
        QCOMPARE(saved.first().at(0).toString(), other + QStringLiteral(".xlsx"));
        QVERIFY(QFileInfo::exists(other + QStringLiteral(".xlsx")));
        QCOMPARE(e.win.w.filePath(), other + QStringLiteral(".xlsx"));
        // the first file is as it was
        Package p;
        QVERIFY(p.readLazyFile(e.path));
        Workbook b;
        QVERIFY(b.open(p));
        Sheet s;
        QVERIFY(b.readSheet(0, &s));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:1"));
        // a wrong ending is refused
        QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("x.xlsm"))));
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY(e.win.told[0].contains(QLatin1String(".xlsx")));
    }

    // A save that fails says why and leaves the file that was there as it was.
    void aSaveThatFailsLeavesTheOldFile()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray before = f.readAll();
        f.close();
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("3"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("no/such/folder/book.xlsx"))));
        QVERIFY(saved.wait(20000));
        QCOMPARE(saved.first().at(1).toBool(), false);
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].startsWith(QLatin1String("Save: ")) && e.win.told[0].contains(QLatin1String("couldn't save")), qPrintable(e.win.told[0]));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(f.readAll() == before);
        QVERIFY(e.win.w.isModified());                  // (still to be saved)
        QVERIFY(!e.win.w.isSaving());
        QCOMPARE(e.win.w.filePath(), e.path);
        // and it can be edited and saved again
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("4"));
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QCOMPARE(saved.last().at(1).toBool(), true);
    }

    // A window with changes that are not saved asks before it closes: Cancel keeps it, Don't Save closes it, Save saves and then closes. One without changes closes.
    void closingAsksAboutChangesThatAreNotSaved()
    {
        auto make = [](EditWindow &e, CrunchWindow::Answer answer, int *asked) {
            e.win.w.setAsker([answer, asked](const QString &, const QString &text) {
                ++*asked;
                return text.contains(QLatin1String("book.xlsx")) ? answer : CrunchWindow::Answer::Cancel;
            });
        };
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            int asked = 0;
            make(e, CrunchWindow::Answer::Cancel, &asked);
            QVERIFY(e.win.w.close());          // nothing changed: closes without asking
            QCOMPARE(asked, 0);
        }
        for (const auto answer : {CrunchWindow::Answer::Cancel, CrunchWindow::Answer::Discard, CrunchWindow::Answer::Save}) {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            int asked = 0;
            make(e, answer, &asked);
            e.win.w.setAttribute(Qt::WA_DeleteOnClose, false);
            e.select(QStringLiteral("A1"));
            e.enter(QStringLiteral("8"));
            QVERIFY(e.settle());
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            e.win.w.close();
            QCOMPARE(asked, 1);
            if (answer == CrunchWindow::Answer::Cancel) {
                QVERIFY(e.win.w.isVisible());
                QVERIFY(e.win.w.isModified());
            } else if (answer == CrunchWindow::Answer::Discard) {
                QVERIFY(!e.win.w.isVisible());
            } else {
                QVERIFY(saved.wait(20000));
                QTRY_VERIFY(!e.win.w.isVisible());
                Package p;
                QVERIFY(p.readLazyFile(e.path));
                Workbook b;
                QVERIFY(b.open(p));
                Sheet s;
                QVERIFY(b.readSheet(0, &s));
                QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:8"));
            }
        }
    }

    // A save asked for while the formulas are still being worked out waits until they are: the file has their results.
    void saveWaitsForTheCalculation()
    {
        const int n = 20000;
        QString rows;
        rows.reserve(n * 90);
        for (int r = 1; r <= n; ++r)
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.select(QStringLiteral("D1"));
        e.win.w.setWorkerPace(3000);
        e.enter(QStringLiteral("1"));
        QVERIFY(e.win.w.isCalculating());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(e.win.w.isCalculating());
        QCOMPARE(saved.count(), 0);
        QVERIFY(e.win.w.statusText().contains(QLatin1String("calculation")));
        e.win.w.setWorkerPace(0);
        QVERIFY(saved.wait(60000));
        QCOMPARE(saved.first().at(1).toBool(), true);
        Package p;
        QVERIFY(p.readLazyFile(e.path));
        Workbook b;
        QVERIFY(b.open(p));
        Sheet s;
        QVERIFY(b.readSheet(0, &s));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:3"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B%1").arg(n)), QStringLiteral("n:%1").arg(2 * n + 1));
    }

    // The first cell of a shared formula typed over, and the workbook saved: the cells that shared its formula keep theirs (each written out), with the results the worker gave them.
    void aSharedFormulaSurvivesItsFirstCellBeingTypedOver()
    {
        QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"shared\" ref=\"B1:B4\" si=\"0\">A1*2</f><v>2</v></c></row>");
        for (int r = 2; r <= 4; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f t=\"shared\" si=\"0\"/><v>%2</v></c></row>").arg(r).arg(r * 2);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("99"));
        e.select(QStringLiteral("A3"));
        e.enter(QStringLiteral("30"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QCOMPARE(saved.first().at(1).toBool(), true);
        Package p;
        QVERIFY(p.readLazyFile(e.path));
        Workbook b;
        QVERIFY(b.open(p));
        Sheet s;
        QVERIFY(b.readSheet(0, &s));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:99"));
        QVERIFY(!at(s, QStringLiteral("B1"))->hasFormula());
        for (const int r : {2, 3, 4}) {
            const Cell *c = at(s, QStringLiteral("B%1").arg(r));
            QVERIFY(c && c->hasFormula());
            QCOMPARE(s.formulaText(*c), QStringLiteral("A%1*2").arg(r));
        }
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B3")), QStringLiteral("n:60"));
        QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B2")), QStringLiteral("n:4"));
    }

    // Only the results the edit changed are written: a formula that is worked out again with the value the engine gave it before keeps the result the file stored (the engine's differences from
    // the program that wrote the file are not made into changes).
    void onlyTheResultsTheEditChangedAreWritten()
    {
        // A1 is 1; B1 = A1*0 and C1 = A1*2, both stored as 999 (which the engine does not give)
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*0</f><v>999</v></c><c r=\"C1\"><f>A1*2</f><v>999</v></c></row>"))});
        EditWindow e;
        QVERIFY(e.open(book));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:2"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        auto savedValue = [&](const QString &ref) -> QString {
            Package p;
            Workbook b;
            Sheet s;
            if (!p.readLazyFile(e.path) || !b.open(p) || !b.readSheet(0, &s)) return QStringLiteral("the file does not open");
            return show(s, b.sharedStrings(), ref);
        };
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:10"));
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QCOMPARE(savedValue(QStringLiteral("A1")), QStringLiteral("n:5"));
        QCOMPARE(savedValue(QStringLiteral("C1")), QStringLiteral("n:10"));    // the engine changed it
        QCOMPARE(savedValue(QStringLiteral("B1")), QStringLiteral("n:999"));   // worked out again, and the same (0): the file's own stays
        // typed back: C1 is what the engine says again (2), B1 still the file's
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("1"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.save());
        QVERIFY(saved.count() == 2 || saved.wait(20000));
        QCOMPARE(savedValue(QStringLiteral("C1")), QStringLiteral("n:2"));
        QCOMPARE(savedValue(QStringLiteral("B1")), QStringLiteral("n:999"));
    }

    // A workbook with macros is saved as a .xlsm with its macros as they were, and cannot be given another ending.
    void aWorkbookWithMacrosKeepsThem()
    {
        QByteArray macroBook;
        {
            // the editBook, with a macro-enabled main part and a VBA project
            const QByteArray plain = editBook();
            QMap<QString, QByteArray> parts;
            QVERIFY(readZip(plain, parts));
            ZipWriter z;
            for (auto it = parts.begin(); it != parts.end(); ++it) {
                QByteArray data = it.value();
                if (it.key() == QLatin1String("[Content_Types].xml"))
                    data.replace("spreadsheetml.sheet.main+xml", "ms-excel.sheet.macroEnabled.main+xml").replace("</Types>", "<Override PartName=\"/xl/vbaProject.bin\" ContentType=\"application/vnd.ms-office.vbaProject\"/></Types>");
                z.add(it.key(), data, true);
            }
            z.add(QStringLiteral("xl/vbaProject.bin"), QByteArray("VBA project bytes \x01\x02\x03 that must not change", 40), true);
            macroBook = z.finish();
        }
        EditWindow e;
        QVERIFY(e.open(macroBook, QStringLiteral("macros.xlsm")));
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("2"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("macros.xlsx"))));
        QVERIFY(e.win.told.last().contains(QLatin1String(".xlsm")));
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray now = f.readAll();
        QCOMPARE(rawParts(now).value(QStringLiteral("xl/vbaProject.bin")), rawParts(macroBook).value(QStringLiteral("xl/vbaProject.bin")));
        QVERIFY(entriesOf(now).value(QStringLiteral("[Content_Types].xml")).contains("macroEnabled"));
    }

private:
    // The formula of a cell of a saved file, as the file holds it.
    static QString savedFormula(const QString &path, const QString &ref, int sheet = 0)
    {
        Package p;
        QString why;
        if (!p.readLazyFile(path, &why)) return why;
        Workbook b;
        if (!b.open(p, &why)) return why;
        Sheet s;
        if (!b.readSheet(sheet, &s)) return QStringLiteral("unreadable");
        const Cell *c = at(s, ref);
        return c ? s.formulaText(*c) : QStringLiteral("-");
    }
    // The cell of a saved file, as the file holds it.
    static QString savedCell(const QString &path, const QString &ref, int sheet = 0)
    {
        Package p;
        QString why;
        if (!p.readLazyFile(path, &why)) return why;
        Workbook b;
        if (!b.open(p, &why)) return why;
        Sheet s;
        if (!b.readSheet(sheet, &s)) return QStringLiteral("unreadable");
        return show(s, b.sharedStrings(), ref);
    }
    // A workbook like a real one's package: the editBook, a calculation chain, a theme, and a part nothing reads.
    static QByteArray editBookWithParts()
    {
        const QByteArray plain = editBook();
        QMap<QString, QByteArray> parts;
        readZip(plain, parts);
        ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            QByteArray data = it.value();
            if (it.key() == QLatin1String("[Content_Types].xml"))
                data.replace("</Types>", "<Override PartName=\"/xl/calcChain.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml\"/></Types>");
            if (it.key() == QLatin1String("xl/_rels/workbook.xml.rels"))
                data.replace("</Relationships>", "<Relationship Id=\"rId90\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/calcChain\" Target=\"calcChain.xml\"/></Relationships>");
            z.add(it.key(), data, true);
        }
        z.add(QStringLiteral("xl/calcChain.xml"), QByteArray("<calcChain xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><c r=\"B1\" i=\"1\"/></calcChain>"), true);
        z.add(QStringLiteral("xl/theme/theme1.xml"), QByteArray("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>"), true);
        z.add(QStringLiteral("customXml/item1.xml"), QByteArray("<x>not read here</x>"), true);
        return z.finish();
    }
    static QMap<QString, QByteArray> entriesOf(const QByteArray &zip)
    {
        QMap<QString, QByteArray> out;
        readZip(zip, out);
        return out;
    }
    static QMap<QString, QByteArray> rawParts(const QByteArray &zip)
    {
        QMap<QString, QByteArray> out;
        QVector<ZipEntry> dir;
        if (readZipDirectory(zip, &dir))
            for (const ZipEntry &e : dir) out.insert(e.name, zip.mid(int(e.dataAt), int(e.csize)));
        return out;
    }
};

JP_TEST_CLASS(CrunchEditTests)
#include "tst_crunch_edit.moc"
