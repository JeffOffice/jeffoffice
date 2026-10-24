// Tests of rows, columns, and cells inserted and deleted in an open workbook: the cells and what refers to them move, the values the window holds move with them, one step undoes it, and the
// keys and menus ask for it. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "../sheet/richbook.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QApplication>
#include <QMenu>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

bool sameValue(const calc::Value &a, const calc::Value &b)
{
    if (a.isNumber() && b.isNumber()) return calc::sameTo15Digits(a.number(), b.number());
    if ((a.isEmpty() && b.isNumber() && b.number() == 0) || (b.isEmpty() && a.isNumber() && a.number() == 0)) return true;
    return a == b;
}

// What two calculations of the same workbook differ in: the cells (what they hold, their formats, and their values), the merged ranges, and the names. Empty if they agree.
QString differences(const office::sheet::Calculation *a, const office::sheet::Calculation *b, const QStringList &names)
{
    using office::sheet::Calculation;
    QStringList out;
    if (a->sheetCount() != b->sheetCount()) return QStringLiteral("the number of sheets");
    auto empty = [](const Calculation::CellContent &c) { return !c.exists || (!c.isFormula && c.value.isEmpty() && c.style == 0); };
    // (formulas in a circle have no value to agree on, and what reads them neither: the values are left out then)
    bool circular = false;
    for (int s = 0; s < a->sheetCount(); ++s)
        a->sheet(s).cells.forEach([&](const office::sheet::Cell &c) { circular = circular || a->isCircular(s, c.row, c.col); });
    for (int s = 0; s < a->sheetCount() && out.size() < 12; ++s) {
        auto compareAt = [&](quint32 row, quint32 col) {
            const Calculation::CellContent x = a->content(s, row, col), y = b->content(s, row, col);
            const QString where = QStringLiteral("%1!%2").arg(s).arg(cellName(row, col));
            if (empty(x) && empty(y)) return;
            // (a cell of an array holds the result the file had, which is no part of what it means: what it shows is compared below)
            const bool member = !x.editable && !x.isFormula;
            if (empty(x) != empty(y) || x.isFormula != y.isFormula || x.formula != y.formula || (!member && !sameValue(x.value, y.value)) || x.style != y.style) {
                out << QStringLiteral("%1 holds %2 / %3 (%4) against %5 / %6 (%7)").arg(where, x.isFormula ? x.formula : x.value.debug(), QString::number(x.style), empty(x) ? "none" : "cell",
                                                                                         y.isFormula ? y.formula : y.value.debug(), QString::number(y.style), empty(y) ? "none" : "cell");
                return;
            }
            if (!circular && !sameValue(a->cell(s, row, col), b->cell(s, row, col))) out << QStringLiteral("%1 shows %2 against %3").arg(where, a->cell(s, row, col).debug(), b->cell(s, row, col).debug());
        };
        a->sheet(s).cells.forEach([&](const office::sheet::Cell &c) { compareAt(c.row, c.col); });
        b->sheet(s).cells.forEach([&](const office::sheet::Cell &c) { compareAt(c.row, c.col); });
        QVector<office::sheet::CellRange> ma = a->sheet(s).merges, mb = b->sheet(s).merges;
        auto less = [](const office::sheet::CellRange &p, const office::sheet::CellRange &q) { return p.row1 != q.row1 ? p.row1 < q.row1 : p.col1 < q.col1; };
        std::sort(ma.begin(), ma.end(), less);
        std::sort(mb.begin(), mb.end(), less);
        if (!(ma == mb)) {
            QStringList x, y;
            for (const auto &r : ma) x << rangeName(r);
            for (const auto &r : mb) y << rangeName(r);
            out << QStringLiteral("merges %1 against %2").arg(x.join(' '), y.join(' '));
        }
        // the widths of columns and the heights of rows
        for (quint32 i = 0; i < 24; ++i) {
            const auto x = a->columnRuns(s, i, i), y = b->columnRuns(s, i, i);
            const double wx = x.empty() ? -1 : x.front().width, wy = y.empty() ? -1 : y.front().width;
            const bool hx = !x.empty() && x.front().hidden, hy = !y.empty() && y.front().hidden;
            if (std::abs(wx - wy) > 0.01 || hx != hy) out << QStringLiteral("column %1 of sheet %2 is %3%4 against %5%6").arg(i).arg(s).arg(wx).arg(hx ? " hidden" : "").arg(wy).arg(hy ? " hidden" : "");
        }
        for (quint32 i = 0; i < 40; ++i) {
            const auto x = a->rowSpecsIn(s, i, i), y = b->rowSpecsIn(s, i, i);
            // (a row with the sheet's own height is no different from one with none: the file does not keep it)
            const double normal = a->sheet(s).format.defaultRowHeight;
            double hx = x.empty() ? -1 : x.front().height, hy = y.empty() ? -1 : y.front().height;
            if (std::abs(hx - normal) < 0.001) hx = -1;
            if (std::abs(hy - normal) < 0.001) hy = -1;
            const bool vx = !x.empty() && x.front().hidden, vy = !y.empty() && y.front().hidden;
            if (std::abs(hx - hy) > 0.01 || vx != vy) out << QStringLiteral("row %1 of sheet %2 is %3%4 against %5%6").arg(i).arg(s).arg(hx).arg(vx ? " hidden" : "").arg(hy).arg(vy ? " hidden" : "");
        }
        for (const QString &name : names)
            if (a->definedName(s, name) != b->definedName(s, name)) out << QStringLiteral("name %1: %2 against %3").arg(name, a->definedName(s, name), b->definedName(s, name));
    }
    return out.join(QStringLiteral("; "));
}

// What the window holds for the formulas (its store) against what the calculation has: the grid shows the first. Empty if they agree.
QString storeDiffers(EditWindow &e)
{
    // (the edits were made through the session, not the window's keys: the worker is waited for here)
    auto &loader = e.win.w.workbookLoader();
    QTest::qWait(1);
    QElapsedTimer waited;
    waited.start();
    while (!loader.result()->calcIdle.load() && waited.elapsed() < 20000) QTest::qWait(2);
    QTest::qWait(5);
    const office::sheet::Calculation *calc = e.win.w.calculation();
    for (int s = 0; s < calc->sheetCount(); ++s) {
        bool circular = false;
        calc->sheet(s).cells.forEach([&](const office::sheet::Cell &c) { circular = circular || calc->isCircular(s, c.row, c.col); });
        if (circular) continue;
        QString problem;
        calc->sheet(s).cells.forEach([&](const office::sheet::Cell &c) {
            if (!problem.isEmpty() || !(c.hasFormula() || (c.flags & office::sheet::ArrayMember))) return;
            calc::Value held;
            if (e.win.w.values().find(s, c.row, c.col, &held) && !sameValue(held, calc->cell(s, c.row, c.col)))
                problem = QStringLiteral("%1!%2 shows %3 and is %4").arg(s).arg(cellName(c.row, c.col), held.debug(), calc->cell(s, c.row, c.col).debug());
        });
        if (!problem.isEmpty()) return problem;
    }
    return QString();
}

QString savedSheetText(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
}

QString formulaOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->editText(0, r, c);
}

} // namespace

class CrunchCellsTests : public QObject {
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

    // A row inserted at the selected row: the cells below move down, the formulas that read them follow, a range with the row inside grows, the values the window holds move with their cells
    // (before anything is worked out again), and one step of Undo puts everything back.
    void insertingARowMovesTheCellsAndUndoPutsThemBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==SUM(A1:A3)", "B2==A3*2", "C3==B1+B2"})));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:6"));
        QAction *insert = e.win.w.act(QStringLiteral("cells.insert"));
        QVERIFY(insert && insert->isEnabled());
        e.view()->selectRows(1, 1);                                              // row 2
        insert->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting a row"));
        QVERIFY(e.win.w.isModified());
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:3"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A4)"));
        QCOMPARE(formulaOf(e, QStringLiteral("B3")), QStringLiteral("=A4*2"));
        QCOMPARE(formulaOf(e, QStringLiteral("C4")), QStringLiteral("=B1+B3"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:12"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("-"));
        // a value typed under the new row changes what reads it
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("10"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:16"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:22"));
        pressUndo(&e.win.w);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), 0);
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:2"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A3)"));
        QCOMPARE(formulaOf(e, QStringLiteral("B2")), QStringLiteral("=A3*2"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:12"));
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:12"));
    }

    // Rows and columns deleted: what read them has #REF!, and Undo gives the cells and the formulas back.
    void deletingRowsAndColumnsMakesRefAndUndoGivesThemBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==A2*10", "C1==SUM(A1:A3)", "C2==B1+1", "D3==A3"})));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:6"));
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("cells.delete"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting a row"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=#REF!*10"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("e:#REF!"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A2)"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("-"));         // (C2 was in the deleted row)
        QCOMPARE(formulaOf(e, QStringLiteral("D2")), QStringLiteral("=A2"));     // (D3 moved up to D2)
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A2*10"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:20"));
        QCOMPARE(formulaOf(e, QStringLiteral("C2")), QStringLiteral("=B1+1"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:21"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:2"));
        // a column
        e.view()->selectColumns(1, 1);                                           // column B
        e.win.w.act(QStringLiteral("cells.delete"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting a column"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A3)"));   // (C1 moved left)
        QCOMPARE(formulaOf(e, QStringLiteral("B2")), QStringLiteral("=#REF!+1"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("C2")), QStringLiteral("=B1+1"));
        QVERIFY(!e.win.w.isModified());
    }

    // Cells inserted and deleted, shifted down, right, left, or up: asked of the person, who can also choose whole rows or columns.
    void cellsAreShiftedTheWayThePersonChooses()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=10", "B2=20", "C1==SUM(A1:A3)", "C2==B2"})));
        using Way = CrunchWindow::ShiftRequest::Way;
        Way answer = Way::Down;
        bool inserting = true;
        e.win.w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            inserting = r->inserting;
            r->way = answer;
            return true;
        });
        e.view()->selectRange(1, 0, 1, 0);                                       // A2
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(inserting);
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("1||2|3"));
        QCOMPARE(shown(e, QStringLiteral("B1:B2")), QStringLiteral("10|20"));    // (the other columns stay)
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A4)"));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting cells"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        // right
        answer = Way::Right;
        e.view()->selectRange(0, 1, 0, 1);                                       // B1
        e.win.w.act(QStringLiteral("cells.insert.cells"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("1||10|6"));
        QCOMPARE(formulaOf(e, QStringLiteral("D1")), QStringLiteral("=SUM(A1:A3)"));
        pressUndo(&e.win.w);
        // delete: up and left
        answer = Way::Up;
        e.view()->selectRange(0, 0, 0, 0);                                       // A1
        e.win.w.act(QStringLiteral("cells.delete.cells"))->trigger();
        QVERIFY(e.settle());
        QVERIFY(!inserting);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("2|3|"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A2)"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        answer = Way::Left;
        e.view()->selectRange(0, 0, 1, 0);                                       // A1:A2
        e.win.w.act(QStringLiteral("cells.delete.cells"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:B2")), QStringLiteral("10|33|20|20"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=SUM(A1:A3)"));   // (a range across other rows stays as it is)
        QCOMPARE(formulaOf(e, QStringLiteral("B2")), QStringLiteral("=A2"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=SUM(A1:A3)"));
        // a whole row chosen in the question
        answer = Way::Rows;
        e.view()->selectRange(1, 0, 1, 0);
        e.win.w.act(QStringLiteral("cells.delete.cells"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting a row"));
        QCOMPARE(shown(e, QStringLiteral("A1:A2")), QStringLiteral("1|3"));
        // giving it up changes nothing
        const int steps = e.win.w.undoStack()->count();
        e.win.w.setShiftAsker([](CrunchWindow::ShiftRequest *) { return false; });
        e.win.w.act(QStringLiteral("cells.insert.cells"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }

    // Ctrl++ and Ctrl+- (the keys the action has) insert and delete the selected rows or columns; the right-click menu of a header has them.
    void theKeysAndTheMenusAskForIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==A2"})));
        QAction *insert = e.win.w.act(QStringLiteral("cells.insert")), *del = e.win.w.act(QStringLiteral("cells.delete"));
        QCOMPARE(insert->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_Plus).toString(QKeySequence::NativeText));
        QCOMPARE(del->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_Minus).toString(QKeySequence::NativeText));
        e.view()->selectRows(0, 1);                                              // rows 1 and 2
        const QKeyCombination plus = insert->shortcut()[0];
        QTest::keyClick(e.view(), plus.key(), plus.keyboardModifiers());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting rows"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:1"));
        QCOMPARE(formulaOf(e, QStringLiteral("B3")), QStringLiteral("=A4"));
        const QKeyCombination minus = del->shortcut()[0];
        QTest::keyClick(e.view(), minus.key(), minus.keyboardModifiers());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Deleting rows"));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        // a column
        e.view()->selectColumns(0, 0);
        QTest::keyClick(e.view(), plus.key(), plus.keyboardModifiers());
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Inserting a column"));
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:1"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=B2"));
        // the menu of a header: a right click on the number of a row
        const QRect cell = e.view()->cellRect(2, 1);
        const QPoint header(e.view()->viewport()->x() + 10, cell.center().y());
        QTest::mouseClick(e.view()->viewport(), Qt::RightButton, Qt::NoModifier, QPoint(10, cell.center().y()));
        QMenu *menu = e.win.w.findChild<QMenu *>(QStringLiteral("cellMenu"));
        QVERIFY(menu);
        QStringList texts;
        for (QAction *a : menu->actions()) texts << a->text();
        QVERIFY2(texts.contains(QStringLiteral("Insert Sheet Rows")) && texts.contains(QStringLiteral("Delete Sheet Rows")), qPrintable(texts.join('|')));
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::Rows);
        QCOMPARE(e.view()->selection().range.row1, qint64(2));
        menu->close();
        Q_UNUSED(header);
    }

    // What cannot be done is not: cutting through an array formula, a protected sheet; the person is told, and nothing changes.
    void whatCannotBeDoneIsRefused()
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("array")), qPrintable(e.win.told.join('|')));
        // above the array it moves whole
        e.view()->selectRows(0, 0);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:6"));
        // and it still works out from what it reads
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("10"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:20"));
    }

    // The file that is saved after inserting and deleting means what the window shows: opened again, every cell, formula, value, merge, and name is what it was (the sheets' parts, the
    // names, and the charts, drawings, and notes follow; what the edits did not touch is as it was).
    void theSavedFileMeansWhatTheWindowShows()
    {
        EditWindow e;
        QVERIFY(e.open(richBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 2, 1).ok);
        QVERIFY(s->deleteColumns(0, 3, 1).ok);                                   // column D (the sum is among its cells)
        QVERIFY(s->insertColumns(0, 0, 2).ok);
        QVERIFY(s->deleteRows(0, 5, 2).ok);
        QVERIFY(e.settle());
        // (the check sees a difference: the window is not what the file was)
        {
            Window original;
            QVERIFY(original.open(e.folder.write(QStringLiteral("original.xlsx"), richBook())));
            QVERIFY(!differences(e.win.w.calculation(), original.w.calculation(), {"Total"}).isEmpty());
        }
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const QString diff = differences(e.win.w.calculation(), second.w.calculation(), {"Total", "_xlnm.Print_Area"});
        QVERIFY2(diff.isEmpty(), qPrintable(diff));
        QCOMPARE(second.w.calculation()->definedName(0, QStringLiteral("Total")), e.win.w.calculation()->definedName(0, QStringLiteral("Total")));
        // Undo after the save goes back no further than the moves it holds
        QVERIFY(!e.win.w.isModified());
        QVERIFY(!e.win.w.session()->canUndo());
    }

    // Many edits at random (rows, columns, and cells inserted and deleted on both sheets, with values and formulas typed between, some undone and done again), then saved and opened again:
    // the file always means what the window showed.
    void randomEditsSaveWhatTheWindowShows()
    {
        int moves = 0, refused = 0, pastes = 0, cuts = 0;
        // (CRUNCH_SEED=n runs only that seed, and says what it does)
        const quint32 only = qEnvironmentVariableIntValue("CRUNCH_SEED");
        const quint32 seeds = qEnvironmentVariableIsSet("CRUNCH_SEEDS") ? qEnvironmentVariableIntValue("CRUNCH_SEEDS") : 40;
        for (quint32 seed = only ? only : 1; seed <= (only ? only : seeds); ++seed) {
            QRandomGenerator rng(seed);
            EditWindow e;
            QVERIFY(e.open(richBook()));
            crunch::EditSession *s = e.win.w.session();
            for (int step = 0; step < 10; ++step) {
                const int sheet = rng.bounded(3) == 0 ? 1 : 0;
                const quint32 at = quint32(rng.bounded(9)), count = 1 + quint32(rng.bounded(2));
                office::sheet::CellRange block;
                block.valid = true;
                block.row1 = at;
                block.row2 = at + count - 1;
                block.col1 = quint32(rng.bounded(5));
                block.col2 = block.col1 + quint32(rng.bounded(2));
                const int which = rng.bounded(17);
                if (only) qInfo() << "step" << step << "which" << which << "sheet" << sheet << "at" << at << "count" << count << "block" << rangeName(block);
                switch (which) {
                case 0: (void((s->insertRows(sheet, at, count)).ok ? ++moves : ++refused)); break;
                case 1: (void((s->deleteRows(sheet, at, count)).ok ? ++moves : ++refused)); break;
                case 2: (void((s->insertColumns(sheet, at % 6, count)).ok ? ++moves : ++refused)); break;
                case 3: (void((s->deleteColumns(sheet, at % 6, count)).ok ? ++moves : ++refused)); break;
                case 4: (void((s->insertCells(sheet, block, true)).ok ? ++moves : ++refused)); break;
                case 5: (void((s->deleteCells(sheet, block, true)).ok ? ++moves : ++refused)); break;
                case 6: (void((s->insertCells(sheet, block, false)).ok ? ++moves : ++refused)); break;
                case 7: (void((s->deleteCells(sheet, block, false)).ok ? ++moves : ++refused)); break;
                case 8: s->type(sheet, quint32(rng.bounded(10)), quint32(rng.bounded(6)), QString::number(rng.bounded(100))); break;
                case 11: {
                    crunch::EditSession::SizeChange by;
                    by.setSize = true;
                    by.size = 4 + rng.bounded(30);
                    s->resizeColumns(sheet, at % 8, at % 8 + rng.bounded(3), by, QStringLiteral("w"));
                    break;
                }
                case 12: {
                    crunch::EditSession::SizeChange by;
                    if (rng.bounded(2)) {
                        by.setSize = true;
                        by.size = 10 + rng.bounded(40);
                    } else {
                        by.setHidden = true;
                        by.hidden = rng.bounded(2);
                    }
                    s->resizeRows(sheet, at, at + rng.bounded(3), by, QStringLiteral("h"));
                    break;
                }
                case 14: {
                    crunch::EditSession::PasteOptions o;
                    o.what = crunch::EditSession::PasteOptions::What(rng.bounded(4));
                    o.transpose = rng.bounded(4) == 0;
                    o.skipBlanks = rng.bounded(4) == 0;
                    if (rng.bounded(5) == 0) o.operation = crunch::EditSession::PasteOptions::Operation(1 + rng.bounded(4));
                    office::sheet::CellRange at1;
                    at1.valid = true;
                    at1.row1 = at1.row2 = quint32(rng.bounded(10));
                    at1.col1 = at1.col2 = quint32(rng.bounded(6));
                    (void((s->paste(sheet, at1, s->copyCells(sheet, block, false), o)).ok ? ++pastes : ++refused));
                    break;
                }
                case 15: {
                    const int toSheet = rng.bounded(3) == 0 ? 1 - sheet : sheet;
                    const quint32 toRow = quint32(rng.bounded(10)), toCol = quint32(rng.bounded(6));
                    const auto outcome = s->moveCells(sheet, block, toSheet, toRow, toCol);
                    if (only) qInfo() << "  move to" << toSheet << cellName(toRow, toCol) << outcome.ok << outcome.problem;
                    (void(outcome.ok ? ++cuts : ++refused));
                    break;
                }
                case 16: {
                    office::sheet::CellRange at1;
                    at1.valid = true;
                    at1.row1 = at1.row2 = quint32(rng.bounded(10));
                    at1.col1 = at1.col2 = quint32(rng.bounded(6));
                    QVector<QStringList> table{{QStringLiteral("7"), QStringLiteral("x"), QStringLiteral("=A1+1")}, {QStringLiteral("2024-03-04"), QString(), QStringLiteral("3.5")}};
                    (void((s->pasteText(sheet, at1, table, crunch::EditSession::PasteOptions())).ok ? ++pastes : ++refused));
                    break;
                }
                case 13: s->type(sheet, quint32(rng.bounded(10)), 0, QStringLiteral("123456789012345")); break;
                case 9: s->type(sheet, quint32(rng.bounded(10)), quint32(rng.bounded(6)), QStringLiteral("=Sheet1!A%1+Sheet2!A1").arg(1 + rng.bounded(6))); break;
                default:
                    if (s->canUndo()) pressUndo(&e.win.w);
                    break;
                }
                QVERIFY(e.settle());
                const QString stale = storeDiffers(e);
                QVERIFY2(stale.isEmpty(), qPrintable(QStringLiteral("seed %1 step %2: %3").arg(seed).arg(step).arg(stale)));
            }
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            const bool modified = e.win.w.isModified();
            QVERIFY(e.win.w.save() || !modified);
            if (modified) QVERIFY(saved.wait(20000));
            Window second;
            QVERIFY(second.open(e.copyOfFile()));
            const QString diff = differences(e.win.w.calculation(), second.w.calculation(), {"Total", "_xlnm.Print_Area"});
            if (only) {
                QMap<QString, QByteArray> parts;
                QFile f(e.path);
                if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
                qInfo().noquote() << QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
            }
            QVERIFY2(diff.isEmpty(), qPrintable(QStringLiteral("seed %1: %2").arg(seed).arg(diff)));
        }
        QVERIFY2(only || (moves > 25 && refused < moves + pastes + cuts && (seeds < 20 || (pastes > seeds / 2 && cuts > seeds / 4))), qPrintable(QStringLiteral("%1 inserts and deletes, %2 pastes, %3 cuts, %4 refused").arg(moves).arg(pastes).arg(cuts).arg(refused)));
    }

    // Tables: rows inserted inside one make it longer, rows or columns before it move it, the header row and the columns can't be cut through; the saved part has its place.
    void tablesMoveWithTheirCellsAndRefuseToBeCut()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Q1</t></is></c><c r=\"C1\" t=\"inlineStr\"><is><t>Total</t></is></c><c r=\"E1\"><f>SUM(Sales[Q1])</f><v>6</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>East</t></is></c><c r=\"B2\"><v>1</v></c><c r=\"C2\"><f>Sales[[#This Row],[Q1]]*2</f><v>2</v></c></row>"
                                            "<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>West</t></is></c><c r=\"B3\"><v>2</v></c><c r=\"C3\"><f>Sales[[#This Row],[Q1]]*2</f><v>4</v></c></row>"
                                            "<row r=\"4\"><c r=\"A4\" t=\"inlineStr\"><is><t>North</t></is></c><c r=\"B4\"><v>3</v></c><c r=\"C4\"><f>Sales[[#This Row],[Q1]]*2</f><v>6</v></c></row>");
        const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:C4\">"
                                             "<autoFilter ref=\"A1:C4\"/><tableColumns count=\"3\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/>"
                                             "<tableColumn id=\"3\" name=\"Total\"><calculatedColumnFormula>Sales[[#This Row],[Q1]]*2</calculatedColumnFormula></tableColumn></tableColumns></table>");
        const QByteArray book = withTables(buildXlsx({worksheet(rows)}), {table});
        auto tableRef = [](const QString &path) {
            QFile f(path);
            f.open(QIODevice::ReadOnly);
            QMap<QString, QByteArray> parts;
            jp::readZip(f.readAll(), parts);
            const QString xml = QString::fromUtf8(parts.value(QStringLiteral("xl/tables/table1.xml")));
            const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("<table\\b[^>]* ref=\"([^\"]*)\"")).match(xml);
            const QRegularExpressionMatch f2 = QRegularExpression(QStringLiteral("<autoFilter ref=\"([^\"]*)\"")).match(xml);
            return m.captured(1) + QLatin1Char('|') + f2.captured(1);
        };
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        auto saveNow = [&] {
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY(saved.wait(20000));
        };
        // refused: the header row, all of the table's rows, a column inside it, a block that takes part of it with it
        QVERIFY(!s->deleteRows(0, 0, 1).ok);
        QVERIFY(!s->deleteRows(0, 0, 4).ok);
        QVERIFY(!s->insertColumns(0, 1, 1).ok);
        QVERIFY(!s->deleteColumns(0, 1, 1).ok);
        office::sheet::CellRange part;
        part.valid = true;
        part.row1 = 2;
        part.row2 = 2;
        part.col1 = 0;
        part.col2 = 1;
        QVERIFY(!s->insertCells(0, part, true).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        // longer: a row inserted in the middle, then a row deleted from the end
        QVERIFY(s->insertRows(0, 2, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("E1")), QStringLiteral("=SUM(Sales[Q1])"));
        saveNow();
        QCOMPARE(tableRef(e.path), QStringLiteral("A1:C5|A1:C5"));
        // the table is where the rows are: a header typed over renames its column
        QVERIFY(s->insertRows(0, 0, 2).ok);                                       // before the table: it moves down
        QVERIFY(e.settle());
        QVERIFY(s->type(0, 2, 1, QStringLiteral("Quarter")).ok);                  // B3, the header of the second column
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("E3")), QStringLiteral("=SUM(Sales[Quarter])"));
        saveNow();
        QCOMPARE(tableRef(e.path), QStringLiteral("A3:C7|A3:C7"));
        // and the table is a table in the file written
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.session()->editText(0, 2, 1), QStringLiteral("Quarter"));
        QCOMPARE(differences(e.win.w.calculation(), second.w.calculation(), {}), QString());
        // Undo goes back as far as the last move saved: over the renaming, which came after it
        QVERIFY(s->canUndo());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();                    // (the other window has the keys now)
        QVERIFY(!s->canUndo());
    }

    // A table column renamed in its header: the formulas of cells, the defined names, the rules of validation and of conditional formats, and charts that name it follow, in the
    // model and in the saved file; Undo (even after a save) puts the old name back everywhere.
    void aRenamedColumnFollowsInNamesRulesAndCharts()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Q1</t></is></c><c r=\"E1\"><f>SUM(Sales[Q1])</f><v>3</v></c><c r=\"F1\"><f>Q1Total*2</f><v>6</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>East</t></is></c><c r=\"B2\"><v>1</v></c></row>"
                                            "<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>West</t></is></c><c r=\"B3\"><v>2</v></c></row>");
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"B2:B3\"><cfRule type=\"expression\" priority=\"1\"><formula>Sales[Q1]&gt;1</formula></cfRule></conditionalFormatting>"
                                             "<dataValidations count=\"1\"><dataValidation type=\"custom\" sqref=\"B2:B3\"><formula1>Sales[Q1]&lt;10</formula1></dataValidation></dataValidations>");
        const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Sales\" displayName=\"Sales\" ref=\"A1:B3\">"
                                             "<autoFilter ref=\"A1:B3\"/><tableColumns count=\"2\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/></tableColumns></table>");
        const QByteArray chart = "<?xml version=\"1.0\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart><c:plotArea><c:barChart><c:ser><c:val><c:numRef><c:f>Sales[Q1]</c:f></c:numRef></c:val></c:ser></c:barChart></c:plotArea></c:chart></c:chartSpace>";
        QByteArray book = buildXlsx({worksheet(rows, after)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Q1Total\">SUM(Sales[Q1])</definedName>"));
        book = withTables(book, {table});
        book = withParts(book, {{QStringLiteral("xl/charts/chart1.xml"), chart, QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), -1, QString(), QString(), QString()}});
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:6"));
        auto saveAndRead = [&](const QString &part) {
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            const bool modified = e.win.w.isModified();
            const bool started = e.win.w.save();
            if (started && modified) saved.wait(20000);
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            QMap<QString, QByteArray> parts;
            jp::readZip(f.readAll(), parts);
            return QString::fromUtf8(parts.value(part));
        };
        QVERIFY(s->type(0, 0, 1, QStringLiteral("Quarter")).ok);
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("E1")), QStringLiteral("=SUM(Sales[Quarter])"));
        QCOMPARE(e.win.w.calculation()->definedName(0, QStringLiteral("Q1Total")), QStringLiteral("SUM(Sales[Quarter])"));
        QCOMPARE(e.valueAt(QStringLiteral("F1")), QStringLiteral("n:6"));
        QVERIFY2(saveAndRead(QStringLiteral("xl/workbook.xml")).contains(QStringLiteral(">SUM(Sales[Quarter])</definedName>")), "names");
        const QString sheet = savedSheetText(e.path);
        QVERIFY2(sheet.contains(QStringLiteral("<formula>Sales[Quarter]&gt;1</formula>")) && sheet.contains(QStringLiteral("<formula1>Sales[Quarter]&lt;10</formula1>")), qPrintable(sheet));
        QVERIFY(saveAndRead(QStringLiteral("xl/charts/chart1.xml")).contains(QStringLiteral("<c:f>Sales[Quarter]</c:f>")));
        // Undo after the save puts the old name back in the file the next save makes
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.calculation()->definedName(0, QStringLiteral("Q1Total")), QStringLiteral("SUM(Sales[Q1])"));
        QVERIFY(saveAndRead(QStringLiteral("xl/workbook.xml")).contains(QStringLiteral(">SUM(Sales[Q1])</definedName>")));
        const QString again = savedSheetText(e.path);
        QVERIFY2(again.contains(QStringLiteral("<formula>Sales[Q1]&gt;1</formula>")) && again.contains(QStringLiteral("<formula1>Sales[Q1]&lt;10</formula1>")), qPrintable(again));
        QVERIFY(saveAndRead(QStringLiteral("xl/charts/chart1.xml")).contains(QStringLiteral("<c:f>Sales[Q1]</c:f>")));
        QCOMPARE(formulaOf(e, QStringLiteral("E1")), QStringLiteral("=SUM(Sales[Q1])"));
    }

    // After a save, Undo does not go back over the rows, columns, and cells moved before it (the file has them), but goes on over what came after; Redo of what was undone works.
    void undoAfterASaveStopsAtTheLastMoveSaved()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==A3"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 1, 1).ok);                                      // 1: a move, saved below
        QVERIFY(s->type(0, 0, 0, QStringLiteral("10")).ok);                      // 2: typing
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.settle());
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        QVERIFY(s->deleteRows(0, 0, 1).ok);                                      // 3: a move after the save
        QVERIFY(s->type(0, 4, 0, QStringLiteral("7")).ok);                       // 4
        QVERIFY(e.settle());
        QVERIFY(s->canUndo());
        pressUndo(&e.win.w);                                                     // 4 undone
        pressUndo(&e.win.w);                                                     // 3 undone
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A4"));
        QVERIFY(!e.win.w.isModified());
        QVERIFY(s->canUndo());                                                   // the typing (2) came after the move (1) and can be undone
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QVERIFY(!s->canUndo());                                                  // the move (1) is in the file
        pressUndo(&e.win.w);
        QCOMPARE(e.win.w.undoStack()->index(), 1);
        pressRedo(&e.win.w);
        pressRedo(&e.win.w);
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), 4);
        QVERIFY(e.win.w.isModified());
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(differences(e.win.w.calculation(), second.w.calculation(), {}), QString());
        QCOMPARE(second.w.session()->editText(0, 0, 1), QString());             // (B1 was in the row that was deleted)
        QCOMPARE(second.w.session()->editText(0, 2, 0), QStringLiteral("3"));
        QCOMPARE(second.w.session()->editText(0, 4, 0), QStringLiteral("7"));
    }

    // A sheet that has a PivotTable is not changed by inserting or deleting (the table sits where the file says it is): the person is told.
    void aSheetWithAPivotTableIsNotMoved()
    {
        QByteArray book = sheetOf({"A1=1", "A2=2"});
        book = withParts(book, {{QStringLiteral("xl/pivotTables/pivotTable1.xml"), QByteArray("<pivotTableDefinition/>"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"), 0, QStringLiteral("rIdP"),
                                 QStringLiteral("pivotTable"), QStringLiteral("../pivotTables/pivotTable1.xml")}});
        EditWindow e;
        QVERIFY(e.open(book));
        e.view()->selectRows(0, 0);
        e.win.told.clear();
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("PivotTable")), qPrintable(e.win.told.join('|')));
        // typing is as ever
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("5"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
    }

    // Results worked out before cells moved name them by where they were; they are put where the cells are now (and dropped with the cells that are gone).
    void resultsMadeBeforeTheCellsMovedAreMovedToo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1"})));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->generation(), 0);
        auto item = [](quint32 row, quint32 col, double v) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = row;
            it.col = col;
            it.value = calc::Value::number(v);
            return it;
        };
        QVERIFY(s->insertRows(0, 2, 3).ok);                                      // 3 rows at row 3
        QVERIFY(s->deleteColumns(0, 5, 1).ok);                                   // column F
        QCOMPARE(s->generation(), 2);
        std::vector<grid::ValueStore::Item> items{item(0, 1, 1), item(1, 1, 2), item(2, 1, 3), item(10, 1, 4), item(1, 5, 5), item(1, 6, 6)};
        s->moveToNow(&items, 0);
        QCOMPARE(int(items.size()), 5);
        QCOMPARE(items[0].row, quint32(0));
        QCOMPARE(items[2].row, quint32(5));
        QCOMPARE(items[3].row, quint32(13));
        QCOMPARE(items[4].col, quint32(5));                                       // (G moved to F)
        // made after the first edit: only the second applies
        std::vector<grid::ValueStore::Item> later{item(10, 1, 4), item(1, 6, 6)};
        s->moveToNow(&later, 1);
        QCOMPARE(later[0].row, quint32(10));
        QCOMPARE(later[1].col, quint32(5));
        QVERIFY(e.settle());
        // the generation is the loader's: results worked out now carry it
        QCOMPARE(e.win.w.workbookLoader().structureGeneration(), 2);
    }
};

JP_TEST_CLASS(CrunchCellsTests)
#include "tst_crunch_cells.moc"
