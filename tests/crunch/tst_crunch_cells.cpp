// Tests of rows, columns, and cells inserted and deleted in an open workbook: the cells and what refers to them move, the values the window holds move with them, one step undoes it, and the
// keys and menus ask for it. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "savedok.h"
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
        a->sheet(s).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &c) { circular = circular || a->isCircular(s, row, col); });
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
        a->sheet(s).cells.forEach([&](quint32 cellRow, quint32 cellCol, const office::sheet::Cell &c) { compareAt(cellRow, cellCol); });
        b->sheet(s).cells.forEach([&](quint32 cellRow, quint32 cellCol, const office::sheet::Cell &c) { compareAt(cellRow, cellCol); });
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
    if (!e.settle()) return QStringLiteral("the worker did not finish");
    const office::sheet::Calculation *calc = e.win.w.calculation();
    for (int s = 0; s < calc->sheetCount(); ++s) {
        bool circular = false;
        calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &c) { circular = circular || calc->isCircular(s, row, col); });
        if (circular) continue;
        QString problem;
        calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &c) {
            if (!problem.isEmpty() || !(c.hasFormula() || (c.flags & office::sheet::ArrayMember))) return;
            calc::Value held;
            if (e.win.w.values().find(s, c.rowId, c.colId, &held) && !sameValue(held, calc->cell(s, row, col)))
                problem = QStringLiteral("%1!%2 shows %3 and is %4").arg(s).arg(cellName(row, col), held.debug(), calc->cell(s, row, col).debug());
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


// A small generator whose numbers are the same on every Qt (QRandomGenerator's are not promised to be).
struct Lcg {
    quint64 state;
    explicit Lcg(quint64 seed) : state(seed * 2862933555777941757ull + 3037000493ull) {}
    quint32 next()
    {
        state = state * 6364136223846793005ull + 1442695040888963407ull;
        return quint32(state >> 33);
    }
    int bounded(int n) { return int(next() % quint32(n)); }
};

// A workbook of eight rows (A a number, B three times it, C and D formulas that read them), edited in this order: a few rows (or columns) inserted, a cell typed, the edit `kind` (0 a column inserted,
// 1 cells shifted down, 2 cells moved, 3 a row inserted; a step that deleted cells is not undone after a save, so a kept edit that is is not used), then one more insert of rows (or of columns, for kind 3) that, with `limited`, finds the order of ids full and puts it back to one run
// (renumbering every id there is). Saved, then Undo of those two and saved, then Redo of them and saved: the three saved sheets are returned, and what the window shows is checked at each.
QStringList rebaseScript(bool limited, int kind, bool *rebased, QString *problem)
{
    using office::sheet::CellRange;
    *rebased = false;
    QStringList specs;
    for (int r = 1; r <= 8; ++r)
        specs << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1=%2").arg(r).arg(r * 3) << QStringLiteral("C%1==B%1+1").arg(r) << QStringLiteral("D%1==A%1+C%1").arg(r);
    EditWindow e;
    if (!e.open(sheetOf(specs)) || !e.settle()) {
        *problem = QStringLiteral("the workbook did not open");
        return {};
    }
    crunch::EditSession *s = e.win.w.session();
    const bool rowsAxis = kind != 3;   // (the edit that is kept is of the other axis from the one that is put back to one run)
    if (limited) {
        jp::calc::LineLimits tight = rowsAxis ? jp::calc::LineLimits::rows() : jp::calc::LineLimits::cols();
        tight.idLimit = tight.firstNew + 10;
        s->setLineLimitsForTest(0, rowsAxis ? tight : jp::calc::LineLimits::rows(), rowsAxis ? jp::calc::LineLimits::cols() : tight);
    }
    auto line = [&](quint32 at, quint32 count) { return rowsAxis ? s->insertRows(0, at, count) : s->insertColumns(0, at, count); };
    auto nextId = [&] { return rowsAxis ? s->calculation()->sheet(0).rows().nextId().value() : s->calculation()->sheet(0).cols().nextId().value(); };
    const quint32 usedBefore = nextId();
    bool ok = line(3, 3).ok && line(7, 2).ok && line(1, 2).ok;
    ok = ok && s->type(0, 1, 6, QStringLiteral("77")).ok;
    CellRange block;
    block.valid = true;
    block.row1 = 1;
    block.row2 = 2;
    block.col1 = 1;
    block.col2 = 1;
    if (kind == 2) block.row1 = block.row2 = 0;   // (B1, a number that C1 reads: a block of cells with something in them; the rows that the inserts made are empty)
    switch (kind) {
    case 0: ok = ok && s->insertColumns(0, 1, 1).ok; break;
    case 1: ok = ok && s->insertCells(0, block, true).ok; break;
    case 2: ok = ok && s->moveCells(0, block, 0, 20, 8).ok; break;
    default: ok = ok && s->insertRows(0, 2, 1).ok; break;
    }
    const quint32 usedAfterKept = nextId();
    ok = ok && line(5, 2).ok;
    *rebased = nextId() < usedAfterKept && usedAfterKept > usedBefore;
    if (!ok || (limited && !*rebased)) {
        *problem = QStringLiteral("the edits were refused, or the order was not put back to one run");
        return {};
    }
    QStringList saved;
    auto save = [&] {
        if (!e.settle()) return false;
        const QString stale = storeDiffers(e);
        if (!stale.isEmpty()) {
            *problem = stale;
            return false;
        }
        // (the cells the file will have written whole, with their formulas and values: a mark on a cell it should not be on shows here)
        crunch::EditSession::Snapshot snap;
        QString why;
        if (!s->takeSnapshot(&snap, &why)) {
            *problem = why;
            return false;
        }
        QString digest;
        for (const office::sheet::SavedCell &c : snap.changes.sheets[0].cells)
            digest += QStringLiteral("%1:%2=%3|").arg(cellName(c.row, c.col), c.isFormula ? c.formula : QString(), QString::number(c.value.number));
        QSignalSpy spy(&e.win.w, &CrunchWindow::saved);
        if (!e.win.w.save() || !savedOk(spy, 20000)) return false;
        saved << digest + savedSheetText(e.path);
        return true;
    };
    // (what the Move did, in absolute terms: B1's 3 is at I23 -- row 21 moved down by the two rows the last insert made -- B1 is empty, and C1, which read B1, reads I23)
    auto content = [&](quint32 row, quint32 col) { return s->calculation()->content(0, row, col); };
    if (kind == 2 && (content(22, 8).value.number() != 3 || content(0, 1).exists || content(0, 2).formula != QStringLiteral("I23+1"))) {
        *problem = QStringLiteral("the cells were not moved: I23 holds %1, C1 holds %2").arg(content(22, 8).value.debug(), content(0, 2).formula);
        return {};
    }
    if (!save()) return {};
    e.win.w.undoStack()->undo();   // (the insert that put the order back to one run: the edit before it is kept, and is undone after it)
    e.win.w.undoStack()->undo();
    if (kind == 2 && (content(0, 1).value.number() != 3 || content(22, 8).exists || content(0, 2).formula != QStringLiteral("B1+1"))) {
        *problem = QStringLiteral("the Move was not undone: B1 holds %1, C1 holds %2").arg(content(0, 1).value.debug(), content(0, 2).formula);
        return {};
    }
    if (!save()) return {};
    e.win.w.undoStack()->redo();
    e.win.w.undoStack()->redo();
    if (kind == 2 && (content(22, 8).value.number() != 3 || content(0, 1).exists || content(0, 2).formula != QStringLiteral("I23+1"))) {
        *problem = QStringLiteral("the Move was not done again: I23 holds %1, C1 holds %2").arg(content(22, 8).value.debug(), content(0, 2).formula);
        return {};
    }
    if (!save()) return {};
    return saved;
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
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const QString diff = differences(e.win.w.calculation(), second.w.calculation(), {"Total", "_xlnm.Print_Area"});
        QVERIFY2(diff.isEmpty(), qPrintable(diff));
        QCOMPARE(second.w.calculation()->definedName(0, QStringLiteral("Total")), e.win.w.calculation()->definedName(0, QStringLiteral("Total")));
        // Undo after the save goes back no further than the moves it holds
        QVERIFY(!e.win.w.isModified());
        QVERIFY(!e.win.w.session()->canUndo());
    }

    // A formula that is its own row and column (ROW() and COLUMN()) shows the new place when an edit moves its cell, as it does for a cell that was moved by Undo, a cut, or a sort.
    void aFormulaThatIsItsOwnPlaceIsWorkedOutAgainWhenAnEditMovesIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1==SUM(A1:A2)"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 2, 1, QStringLiteral("=ROW()*100+COLUMN()")).ok);   // B3
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:302"));
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B5")), QStringLiteral("n:502"));
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("-"));
        QVERIFY(s->insertColumns(0, 0, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C5")), QStringLiteral("n:503"));
        QVERIFY(s->deleteRows(0, 1, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C4")), QStringLiteral("n:403"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C5")), QStringLiteral("n:503"));
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:302"));
        // the value the window holds is the calculation's, and a file saved now says the same
        QVERIFY(storeDiffers(e).isEmpty());
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B5")), QStringLiteral("n:502"));
    }

    // A shared formula that an edit wrote again, whose cells a later edit deleted, is what it was after both are undone: each cell reads the cells beside it, not those of the family's first cell.
    void aSharedFormulaWrittenAgainAndThenDeletedIsRightAfterBothAreUndone()
    {
        EditWindow e;
        QVERIFY(e.open(richBook()));   // (B1:B5 is one shared formula, =A1*2 down the column)
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->deleteColumns(0, 0, 1).ok);   // the family reads the column that is gone: all five are written again alike (#REF!*2), and are in column A now
        QVERIFY(s->deleteColumns(0, 0, 1).ok);   // and these are deleted
        QVERIFY(e.settle());
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("A4")), QStringLiteral("=#REF!*2"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        for (int row = 1; row <= 5; ++row) QCOMPARE(formulaOf(e, QStringLiteral("B%1").arg(row)), QStringLiteral("=A%1*2").arg(row));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:8"));
        QVERIFY(storeDiffers(e).isEmpty());
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
                const int which = rng.bounded(18);
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
                case 17:   // a save in the middle: what comes after (an Undo of a step made before it too) is made against the saved file
                    if (e.win.w.isModified()) {
                        QSignalSpy midway(&e.win.w, &CrunchWindow::saved);
                        QVERIFY(e.win.w.save());
                        QVERIFY2(savedOk(midway, 20000), "the save failed");
                    }
                    break;
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
            if (modified) QVERIFY2(savedOk(saved, 20000), "the save failed");
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

    // A one-cell array formula whose row is deleted, with the delete undone and the file saved: the file has the array again, as the window has it.
    void aOneCellArrayDeletedAndUndoneIsSavedAsAnArray()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row>"
                                            "<row r=\"5\"><c r=\"D5\"><f t=\"array\" ref=\"D5\">SUM(A1:A3*2)</f><v>12</v></c></row><row r=\"6\"><c r=\"D6\"><v>9</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->deleteRows(0, 4, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D5")), QStringLiteral("n:9"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D5")), QStringLiteral("n:12"));
        QVERIFY(!s->canEditCell(0, 4, 3));   // (a cell of an array is not edited alone: it is an array again)
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("5"));        // (something to save)
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QString sheet = savedSheetText(e.path);
        QVERIFY2(sheet.contains(QStringLiteral("<f t=\"array\" ref=\"D5\">SUM(A1:A3*2)</f>")), qPrintable(sheet));
        QVERIFY2(sheet.contains(QStringLiteral("<c r=\"D6\"><v>9</v></c>")), qPrintable(sheet));
    }

    // Undo goes back past a save for everything the saved file can take: typing, rows inserted, and cells cut and pasted, done before the save, are undone after it, and the file saved
    // again has them undone (the edit that undoes a step is made against the saved file).
    void undoGoesBackPastASaveForWhatTheFileCanTake()
    {
        auto save = [](EditWindow &e) {
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved, 20000), "the save failed");
        };
        auto agreeWithFile = [](EditWindow &e) {
            Window second;
            QVERIFY(second.open(e.copyOfFile()));
            const QString diff = differences(e.win.w.calculation(), second.w.calculation(), {"Total"});
            QVERIFY2(diff.isEmpty(), qPrintable(diff));
        };
        {
            EditWindow e;
            QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==A1+A2", "B3==SUM(A1:A3)"})));
            e.select(QStringLiteral("D1"));
            e.enter(QStringLiteral("5"));                                  // step 1
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->insertRows(0, 1, 2).ok);            // step 2: two rows above row 2
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:2"));
            save(e);
            QCOMPARE(e.win.w.session()->stepsForgottenBySave(), 0);
            QVERIFY(e.win.w.session()->canUndo());
            e.win.w.act(QStringLiteral("edit.undo"))->trigger();           // the rows inserted
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:3"));
            QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("-"));
            QCOMPARE(formulaOf(e, QStringLiteral("B3")), QStringLiteral("=SUM(A1:A3)"));
            QVERIFY(e.win.w.isModified());
            save(e);
            agreeWithFile(e);
            QCOMPARE(savedFormula(e.path, QStringLiteral("B3")), QStringLiteral("SUM(A1:A3)"));
            e.win.w.act(QStringLiteral("edit.undo"))->trigger();           // the typing before the first save
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("-"));
            save(e);
            agreeWithFile(e);
            QCOMPARE(savedCell(e.path, QStringLiteral("D1")), QStringLiteral("none"));
            e.win.w.act(QStringLiteral("edit.redo"))->trigger();
            e.win.w.act(QStringLiteral("edit.redo"))->trigger();           // both again
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:5"));
            QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:2"));
            save(e);
            agreeWithFile(e);
        }
        {
            // cells cut and pasted, the file saved, then Undo
            EditWindow e;
            QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==A1+A2", "E4=old"})));
            copyRange(e, QStringLiteral("A1:A2"), "edit.cut");
            paste(e, QStringLiteral("E4"));
            QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=E4+E5"));
            save(e);
            e.win.w.act(QStringLiteral("edit.undo"))->trigger();
            QVERIFY(e.settle());
            QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A1+A2"));
            QCOMPARE(e.valueAt(QStringLiteral("E4")), QStringLiteral("s:old"));
            save(e);
            agreeWithFile(e);
            QCOMPARE(savedFormula(e.path, QStringLiteral("B1")), QStringLiteral("A1+A2"));
            QCOMPARE(savedCell(e.path, QStringLiteral("E4")), QStringLiteral("i:old"));
        }
    }

    // A step that deleted rows, columns, or cells is the one thing the saved file cannot take back (what was on them is gone from it): after a save, Undo goes back as far as that step
    // and no further, and the save says so; what is done after it is undone as ever.
    void aDeleteBeforeASaveIsNotUndoneAfterItAndTheSaveSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->deleteRows(0, 1, 1).ok);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QVERIFY(e.win.w.session()->stepsForgottenBySave() > 0);
        QVERIFY2(e.win.w.statusText().contains(QStringLiteral("can't be undone")), qPrintable(e.win.w.statusText()));
        QVERIFY(!e.win.w.session()->canUndo());
        // typing after the save is undone as ever
        e.select(QStringLiteral("D2"));
        e.enter(QStringLiteral("7"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->canUndo());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("-"));
        QVERIFY(!e.win.w.session()->canUndo());
    }

    // A step that cannot be undone (the workbook cannot be held) does not leave Undo as if it had been: the person is told, Undo and Redo are given up, and the workbook is still modified.
    void aStepThatCannotBeUndoneGivesUpUndoAndSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2"})));
        e.select(QStringLiteral("C1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        e.select(QStringLiteral("C2"));
        e.enter(QStringLiteral("6"));
        QVERIFY(e.settle());
        e.win.w.workbookLoader().cancel();   // (the worker is gone: nothing can be held any more)
        e.win.told.clear();
        e.win.w.undoStack()->undo();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("could not be undone")), qPrintable(e.win.told.join('|')));
        QVERIFY(!e.win.w.session()->canUndo());
        QVERIFY(e.win.w.isModified());
        e.win.told.clear();
        e.win.w.undoStack()->undo();   // (nothing more is tried)
        QVERIFY(e.win.told.isEmpty());
        // and one that cannot be done again (Redo) is told the same way, and is not taken for done
        EditWindow f;
        QVERIFY(f.open(sheetOf({"A1=1", "A2=2"})));
        f.select(QStringLiteral("C1"));
        f.enter(QStringLiteral("5"));
        QVERIFY(f.settle());
        f.select(QStringLiteral("C2"));
        f.enter(QStringLiteral("6"));
        QVERIFY(f.settle());
        f.win.w.undoStack()->undo();   // (the worker is there)
        QVERIFY(f.settle());
        QCOMPARE(f.valueAt(QStringLiteral("C2")), QStringLiteral("-"));
        f.win.w.workbookLoader().cancel();
        f.win.told.clear();
        f.win.w.undoStack()->redo();
        QVERIFY2(!f.win.told.isEmpty() && f.win.told.last().contains(QStringLiteral("could not be done again")), qPrintable(f.win.told.join('|')));
        QVERIFY(!f.win.w.session()->canUndo());
        QVERIFY(f.win.w.isModified());
        f.win.told.clear();
        f.win.w.undoStack()->undo();   // (nothing more is tried, either way)
        f.win.w.undoStack()->redo();
        QVERIFY(f.win.told.isEmpty());
    }

    // A step that moves cells and is bigger than Undo holds (more than the undo limit of bytes would be kept to undo it) is asked about before it is done, as in the programs people know:
    // "this can't be undone"; if the person goes on it is done, and it and the steps before it are not undone. A step that fits is not asked about. While it is done the window says it is working.
    void aStepBiggerThanTheUndoLimitIsAskedAboutAndTheWindowShowsItIsWorking()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setUndoByteLimit(4000);
        for (int i = 0; i < 40; ++i) QVERIFY(s->type(0, quint32(20 + i), 0, QString::number(i)).ok);   // forty cells in A21:A60 (typing is not asked about)
        QVERIFY(e.settle());
        QVERIFY(s->insertRows(0, 100, 1).ok);   // fits: nothing is lost by it
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        // too big: not done, and the person is asked (cells shifted up: a delete of whole rows is not asked about in the id frame, where Undo of it holds no cell)
        crunch::EditSession::Outcome o = s->deleteCells(0, rangeOf(QStringLiteral("A20:A60")), true);
        QVERIFY(!o.ok && o.confirm);
        QVERIFY2(o.problem.contains(QStringLiteral("can't be undone")), qPrintable(o.problem));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.valueAt(QStringLiteral("A21")), QStringLiteral("n:0"));
        // through the window: asked, and said no to
        QStringList asked;
        bool answer = false;
        e.win.w.setBigStepAsker([&](const QString &text) {
            asked << text;
            return answer;
        });
        bool cursorBusy = false;
        QString statusDuring;
        s->setHoldHook([&](jp::office::sheet::Calculation *, bool after) {
            if (after) return;
            cursorBusy = cursorBusy || QGuiApplication::overrideCursor() != nullptr;
            statusDuring = e.win.w.statusText();
        });
        e.win.w.setShiftAsker([](CrunchWindow::ShiftRequest *) { return true; });   // (the way is Up, which the request has to begin with)
        e.view()->selectRange(19, 0, 59, 0);
        e.win.w.act(QStringLiteral("cells.delete"))->trigger();
        QCOMPARE(asked.size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!cursorBusy);   // (nothing was done)
        // and yes
        answer = true;
        e.win.w.act(QStringLiteral("cells.delete"))->trigger();
        QCOMPARE(asked.size(), 2);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A21")), QStringLiteral("-"));
        QVERIFY(cursorBusy);
        QVERIFY2(statusDuring.contains(QStringLiteral("Working")), qPrintable(statusDuring));
        QVERIFY(QGuiApplication::overrideCursor() == nullptr);   // (and the cursor is back)
        QVERIFY(!s->canUndo());                                  // it and what came before are not undone
        // a step that fits is not asked about
        s->setHoldHook(nullptr);
        s->setUndoByteLimit(crunch::kUndoBytes);
        QVERIFY(s->insertRows(0, 2, 1).ok);
        QCOMPARE(asked.size(), 2);
        QVERIFY(s->canUndo());
    }

    // ---- round 6 (docs/jeffcrunch-rowids.md, 4.7): what Undo holds of an edit of rows or columns is the change to their order and the before-images of what the edit changed, kilobytes whatever the
    // size of the sheet; no question "this can't be undone" for them.
private:
    // 100,000 rows that each have their own height, a number in A and a formula in B, three merged ranges, and a width for the first two columns.
    static QByteArray tallBook(int rows)
    {
        QString xml;
        xml.reserve(rows * 110);
        for (int r = 1; r <= rows; ++r)
            xml += QStringLiteral("<row r=\"%1\" ht=\"%2\" customHeight=\"1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2</f><v>0</v></c></row>").arg(r).arg(15 + r % 7);
        const QString merges = QStringLiteral("<mergeCells count=\"3\"><mergeCell ref=\"D5:E6\"/><mergeCell ref=\"D500:F501\"/><mergeCell ref=\"D60000:E60002\"/></mergeCells>");
        return buildXlsx({worksheet(xml, merges, QStringLiteral("<cols><col min=\"1\" max=\"2\" width=\"14\" customWidth=\"1\"/></cols>"))});
    }

private Q_SLOTS:
    // Twelve edits of rows and columns in a sheet of 100,000 rows with a height for each: the steps hold under 64 KB between them (counted), and Undo and Redo of all twelve put it right.
    void twelveEditsOfRowsAndColumnsAt100000RowsHoldUnder64KbOfUndo()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the formulas are held in places: the edits of rows write the specs of rows again, and Undo holds them, as before ids");
        EditWindow e;
        QVERIFY(e.open(tallBook(100000)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const qint64 base = s->undoBytes();
        auto rowHeight = [&](quint32 row) {
            const auto specs = s->calculation()->rowSpecsIn(0, row, row);
            return specs.empty() ? -1.0 : specs.front().height;
        };
        const double h50000 = rowHeight(49999);   // (the row of A50000)
        QVERIFY(h50000 > 0);
        QVERIFY(s->insertRows(0, 10, 2).ok);
        QVERIFY(s->deleteRows(0, 50000, 1).ok);
        QVERIFY(s->insertColumns(0, 3, 1).ok);
        QVERIFY(s->deleteColumns(0, 3, 1).ok);
        QVERIFY(s->insertRows(0, 99000, 1).ok);
        QVERIFY(s->deleteRows(0, 20, 3).ok);
        QVERIFY(s->insertRows(0, 5, 1).ok);
        QVERIFY(s->insertColumns(0, 0, 1).ok);
        QVERIFY(s->deleteColumns(0, 0, 1).ok);
        QVERIFY(s->deleteRows(0, 70000, 5).ok);
        QVERIFY(s->insertRows(0, 60000, 1).ok);
        QVERIFY(s->insertRows(0, 3, 1).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), 12);
        QVERIFY2(s->undoBytes() - base < 64 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
        // (A99999 is the one that was at A100000: two rows and one row and a row put in above it, three taken out, a row put in and five taken out, ...)
        const QString before = e.valueAt(QStringLiteral("B100"));
        for (int i = 0; i < 12; ++i) e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A50000")), QStringLiteral("n:50000"));
        QCOMPARE(e.valueAt(QStringLiteral("B100000")), QStringLiteral("n:200000"));
        QCOMPARE(rowHeight(49999), h50000);
        QCOMPARE(s->calculation()->sheet(0).merges.size(), 3);
        QCOMPARE(s->calculation()->sheet(0).merges[2].row1, quint32(59999));
        for (int i = 0; i < 12; ++i) e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B100")), before);
        QVERIFY2(s->undoBytes() - base < 64 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
    }

    // A delete of 500,000 rows is not asked about (nothing it holds is big) and is undone at once: the values are the window's before the worker has worked anything out again.
    void aDeleteOfHalfAMillionRowsIsNotAskedAboutAndIsUndoneAtOnce()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the formulas are held in places: a delete of rows holds the cells it deletes, as before ids");
        const int n = 550000;
        QString xml;
        xml.reserve(n * 60);
        for (int r = 1; r <= n; ++r) xml += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2</f><v>0</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(xml)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const qint64 base = s->undoBytes();
        const crunch::EditSession::Outcome o = s->deleteRows(0, 20000, 500000);
        QVERIFY2(o.ok && !o.confirm, qPrintable(o.problem));
        // (the record is the change to the order and the dead cells it left in the store, 1,000,000 of them at 48 bytes counted: 48 MB, not the 320 MB their copies took)
        QVERIFY2(s->undoBytes() - base < 1000000 * 48 + 64 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A20001")), QStringLiteral("n:520001"));
        QCOMPARE(e.valueAt(QStringLiteral("B20001")), QStringLiteral("n:1040002"));
        e.win.w.undoStack()->undo();
        QCoreApplication::processEvents();   // (the grid takes what the undo told it; the worker is not waited for)
        QCOMPARE(e.valueAt(QStringLiteral("B400000")), QStringLiteral("n:800000"));
        QCOMPARE(e.valueAt(QStringLiteral("A20001")), QStringLiteral("n:20001"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B300000")), QStringLiteral("n:600000"));
        QVERIFY(storeDiffers(e).isEmpty());
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A20001")), QStringLiteral("n:520001"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B549999")), QStringLiteral("n:1099998"));
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // Redo after Undo after Redo: each time the sheet is as it was after the edit (cells, formulas, heights), and the steps hold what they held.
    void redoAfterUndoAfterRedoGivesTheSameSheetEachTime()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A5)", "B3==A3*2", "C2==B3+A5", "D4=text"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const qint64 base = s->undoBytes();
        auto state = [&] {
            QStringList out;
            for (const char *ref : {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "B1", "B2", "B3", "B4", "B5", "C2", "C3", "C4", "D4", "D5", "E4"})
                out << QString::fromLatin1(ref) + QLatin1Char('=') + e.valueAt(QString::fromLatin1(ref)) + QLatin1Char(' ') + formulaOf(e, QString::fromLatin1(ref));
            return out.join(QLatin1Char('|'));
        };
        const QString start = state();
        QVERIFY(s->insertRows(0, 2, 2).ok);
        QVERIFY(s->deleteRows(0, 5, 1).ok);
        QVERIFY(s->insertColumns(0, 1, 1).ok);
        QVERIFY(s->deleteColumns(0, 0, 1).ok);
        QVERIFY(e.settle());
        const QString done = state();
        const qint64 held = s->undoBytes();
        QVERIFY(done != start);
        for (int round = 0; round < 3; ++round) {
            for (int i = 0; i < 4; ++i) e.win.w.undoStack()->undo();
            QVERIFY(e.settle());
            QCOMPARE(state(), start);
            if (office::sheet::Calculation::idFrameDefault()) QCOMPARE(s->undoBytes(), held);   // (the steps hold the same after: they are the same steps)
            for (int i = 0; i < 4; ++i) e.win.w.undoStack()->redo();
            QVERIFY(e.settle());
            QCOMPARE(state(), done);
            if (office::sheet::Calculation::idFrameDefault()) QCOMPARE(s->undoBytes(), held);
        }
        QVERIFY(storeDiffers(e).isEmpty());
        // an Undo that stops half way, then a new edit: the steps that were undone are let go of, and the rows they took are given back
        for (int i = 0; i < 2; ++i) e.win.w.undoStack()->undo();
        QVERIFY(s->insertRows(0, 0, 1).ok);
        QVERIFY(!e.win.w.undoStack()->canRedo());
        e.win.w.undoStack()->undo();
        for (int i = 0; i < 2; ++i) e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(state(), start);
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // Round 6: the formulas in rows that were deleted are as they were when the rows come back, whatever was done in between and undone before: rows put in, a sheet renamed, a name made, a cell typed.
    // (They stay in the workbook while the step can be undone, and nothing that is done is written for them: a sheet name, a name, a table, and the places formulas see are not theirs to change.)
    void formulasInDeletedRowsComeBackAsTheyWereAfterOtherEditsAreUndone()
    {
        auto cell = [](const char *ref, const QString &formula, int value = 0) {
            return QStringLiteral("<c r=\"%1\"><f>%2</f><v>%3</v></c>").arg(QString::fromLatin1(ref), formula.toHtmlEscaped()).arg(value);
        };
        const QString sheet1 = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>")
            + QStringLiteral("<row r=\"2\"><c r=\"A2\"><v>6</v></c></row>")
            + QStringLiteral("<row r=\"3\"><c r=\"A3\"><v>7</v></c>") + cell("B3", QStringLiteral("A3*2")) + cell("C3", QStringLiteral("Sheet2!A1+1")) + cell("D3", QStringLiteral("INDIRECT(\"A1\")")) + cell("E3", QStringLiteral("Rate*A3")) + QStringLiteral("</row>")
            + QStringLiteral("<row r=\"4\"><c r=\"A4\"><v>8</v></c>") + cell("B4", QStringLiteral("SUM(A1:A4)")) + cell("C4", QStringLiteral("ROW()")) + cell("D4", QStringLiteral("Sheet2!A1+Sheet2!A2")) + QStringLiteral("</row>")
            + QStringLiteral("<row r=\"5\"><c r=\"A5\"><v>9</v></c>") + cell("B5", QStringLiteral("SUM(A3:A4)")) + QStringLiteral("</row>");
        const QString sheet2 = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>100</v></c></row><row r=\"2\"><c r=\"A2\"><v>200</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(sheet1), worksheet(sheet2)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QStringLiteral("<definedName name=\"Rate\">Sheet1!$A$1</definedName>"))));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        auto state = [&] {
            QStringList out;
            for (const char *ref : {"A1", "A2", "A3", "A4", "A5", "A6", "B1", "B2", "B3", "B4", "B5", "C3", "C4", "D3", "D4", "E3"})
                out << QString::fromLatin1(ref) + QLatin1Char('=') + e.valueAt(QString::fromLatin1(ref)) + QLatin1Char(' ') + formulaOf(e, QString::fromLatin1(ref));
            return out.join(QLatin1Char('|'));
        };
        const QString start = state();
        QVERIFY(start.contains(QStringLiteral("C3=n:101")));
        QUndoStack *u = e.win.w.undoStack();
        QVERIFY(s->deleteRows(0, 2, 2).ok);   // rows 3 and 4: their formulas are dead
        QVERIFY(e.settle());
        QVERIFY(s->insertRows(0, 0, 1).ok);   // (a row put in above)
        QVERIFY(s->renameSheet(1, QStringLiteral("Facts")).ok);
        QVERIFY(s->type(1, 0, 0, QStringLiteral("500")).ok);
        crunch::EditSession::NameDraft draft;
        draft.name = QStringLiteral("Other");
        draft.text = QStringLiteral("=Facts!$A$2");
        QVERIFY(s->defineName(draft).ok);
        QVERIFY(e.settle());
        QCOMPARE(u->count(), 5);
        for (int i = 0; i < 4; ++i) u->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->workbook()->info().sheets[1].name, QStringLiteral("Sheet2"));
        u->undo();   // the rows come back
        QVERIFY(e.settle());
        QCOMPARE(state(), start);
        QVERIFY(storeDiffers(e).isEmpty());
        // and the whole of it again, and back
        for (int i = 0; i < 5; ++i) u->redo();
        QVERIFY(e.settle());
        for (int i = 0; i < 5; ++i) u->undo();
        QVERIFY(e.settle());
        QCOMPARE(state(), start);
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // Round 6: what a forgotten delete of rows left (the dead cells, and the nodes of the formulas in them) is let go of a slice at a time: the step that is let go of does none of it, each slice does about as
    // many cells as it is given (counted), and when the last is done the cells, the nodes, and the values the window held for them are gone.
    void theDeadCellsOfAForgottenDeleteAreLetGoOfInSlices()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the formulas are held in places: a delete of rows holds the cells it deletes in the step, as before ids");
        const int n = 20000;
        QString xml;
        for (int r = 1; r <= n; ++r) xml += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2</f><v>0</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(xml)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const office::sheet::Calculation *calc = s->calculation();
        const qint64 formulas = calc->formulaCount(), values = e.win.w.values().count();
        QCOMPARE(values, qint64(n));
        QVERIFY(s->deleteRows(0, 1000, 15000).ok);
        QVERIFY(e.settle());
        QCOMPARE(calc->sheet(0).cells.deadCount(), qint64(30000));
        QCOMPARE(calc->formulaCount(), formulas);   // (the nodes are kept while the step can be undone)
        QCOMPARE(e.win.w.values().count(), values);
        e.win.w.undoStack()->clear();   // the step is let go of
        QVERIFY(s->type(0, 0, 5, QStringLiteral("1")).ok);   // (the next step takes the records of the steps that went)
        QVERIFY(s->purgeWaiting() > 0);
        QCOMPARE(calc->sheet(0).cells.deadCount(), qint64(30000));   // (nothing is let go of yet)
        QCOMPARE(calc->formulaCount(), formulas);
        int slices = 0;
        qint64 last = s->purgeWaiting();
        while (s->purgeSome(2000)) {
            ++slices;
            const qint64 now = s->purgeWaiting();
            QVERIFY2(last - now <= 2000 + 64 * 2, qPrintable(QString::number(last - now)));   // (a block is not cut: it is a slice and at most a block more)
            last = now;
        }
        QVERIFY2(slices >= 10, qPrintable(QString::number(slices)));
        QCOMPARE(s->purgeWaiting(), qint64(0));
        QCOMPARE(calc->sheet(0).cells.deadCount(), qint64(0));
        QCOMPARE(calc->formulaCount(), formulas - 15000);
        QCOMPARE(e.win.w.values().count(), values - 15000);
        // the ids may be used again, and the sheet is as the edits left it
        QVERIFY(s->insertRows(0, 5, 2).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1003")), QStringLiteral("n:32002"));   // (the row of A16001 before, two rows lower: 16001 * 2)
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // Round 6, the budget: the cells a delete of rows leaves dead are counted in its step's bytes, so that the undo limit bounds them (older steps are forgotten past it), and a session that never runs
    // its loop (headless) lets go of what forgotten steps held at its next edit of rows, a slice at a time: a loop of deleting and putting back stays bounded.
    void deadCellsAreCountedInTheUndoLimitAndLetGoOfWithoutTheLoop()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the formulas are held in places: a delete of rows copies the cells, and they are counted as before ids");
        QString xml;
        for (int r = 1; r <= 40000; ++r) xml += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2</f><v>0</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(xml)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const qint64 base = s->undoBytes();
        s->setUndoByteLimit(base + 600 * 1000);   // (a step of 1,000 rows leaves 2,000 cells: 96,000 bytes counted)
        QVERIFY(s->deleteRows(0, 0, 1000).ok);
        QVERIFY2(s->undoBytes() - base >= 90 * 1000, qPrintable(QString::number(s->undoBytes() - base)));
        const office::sheet::Calculation *calc = s->calculation();
        for (int i = 0; i < 30; ++i) {
            QVERIFY(s->deleteRows(0, 0, 1000).ok);   // (no loop runs: nothing here waits for the window)
            QVERIFY2(s->undoBytes() - base <= 600 * 1000, qPrintable(QString::number(s->undoBytes() - base)));
        }
        // (30 steps of 2,000 cells were made; the limit holds six of them, and what the forgotten ones left was let go of by the edits that followed them, at most a few slices behind)
        QVERIFY2(calc->sheet(0).cells.deadCount() < 40000, qPrintable(QString::number(calc->sheet(0).cells.deadCount())));
        QVERIFY(e.settle());
    }

    // Round 6: the specs of rows are renumbered when the order of rows is put back to one run (a hidden row, a height, an outline level), and what is saved after has them at their rows.
    void theSpecsOfRowsSurviveAnOrderPutBackToOneRunAndTheirSave()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the specs of rows are held by place: there are no ids to renumber");
        QStringList cells;
        for (int r = 1; r <= 12; ++r) cells << QStringLiteral("A%1=%1").arg(r);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        crunch::EditSession::SizeChange hide, tall;
        hide.setHidden = hide.hidden = true;
        tall.setSize = true;
        tall.size = 40;
        // (the order may hold few ids: edits of rows until it is near that, and the steps are forgotten and the order is put back to one run)
        s->setLineLimitsForTest(0, [] {
            jp::calc::LineLimits tight = jp::calc::LineLimits::rows();
            tight.idLimit = tight.firstNew + 12;
            return tight;
        }(), jp::calc::LineLimits::cols());
        QVERIFY(s->resizeRows(0, 7, 7, hide, QStringLiteral("Hide")).ok);   // row 8
        QVERIFY(s->resizeRows(0, 9, 9, tall, QStringLiteral("Taller")).ok);  // row 10
        QVERIFY(s->insertRows(0, 0, 3).ok);
        QVERIFY(s->deleteRows(0, 0, 2).ok);
        for (int i = 0; i < 10; ++i) QVERIFY(s->insertRows(0, 20, 1).ok);
        QVERIFY(e.settle());
        // (the ids of the rows were put back to their places: the specs, which are held by id, are at the ids that are the places of their rows)
        QCOMPARE(s->calculation()->sheet(0).rowSpecs.size(), 2);
        QCOMPARE(s->calculation()->sheet(0).rowSpecs[0].row, quint32(8));
        QCOMPARE(s->calculation()->sheet(0).rowSpecs[1].row, quint32(10));
        auto at = [&](quint32 row) { return s->calculation()->rowSpecsIn(0, row, row); };
        QVERIFY(at(8).size() == 1 && at(8).front().hidden);    // (A8 is at place 8 now: one row put in net)
        QCOMPARE(at(10).size(), size_t(1));
        QCOMPARE(at(10).front().height, 40.0);
        QCOMPARE(s->calculation()->rowSpecsIn(0, 0, 100).size(), size_t(2));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        Workbook b;
        QVERIFY(b.open(p, &why));
        office::sheet::Sheet sheet;
        QVERIFY(b.readSheet(0, &sheet));
        QCOMPARE(sheet.rowSpecs.size(), 2);
        QCOMPARE(sheet.rowSpecs[0].row, quint32(8));
        QVERIFY(sheet.rowSpecs[0].hidden);
        QCOMPARE(sheet.rowSpecs[1].row, quint32(10));
        QCOMPARE(sheet.rowSpecs[1].height, 40.0);
    }

    // Round 6: what a delete of rows is asked about by is what its step will hold: its dead cells at 48 bytes each (the same figure the estimate and `StructChange::bytes` use). A delete of 500,000 rows of 5
    // columns leaves 2,500,000 cells, 120,000,000 bytes, which is under the 134,217,728 of the undo limit: it is not asked about, and it holds 120 MB of the 128, so the steps older than it are forgotten as
    // far as they must be. Here the same in small: 5 columns, a limit of 800,000 bytes, a delete of 3,000 rows (15,000 cells: 720,000) and then one of 700 rows (3,500 cells: 168,000).
    void aDeleteOfRowsIsAskedAboutByWhatItsStepWillHoldAndTheBudgetForgetsTheOlderStep()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the formulas are held in places: a delete of rows copies the cells at 320 bytes each, as before ids");
        QString xml;
        for (int r = 1; r <= 5000; ++r) xml += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>2</v></c><c r=\"C%1\"><v>3</v></c><c r=\"D%1\"><v>4</v></c><c r=\"E%1\"><f>SUM(A%1:D%1)</f><v>0</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(xml)})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const qint64 base = s->undoBytes();
        s->setUndoByteLimit(base + 700 * 1000);
        crunch::EditSession::Outcome o = s->deleteRows(0, 0, 3000);
        QVERIFY2(!o.ok && o.confirm, "720,000 bytes against 700,000: it is asked about");
        s->setUndoByteLimit(base + 800 * 1000);
        o = s->deleteRows(0, 0, 3000);
        QVERIFY2(o.ok && !o.confirm, qPrintable(o.problem));
        QVERIFY2(s->undoBytes() - base >= 720 * 1000 && s->undoBytes() - base < 730 * 1000, qPrintable(QString::number(s->undoBytes() - base)));
        QVERIFY(s->canUndo());
        o = s->deleteRows(0, 0, 700);   // 168,000 more: past the limit, so the older step is forgotten (the newest never is)
        QVERIFY2(o.ok && !o.confirm, qPrintable(o.problem));
        QVERIFY2(s->undoBytes() - base <= 800 * 1000, qPrintable(QString::number(s->undoBytes() - base)));
        QVERIFY2(s->undoBytes() - base >= 168 * 1000, qPrintable(QString::number(s->undoBytes() - base)));
        QVERIFY(s->canUndo());
        e.win.w.undoStack()->undo();   // the second delete comes back
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:3001"));
        QVERIFY(!s->canUndo());       // and the first was forgotten
    }

    // Round 6: a row that has a spec (hidden) is deleted, the order is put back to one run: the spec is gone, and is not on another row, in the model and in the saved file; the spec of a row that is left is where
    // its row is.
    void theSpecOfADeletedRowIsGoneAfterAnOrderPutBackToOneRunAndDoesNotMoveToAnotherRow()
    {
        if (!office::sheet::Calculation::idFrameDefault()) QSKIP("the specs of rows are held by place: there are no ids to renumber");
        QStringList cells;
        for (int r = 1; r <= 12; ++r) cells << QStringLiteral("A%1=%1").arg(r);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        crunch::EditSession::SizeChange hide, tall;
        hide.setHidden = hide.hidden = true;
        tall.setSize = true;
        tall.size = 40;
        s->setLineLimitsForTest(0, [] {
            jp::calc::LineLimits tight = jp::calc::LineLimits::rows();
            tight.idLimit = tight.firstNew + 12;
            return tight;
        }(), jp::calc::LineLimits::cols());
        QVERIFY(s->resizeRows(0, 7, 7, hide, QStringLiteral("Hide")).ok);    // row 8
        QVERIFY(s->resizeRows(0, 9, 9, tall, QStringLiteral("Taller")).ok);  // row 10
        QVERIFY(s->deleteRows(0, 7, 1).ok);                                  // the hidden row
        for (int i = 0; i < 10; ++i) QVERIFY(s->insertRows(0, 20, 1).ok);   // (until the order is compacted, with the steps forgotten)
        QVERIFY(e.settle());
        auto at = [&](quint32 row) { return s->calculation()->rowSpecsIn(0, row, row); };
        QVERIFY(at(7).empty());                                  // (A10 is at place 8 now, with its height; nothing is hidden on any row)
        QVERIFY(at(8).size() == 1 && at(8).front().height == 40.0 && !at(8).front().hidden);
        QCOMPARE(s->calculation()->rowSpecsIn(0, 0, 100).size(), size_t(1));
        QCOMPARE(s->calculation()->sheet(0).rowSpecs.size(), 1);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        Workbook b;
        QVERIFY(b.open(p, &why));
        office::sheet::Sheet sheet;
        QVERIFY(b.readSheet(0, &sheet));
        QCOMPARE(sheet.rowSpecs.size(), 1);
        QCOMPARE(sheet.rowSpecs[0].row, quint32(8));
        QCOMPARE(sheet.rowSpecs[0].height, 40.0);
        QVERIFY(!sheet.rowSpecs[0].hidden);
    }

    // Undo and a save: a step of rows put in before a save is undone after it (the saved file has the rows, so the edit that takes them out is made against the saved file), and a step of rows deleted
    // after a save is undone, and the file saved again is the one that was first; what was deleted before a save is the one thing Undo does not go back over (the m_saveGen rules are as they were).
    void undoAcrossASaveKeepsTheRulesOfTheSavedFile()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4", "B1==SUM(A1:A4)", "B3==A3*2"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        auto save = [&] {
            QSignalSpy spy(&e.win.w, &CrunchWindow::saved);
            return e.win.w.save() && savedOk(spy, 20000);
        };
        QVERIFY(s->type(0, 0, 5, QStringLiteral("9")).ok);   // (a file that is not changed is not saved)
        QVERIFY(e.settle());
        QVERIFY(save());
        // (what the saved file holds of the cells, by place: its numbers, and the text of its formulas; the results the file stores for formulas are the saver's, and are not compared)
        auto view = [&] {
            QStringList out;
            for (int row = 1; row <= 8; ++row)
                for (const char col : {'A', 'B', 'F'}) {
                    const QString ref = QString(QLatin1Char(col)) + QString::number(row);
                    out << ref + QLatin1Char('=') + (col == 'B' ? savedFormula(e.path, ref) : savedCell(e.path, ref));
                }
            return out.join(QLatin1Char('|'));
        };
        const QString first = view();
        QVERIFY(s->insertRows(0, 1, 2).ok);
        QVERIFY(e.settle());
        QVERIFY(save());
        QVERIFY(view() != first);
        QVERIFY(s->canUndo());   // (an insert is undone after a save)
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:3"));
        QVERIFY(save());
        QCOMPARE(view(), first);
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("n:3"));
        QVERIFY(save());
        QVERIFY(view() != first);
        // a delete after the save: undone, and the file saved after is as it was
        const QString inserted = view();
        QVERIFY(s->deleteRows(0, 0, 2).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
        QVERIFY(save());
        QVERIFY(view() != inserted);
        QVERIFY(s->stepsForgottenBySave() > 0);   // (a delete before a save is not undone after it)
        QVERIFY(!s->canUndo());
        QVERIFY(s->insertRows(0, 0, 1).ok);       // (what is done after the save is undone as ever)
        QVERIFY(e.settle());
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
        QVERIFY(s->deleteRows(0, 1, 1).ok);       // (a delete after this save)
        QVERIFY(e.settle());
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(s->type(0, 0, 6, QStringLiteral("3")).ok);   // (as it was saved, the sheet is not modified: a file that is not changed is not saved)
        QVERIFY(e.settle());
        QVERIFY(save());
        QVERIFY(storeDiffers(e).isEmpty());
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
            QVERIFY2(savedOk(saved, 20000), "the save failed");
        };
        // refused: the header row, all of the table's rows and columns, a block that takes part of it with it
        QVERIFY(!s->deleteRows(0, 0, 1).ok);
        QVERIFY(!s->deleteRows(0, 0, 4).ok);
        QVERIFY(!s->deleteColumns(0, 0, 3).ok);
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
        // Undo goes back over the renaming, and then over the rows inserted before the table, which are in the saved file: the edit that undoes them is made against it
        QVERIFY(s->canUndo());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();                    // (the other window has the keys now)
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("E3")), QStringLiteral("=SUM(Sales[Q1])"));
        QVERIFY(s->canUndo());
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.settle());
        saveNow();
        QCOMPARE(tableRef(e.path), QStringLiteral("A1:C5|A1:C5"));
        Window third;
        QVERIFY(third.open(e.copyOfFile()));
        QCOMPARE(third.w.session()->editText(0, 0, 1), QStringLiteral("Q1"));
        QCOMPARE(differences(e.win.w.calculation(), third.w.calculation(), {}), QString());
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
            if (started && modified && !savedOk(saved)) return QStringLiteral("the save failed");
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            QMap<QString, QByteArray> parts;
            jp::readZip(f.readAll(), parts);
            f.close();   // (an open file can't be replaced on Windows: the next save would fail)
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

    // After a save, Undo does not go back over a delete of rows, columns, or cells made before it (the file has lost what was in them), but goes on over what came after, a row inserted among
    // it included (that Undo is made against the saved file); Redo of what was undone works.
    void undoAfterASaveStopsAtTheLastDeleteSaved()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1==A3"})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->deleteRows(0, 1, 1).ok);                                      // 1: a delete, saved below
        QVERIFY(s->insertRows(0, 20, 1).ok);                                     // 2: a row inserted after it, saved too
        QVERIFY(s->type(0, 0, 0, QStringLiteral("10")).ok);                      // 3: typing
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.settle());
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QVERIFY(s->deleteRows(0, 0, 1).ok);                                      // 4: a delete after the save
        QVERIFY(s->type(0, 4, 0, QStringLiteral("7")).ok);                       // 5
        QVERIFY(e.settle());
        QVERIFY(s->canUndo());
        pressUndo(&e.win.w);                                                     // 5 undone
        pressUndo(&e.win.w);                                                     // 4 undone
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A2"));
        QVERIFY(!e.win.w.isModified());
        QVERIFY(s->canUndo());                                                   // the typing (3) came after the delete (1) and can be undone
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QVERIFY2(s->canUndo(), "the row inserted (2) is undone against the saved file, though the file has it");
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QVERIFY(e.win.w.isModified());
        QVERIFY(!s->canUndo());                                                  // the delete (1) is in the file
        pressUndo(&e.win.w);
        QCOMPARE(e.win.w.undoStack()->index(), 1);
        pressRedo(&e.win.w);
        pressRedo(&e.win.w);
        pressRedo(&e.win.w);
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), 5);
        QVERIFY(e.win.w.isModified());
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(differences(e.win.w.calculation(), second.w.calculation(), {}), QString());
        QCOMPARE(second.w.session()->editText(0, 0, 1), QString());             // (B1 was in the row that was deleted)
        QCOMPARE(second.w.session()->editText(0, 0, 0), QStringLiteral("3"));
        QCOMPARE(second.w.session()->editText(0, 4, 0), QStringLiteral("7"));
    }

    // A pivot table sits on the cells of its sheet, where the file says: an edit that would move it or cut through it is refused (in plain words), and the rest of the sheet is edited as
    // ever.
    void aPivotTableIsNotMovedButTheRestOfItsSheetIs()
    {
        QByteArray book = sheetOf({"A1=1", "A2=2"});
        book = withParts(book, {{QStringLiteral("xl/pivotTables/pivotTable1.xml"), QByteArray("<pivotTableDefinition name=\"p\"><location ref=\"B3:E8\" firstHeaderRow=\"1\" firstDataRow=\"2\" firstDataCol=\"1\"/></pivotTableDefinition>"),
                                 QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"), 0, QStringLiteral("rIdP"), QStringLiteral("pivotTable"), QStringLiteral("../pivotTables/pivotTable1.xml")}});
        EditWindow e;
        QVERIFY(e.open(book));
        crunch::EditSession *s = e.win.w.session();
        using office::sheet::StructEdit;
        QString why;
        // (B3:E8 is rows 3 to 8, columns B to E)
        QVERIFY(s->canMove(StructEdit::insertRows(0, 10, 2), &why));     // below it
        QVERIFY(s->canMove(StructEdit::deleteRows(0, 20, 1), &why));
        QVERIFY(s->canMove(StructEdit::insertColumns(0, 6, 1), &why));   // right of it
        QVERIFY(s->canMove(StructEdit::deleteColumns(0, 8, 1), &why));
        QVERIFY(!s->canMove(StructEdit::insertRows(0, 5, 1), &why));     // inside it
        QVERIFY2(why.contains(QStringLiteral("pivot table")) && !why.contains(QStringLiteral("PivotTable")), qPrintable(why));
        QVERIFY(!s->canMove(StructEdit::insertRows(0, 0, 1), &why));     // above it: it would move
        QVERIFY(!s->canMove(StructEdit::deleteRows(0, 0, 1), &why));
        QVERIFY(!s->canMove(StructEdit::insertColumns(0, 0, 1), &why));
        QVERIFY(!s->canMove(StructEdit::deleteColumns(0, 3, 1), &why));
        CellRange block;
        block.valid = true;
        block.row1 = block.row2 = 9;
        block.col1 = block.col2 = 6;   // G10
        QVERIFY(s->canMove(StructEdit::move(0, block, 0, 10, 7), &why));     // elsewhere to elsewhere
        QVERIFY(!s->canMove(StructEdit::move(0, block, 0, 4, 2), &why));     // onto it
        block.row1 = block.row2 = 4;
        block.col1 = block.col2 = 3;   // D5, in it
        QVERIFY(!s->canMove(StructEdit::move(0, block, 0, 9, 9), &why));     // out of it
        block.row1 = 0;
        block.row2 = 1;
        block.col1 = block.col2 = 6;   // G1:G2
        QVERIFY(s->canMove(StructEdit::insertCellsDown(0, block), &why));    // columns it does not have
        block.col1 = block.col2 = 3;   // D1:D2: the cells below in column D, which has the pivot table
        QVERIFY(!s->canMove(StructEdit::insertCellsDown(0, block), &why));
        // the window says it in words
        e.view()->selectRows(0, 0);
        e.win.told.clear();
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("pivot table")), qPrintable(e.win.told.join('|')));
        e.view()->selectRows(11, 11);
        e.win.w.act(QStringLiteral("cells.insert"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // typing is as ever
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("5"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);
    }

    // Results worked out before rows and columns were edited name cells by the ids of their rows and columns, which those edits do not change: put where the cells are now (`placed`), they are on
    // the cells they were worked out for, and those of rows and columns that are deleted are left out.
    void resultsInFlightWhenRowsAndColumnsAreEditedLandOnTheirCells()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1"})));
        crunch::EditSession *s = e.win.w.session();
        auto item = [](quint32 row, quint32 col, double v) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = row;
            it.col = col;
            it.value = calc::Value::number(v);
            return it;
        };
        // (before any edit the ids are the places: the worker tells these as it works them out)
        std::vector<grid::ValueStore::Item> items{item(0, 1, 1), item(1, 1, 2), item(2, 1, 3), item(10, 1, 4), item(1, 5, 5), item(1, 6, 6)};
        const quint32 epochBefore = s->calculation()->structEpoch(0);
        QVERIFY(s->insertRows(0, 2, 3).ok);                                      // 3 rows at row 3
        QVERIFY(s->deleteColumns(0, 5, 1).ok);                                   // column F
        s->dropStale(&items);
        QCOMPARE(int(items.size()), 6);                                          // (nothing moved one by one: none is stale)
        const std::vector<grid::ValueStore::Item> where = s->placed(items);
        QCOMPARE(int(where.size()), 5);                                          // (the one in column F is of a column that is gone)
        QCOMPARE(where[0].row, quint32(0));
        QCOMPARE(where[2].row, quint32(5));                                      // (the third row's id is 2, and it is at row 6 now)
        QCOMPARE(where[3].row, quint32(13));
        QCOMPARE(where[4].col, quint32(5));                                      // (G moved to F)
        // and the store's value for a formula is the one the cell shows after the edits: the cell's ids say where
        QVERIFY(s->insertRows(0, 0, 2).ok);
        QVERIFY(e.settle());
        const office::sheet::Cell *b = s->calculation()->sheet(0).cells.find(2, 1);
        QVERIFY(b && b->hasFormula());
        calc::Value held;
        QVERIFY(e.win.w.values().find(0, b->rowId, b->colId, &held));
        QCOMPARE(held.number(), 1.0);
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:1"));
        QCOMPARE(s->calculation()->structEpoch(0), epochBefore);   // (none of the three edits could change what any formula holds or sees: each is rigid, and a result worked out before them is good)
    }

    // A result worked out before a rigid edit (nothing a formula reads or sees changed) is good after it: the epoch stays, the worker is asked nothing, and the window keeps it. An edit that is not
    // rigid for some formula makes the epoch one more, asks the worker once, and the formula it names is stamped: a result of it worked out before is let go of. (Counts are read after the worker
    // has settled.)
    void aResultWorkedOutBeforeARigidEditIsKeptAndOneForAFormulaTheWorkerIsAskedAboutIsLetGoOf()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A5)", "B2==COUNTBLANK(A1:A5)", "B3==A2*2"})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const office::sheet::Calculation *calc = s->calculation();
        QVERIFY(!calc->resultsInFlight());
        calc->valuesTaken(-1);   // (a value told and not taken is on its way)
        QVERIFY(calc->resultsInFlight());
        const quint32 before = calc->structEpoch(0);
        const qint64 evaluated = calc->evaluations(), asked = calc->questionsAnswered();
        auto item = [](quint32 rowId, quint32 epoch) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = rowId;
            it.col = 1;   // (column B, whose id is 1)
            it.epoch = epoch;
            it.value = calc::Value::number(5);
            return it;
        };
        // (the ids of the rows of B1, B2, and B3 are 0, 1, and 2 for as long as the sheet is open)
        QVERIFY(s->insertRows(0, 0, 1).ok);   // above everything: the blocks move down and take nothing in
        QVERIFY(e.settle());
        QCOMPARE(calc->structEpoch(0), before);
        QCOMPARE(calc->evaluations() - evaluated, qint64(0));
        QCOMPARE(calc->questionsAnswered(), asked);
        std::vector<grid::ValueStore::Item> items{item(0, before), item(1, before), item(2, before)};
        s->dropStale(&items);
        QCOMPARE(int(items.size()), 3);   // (all of them are good)
        QVERIFY(s->insertRows(0, 3, 1).ok);   // inside A2:A6, the blocks that were A1:A5: COUNTBLANK holds the gap and the sum does not mind
        const quint32 now = calc->structEpoch(0);
        QCOMPARE(now, before + 1);
        // (the worker answers on its own thread; no event is handled until it has, so the window has not yet been told it is done and the stamps are there: waited for by what it has answered)
        QElapsedTimer waited;
        waited.start();
        while (calc->questionsAnswered() == asked && waited.elapsed() < 30000) QThread::msleep(2);
        QCOMPARE(calc->questionsAnswered(), asked + 1);
        QVERIFY(calc->stampCount() > 0);
        items = {item(0, before), item(1, before), item(2, before), item(1, now)};
        s->dropStale(&items);
        QCOMPARE(int(items.size()), 3);   // (the one for B2 worked out before the edit is gone; the sum's, B3's, and B2's of the new epoch are kept)
        QCOMPARE(items[0].row, quint32(0));
        QCOMPARE(items[1].row, quint32(2));
        QCOMPARE(items[2].epoch, now);
        QVERIFY(e.settle());
        QCOMPARE(calc->evaluations() - evaluated, qint64(1));   // (COUNTBLANK, and no other)
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:15"));   // (the sum, which moved down a row)
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:1"));    // (COUNTBLANK counts the row put in)
        s->pruneStamps({now});
        QCOMPARE(calc->stampCount(), qint64(0));
    }

    // A formula that an edit of cells (moved one by one) put in another cell is stamped with the epoch of the edit, at the place it left and the place it came to: a result worked out under an older
    // epoch is let go of (it names the cell as it was), one worked out after it, and one for a cell the edit did not touch, are kept.
    void aResultOlderThanTheStampOfMovedCellsIsDiscarded()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1", "B2==A1*2", "C1=7"})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const quint32 before = s->calculation()->structEpoch(0);
        // (nothing is in flight now, so an edit stamps nothing; one value told and not taken is what a batch on its way is)
        QVERIFY(!s->calculation()->resultsInFlight());
        s->calculation()->valuesTaken(-1);
        QVERIFY(s->calculation()->resultsInFlight());
        const qint64 evaluated = s->calculation()->evaluations();
        office::sheet::CellRange block;
        block.valid = true;
        block.row1 = block.row2 = 0;
        block.col1 = block.col2 = 1;
        QVERIFY(s->insertCells(0, block, true).ok);                             // B1 and B2 move down one
        const quint32 now = s->calculation()->structEpoch(0);
        QVERIFY(now > before);
        QVERIFY(s->calculation()->stampCount() > 0);
        auto item = [](quint32 row, quint32 col, quint32 epoch) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = row;
            it.col = col;
            it.epoch = epoch;
            it.value = calc::Value::number(5);
            return it;
        };
        // (the ids of B1 and B2 before the edit were (0,1) and (1,1); B2 is at (2,1) now, B1 at (1,1))
        std::vector<grid::ValueStore::Item> items{item(0, 1, before), item(1, 1, before), item(2, 1, before), item(0, 0, before), item(1, 1, now), item(2, 1, now)};
        s->dropStale(&items);
        QCOMPARE(int(items.size()), 3);
        QCOMPARE(items[0].row, quint32(0));
        QCOMPARE(items[0].col, quint32(0));                                      // (A1, which the edit did not touch)
        QCOMPARE(items[1].epoch, now);
        QCOMPARE(items[2].epoch, now);
        // the formulas whose values were let go of are worked out again (those two, and no other), and what the worker then says is kept; once nothing older can be on its way the stamps are let go of
        // (no count is read before the worker has caught up: it may already have worked the two out)
        s->calculation()->valuesTaken(1);
        QVERIFY(e.settle());
        QCOMPARE(s->calculation()->evaluations() - evaluated, qint64(2));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:2"));
        s->pruneStamps({now});
        QCOMPARE(s->calculation()->stampCount(), qint64(0));
    }

    // An edit that is kept while the order of ids is put back to one run holds ids (the cells it rewrote) that the renumbering must reach: a column inserted (the formulas that read the columns after it
    // written again), then the rows put back to one run, then Undo and Redo of the column insert after a save (the marks go by those ids). What is saved is what a session whose ids were never
    // renumbered saves, cell for cell.
    void aRebaseRenumbersTheIdsKeptInTheRecordOfAColumnInsert()
    {
        bool rebased = false;
        QString problem;
        const QStringList plain = rebaseScript(false, 0, &rebased, &problem);
        QVERIFY2(plain.size() == 3, qPrintable(problem));
        const QStringList limited = rebaseScript(true, 0, &rebased, &problem);
        QVERIFY2(limited.size() == 3 && rebased, qPrintable(problem));
        for (int i = 0; i < 3; ++i) QVERIFY2(limited[i] == plain[i], qPrintable(QStringLiteral("the file saved at stage %1 differs").arg(i)));
        QVERIFY(plain[0] != plain[1] && plain[1] != plain[2]);   // (the three are three different files: the script does something)
    }

    // The same with each kind of edit that is kept: a row inserted while the columns are put back to one run, cells shifted down, and cells moved, while the rows are: values, marks, and the saved
    // file after Undo and Redo are those of a session that never renumbered.
    void aRebaseThenUndoAndRedoOfRowsCellsShiftedAndMovedAreRight()
    {
        for (const int kind : {1, 2, 3}) {
            bool rebased = false;
            QString problem;
            const QStringList plain = rebaseScript(false, kind, &rebased, &problem);
            QVERIFY2(plain.size() == 3, qPrintable(QStringLiteral("kind %1: %2").arg(kind).arg(problem)));
            const QStringList limited = rebaseScript(true, kind, &rebased, &problem);
            QVERIFY2(limited.size() == 3 && rebased, qPrintable(QStringLiteral("kind %1: %2").arg(kind).arg(problem)));
            for (int i = 0; i < 3; ++i) QVERIFY2(limited[i] == plain[i], qPrintable(QStringLiteral("kind %1: the file saved at stage %2 differs").arg(kind).arg(i)));
        }
    }

    // A mark on a cell whose row is deleted is no change to the file, and is no reason to ask for a full calculation when the file is opened: type in A5, delete row 5, and the session has no
    // changed cell and the snapshot has none.
    void aMarkOnADeletedRowIsNotAChangeAndAsksForNoCalculation()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==NOSUCHFUNCTION(A1)"})));   // (a formula the engine keeps apart: with a mark that counts, the file would be asked to be worked out)
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->calculation()->keptCount() > 0);
        QVERIFY(s->type(0, 4, 0, QStringLiteral("5")).ok);
        QCOMPARE(s->changedCells(), qint64(1));
        QVERIFY(s->deleteRows(0, 4, 1).ok);
        QCOMPARE(s->changedCells(), qint64(0));
        QCOMPARE(s->changedResults(), qint64(0));
        crunch::EditSession::Snapshot snap;
        QString why;
        QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
        QVERIFY(!snap.changes.fullCalcOnLoad);
        QVERIFY(snap.changes.sheets[0].cells.empty());   // (the formula above the deleted row reads nothing that moved, so it is not written; the suites run with the formulas in ids and in places, and this holds in both)
        e.win.w.undoStack()->undo();   // (the row comes back with its mark)
        QCOMPARE(s->changedCells(), qint64(1));
    }

    // With the formulas held in ids a save writes the formulas whose text is no longer the file's, whole (the saver is a later round's), which asks for no calculation of the file that the formulas in places
    // did not ask for: a row inserted below everything, and one above it, with a formula the engine keeps apart in the sheet.
    void theSaveStopgapOfTheIdFrameAsksForNoMoreCalculation()
    {
        struct Scope {
            bool was = jp::office::sheet::Calculation::idFrameDefault();
            ~Scope() { jp::office::sheet::Calculation::setIdFrameDefault(was); }   // (put back whatever fails)
        } scope;
        QList<bool> asks;
        QList<qint64> cellsWritten;
        for (const bool at0 : {false, true})
            for (const bool ids : {false, true}) {
                jp::office::sheet::Calculation::setIdFrameDefault(ids);
                EditWindow e;
                QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1==NOSUCHFUNCTION(A1)", "C1==A2+1", "C2==SUM(A1:A2)"})));
                QVERIFY(e.settle());
                crunch::EditSession *s = e.win.w.session();
                QCOMPARE(s->calculation()->idFrame(), ids);
                QVERIFY(s->insertRows(0, at0 ? 0 : 30, 1).ok);
                QVERIFY(e.settle());
                crunch::EditSession::Snapshot snap;
                QString why;
                QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
                asks << snap.changes.fullCalcOnLoad;
                cellsWritten << qint64(snap.changes.sheets.count(0) ? snap.changes.sheets[0].cells.size() : 0);
            }
        QCOMPARE(asks[1], asks[0]);   // (below)
        QCOMPARE(asks[3], asks[2]);   // (above)
        // the formulas held in ids are written whole when their text is no longer the file's, which are the cells the formulas in places write again: none for a row put in below everything, and the three that
        // read the rows that moved for one put in above
        QCOMPARE(cellsWritten[1], cellsWritten[0]);
        QCOMPARE(cellsWritten[3], cellsWritten[2]);
        QCOMPARE(cellsWritten[1], qint64(0));
        QCOMPARE(cellsWritten[3], qint64(3));
    }

    // Cells shifted down under ten thousand formulas, with no value on its way, work none of them out again and make no stamp (the formulas read cells that did not move).
    void cellsShiftedOverTenThousandFormulasWorkNoneOfThemOutAgain()
    {
        EditWindow e;
        QStringList specs;
        for (int r = 1; r <= 10000; ++r) specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(!s->calculation()->resultsInFlight());
        const qint64 evaluated = s->calculation()->evaluations();
        office::sheet::CellRange block;
        block.valid = true;
        block.row1 = block.row2 = 0;
        block.col1 = block.col2 = 1;
        QVERIFY(s->insertCells(0, block, true).ok);   // column B moves down one: ten thousand formulas
        QCOMPARE(s->calculation()->stampCount(), qint64(0));   // (no value on its way: no stamp is made, for none can be needed)
        QVERIFY(e.settle());
        QCOMPARE(s->calculation()->evaluations() - evaluated, qint64(0));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("B10001")), QStringLiteral("n:20000"));
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // After the ids of a sheet are renumbered, a value worked out before (and not yet taken) names cells by the old ids, whatever the cell (the cells of an array are never stamped): every one of an
    // older epoch is let go of, one of the epoch the renumbering made is kept.
    void valuesInFlightBeforeARebaseAreDroppedWhateverTheirCell()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1", "C1==B1+1"})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        jp::calc::LineLimits rows = jp::calc::LineLimits::rows();
        rows.idLimit = rows.firstNew + 10;
        s->setLineLimitsForTest(0, rows, jp::calc::LineLimits::cols());
        const quint32 old = s->calculation()->structEpoch(0);
        QVERIFY(s->insertRows(0, 1, 3).ok);
        QVERIFY(s->insertRows(0, 3, 3).ok);
        const quint32 used = s->calculation()->sheet(0).rows().nextId().value();
        QVERIFY(s->insertRows(0, 0, 3).ok);   // (too few ids are left: the order is put back to one run first)
        QVERIFY2(s->calculation()->sheet(0).rows().nextId().value() < used, "the ids were not renumbered");
        auto item = [](quint32 row, quint32 col, quint32 epoch) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = row;
            it.col = col;
            it.epoch = epoch;
            it.value = calc::Value::number(5);
            return it;
        };
        const quint32 now = s->calculation()->structEpoch(0);
        std::vector<grid::ValueStore::Item> items{item(0, 0, old), item(7, 2, old), item(40, 1, old), item(1, 1, now), item(2, 2, now)};
        s->dropStale(&items);
        QCOMPARE(int(items.size()), 2);
        QCOMPARE(items[0].epoch, now);
        QVERIFY(e.settle());
        QVERIFY(storeDiffers(e).isEmpty());   // (and the formulas were worked out again: nothing stale stays)
    }

    // The cells of an array formula are told with it and never stamped: after the ids of the sheet are renumbered, a value in flight for one of them (named by the old ids) is let go of by the epoch,
    // and one of the epoch the renumbering made is kept.
    void anArrayFormulasMemberCellsGetNoValueOfBeforeARebase()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"array\" ref=\"B1:B3\">A1:A3*2</f><v>2</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"B2\"><v>4</v></c></row>"
                                            "<row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"B3\"><v>6</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray())));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const office::sheet::Cell *member = s->calculation()->sheet(0).cells.find(1, 1);   // B2, a cell of the array
        QVERIFY(member && (member->flags & office::sheet::ArrayMember));
        jp::calc::LineLimits rowLimits = jp::calc::LineLimits::rows();
        rowLimits.idLimit = rowLimits.firstNew + 10;
        s->setLineLimitsForTest(0, rowLimits, jp::calc::LineLimits::cols());
        const quint32 old = s->calculation()->structEpoch(0);
        QVERIFY(s->insertRows(0, 5, 3).ok);
        QVERIFY(s->insertRows(0, 8, 3).ok);
        const quint32 used = s->calculation()->sheet(0).rows().nextId().value();
        QVERIFY(s->insertRows(0, 20, 3).ok);   // (too few ids are left: the order is put back to one run first)
        QVERIFY2(s->calculation()->sheet(0).rows().nextId().value() < used, "the ids were not renumbered");
        member = s->calculation()->sheet(0).cells.find(1, 1);
        QVERIFY(member && (member->flags & office::sheet::ArrayMember));
        auto item = [&](quint32 epoch) {
            grid::ValueStore::Item it;
            it.sheet = 0;
            it.row = member->rowId;
            it.col = member->colId;
            it.epoch = epoch;
            it.value = calc::Value::number(99);
            return it;
        };
        std::vector<grid::ValueStore::Item> items{item(old), item(s->calculation()->structEpoch(0))};
        s->dropStale(&items);
        QCOMPARE(int(items.size()), 1);
        QCOMPARE(items[0].epoch, s->calculation()->structEpoch(0));
        QCOMPARE(s->calculation()->stampCount(), qint64(0));   // (no stamp was made for it: the epoch is what lets go of the value)
    }

    // Cells moved (a Move) over ten thousand formulas, with no value on its way, make no stamp. (The formulas it moves are made again where they land, as they always were, so they are worked out,
    // once each: a shift keeps the formulas and works none out, a Move does not.)
    void cellsMovedOverTenThousandFormulasMakeNoStamp()
    {
        EditWindow e;
        QStringList specs;
        for (int r = 1; r <= 10000; ++r) specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(!s->calculation()->resultsInFlight());
        const qint64 evaluated = s->calculation()->evaluations();
        office::sheet::CellRange block;
        block.valid = true;
        block.row1 = 0;
        block.row2 = 9999;
        block.col1 = block.col2 = 1;
        QVERIFY(s->moveCells(0, block, 0, 0, 3).ok);   // column B to column D
        QCOMPARE(s->calculation()->stampCount(), qint64(0));
        QVERIFY(e.settle());
        QCOMPARE(s->calculation()->evaluations() - evaluated, qint64(10000));   // (the formulas it moved, once each, and no other)
        QCOMPARE(s->calculation()->stampCount(), qint64(0));
        QCOMPARE(e.valueAt(QStringLiteral("D10000")), QStringLiteral("n:20000"));
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // A scalar result is told by the ids its node holds: no place is looked up for it. After edits of rows (a sheet whose order is no longer the identity), working out every formula again looks up
    // two places for each formula evaluated (what a formula that reads where it is needs), and none for what it tells.
    void aScalarResultIsToldWithoutLookingUpItsPlace()
    {
        EditWindow e;
        QStringList specs;
        for (int r = 1; r <= 300; ++r) specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 3, 2).ok);
        QVERIFY(s->insertRows(0, 100, 2).ok);
        QVERIFY(e.settle());
        const qint64 evaluated = s->calculation()->evaluations(), looked = s->calculation()->placeLookups();
        QVERIFY(e.win.w.workbookLoader().recalculate());
        QVERIFY(QTest::qWaitFor([&] { return s->calculation()->evaluations() - evaluated >= 300; }, 20000));
        QVERIFY(e.settle());
        const qint64 evals = s->calculation()->evaluations() - evaluated;
        QVERIFY(evals >= 300);
        QCOMPARE(s->calculation()->placeLookups() - looked, 2 * evals);
    }

    // Marks are looked at to let go of those on dead ids only when records of lines are let go of (and only the marks on ids of those lines are asked about): thirty edits of rows that let go of
    // nothing look at none, and the edit that drops the undone ones does, and loses no mark.
    void marksAreSweptOnlyWhenLinesAreLetGoOf()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        crunch::EditSession *s = e.win.w.session();
        for (quint32 r = 0; r < 30; ++r) QVERIFY(s->type(0, r, 3, QString::number(r)).ok);
        QCOMPARE(s->changedCells(), qint64(30));
        for (int i = 0; i < 30; ++i) QVERIFY((i % 2 ? s->deleteRows(0, 2, 1) : s->insertRows(0, 2, 1)).ok);
        QCOMPARE(s->marksSwept(), qint64(0));
        QVERIFY(s->insertRows(0, 40, 2).ok);
        e.win.w.undoStack()->undo();
        QCOMPARE(s->marksSwept(), qint64(0));          // (an Undo lets go of nothing either)
        QVERIFY(s->insertRows(0, 0, 1).ok);            // (the undone step is dropped, and its ids may be given again)
        QVERIFY(s->marksSwept() > 0);
        QCOMPARE(s->changedCells(), qint64(30));        // (no mark was lost: the rows deleted were the empty ones just inserted)
    }

    // An edit made while the worker is at work (a batch of values is on its way): after each of a row inserted over formulas, cells shifted, and the order of rows put back to one run, what the window
    // shows once the worker has caught up is what opening the saved file and working everything out from nothing shows, and nothing stale stays.
    void anEditMadeWhileTheWorkerIsAtWorkLeavesTheValuesOfAFullRecalculation()
    {
        QStringList specs;
        for (int r = 1; r <= 12000; ++r) specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r) << QStringLiteral("C%1==B%1+1").arg(r);
        EditWindow e;
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s);
        jp::calc::LineLimits rows = jp::calc::LineLimits::rows();
        rows.idLimit = rows.firstNew + 12;
        s->setLineLimitsForTest(0, rows, jp::calc::LineLimits::cols());
        office::sheet::CellRange block;
        block.valid = true;
        block.row1 = block.row2 = 0;
        block.col1 = block.col2 = 1;
        // (a value told and not taken is a batch on its way: one is made so, by the count the window keeps, whatever the worker has got to, so that nothing here waits for a time or for the worker)
        const office::sheet::Calculation *calc = s->calculation();
        calc->valuesTaken(-1);
        QVERIFY(calc->resultsInFlight());
        QVERIFY(s->insertRows(0, 5, 3).ok);                    // formulas are written again
        QVERIFY(calc->resultsInFlight());
        QVERIFY(s->insertRows(0, 2, 5).ok);                    // (ids are used up: the order is put back to one run first)
        QVERIFY(s->insertRows(0, 9, 5).ok);
        QVERIFY(calc->resultsInFlight());
        QVERIFY(s->insertCells(0, block, true).ok);            // the last: a cell shift (column B down by one), with nothing written again after it to put right a stale value
        QVERIFY(calc->stampCount() > 0);                       // (values in flight: the moved cells are stamped)
        QVERIFY(e.settle());
        QVERIFY(storeDiffers(e).isEmpty());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        Window fresh;
        QVERIFY(fresh.open(e.copyOfFile()));   // (opened and worked out from nothing)
        const office::sheet::Calculation *mine = s->calculation(), *theirs = fresh.w.calculation();
        QString problem;
        int checked = 0;
        mine->sheet(0).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &c) {
            if (!problem.isEmpty() || !c.hasFormula()) return;
            ++checked;
            calc::Value shown;
            const calc::Value expected = theirs->cell(0, row, col);
            if (!e.win.w.values().find(0, c.rowId, c.colId, &shown) || !sameValue(shown, expected))
                problem = QStringLiteral("%1 shows %2 and should show %3").arg(cellName(row, col), shown.debug(), expected.debug());
        });
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QVERIFY(checked > 20000);
    }

    // Cells the person typed in are marked by the ids of their rows and columns: 200 edits of rows and columns later they are where adjusting each place edit by edit says, those whose row or column was
    // deleted are not in the file, and what the file is made of is the same cells with the same values (and the file opened again is what the window shows).
    void touchedCellsSurvive200RandomEdits()
    {
        using office::sheet::StructEdit;
        int dead = 0;
        for (const quint32 seed : {7u, 8u, 9u}) {
            Lcg rng(seed);
            EditWindow e;
            QStringList specs;
            for (int r = 1; r <= 12; ++r) specs << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
            QVERIFY(e.open(sheetOf(specs)));
            crunch::EditSession *s = e.win.w.session();
            struct Typed {
                quint32 row, col;
                bool alive;
                double value;
            };
            std::vector<Typed> typed, formulas;   // (the cells typed in, and the formulas of column B the file has: where each is after every edit, by adjusting its place edit by edit)
            for (quint32 r = 0; r < 12; ++r) formulas.push_back({r, 1, true, 0});
            for (quint32 r = 0; r < 12; r += 2)
                for (quint32 c = 2; c < 8; c += 2) {
                    typed.push_back({r, c, true, double(100 * r + c)});
                    QVERIFY(s->type(0, r, c, QString::number(typed.back().value)).ok);
                }
            int made = 0;
            for (int step = 0; step < 200; ++step) {
                const quint32 at = quint32(rng.bounded(14)), count = 1 + quint32(rng.bounded(2));
                StructEdit edit;
                crunch::EditSession::Outcome o;
                switch (rng.bounded(10)) {
                case 0: case 1: case 2: case 8: case 9: edit = StructEdit::insertRows(0, at, count); o = s->insertRows(0, at, count); break;
                case 3: edit = StructEdit::deleteRows(0, at, 1); o = s->deleteRows(0, at, 1); break;
                case 4: case 5: case 6: edit = StructEdit::insertColumns(0, at % 7, count); o = s->insertColumns(0, at % 7, count); break;
                default: edit = StructEdit::deleteColumns(0, at % 7, 1); o = s->deleteColumns(0, at % 7, 1); break;
                }
                if (!o.ok) continue;
                ++made;
                for (std::vector<Typed> *list : {&typed, &formulas})
                    for (Typed &t : *list) {
                        int sheet = 0;
                        if (t.alive && office::sheet::adjustCell(edit, &sheet, &t.row, &t.col) == office::sheet::RefFate::Gone) t.alive = false;
                    }
            }
            QVERIFY(made > 150);
            QVERIFY(e.settle());
            crunch::EditSession::Snapshot snap;
            QString why;
            QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
            const std::vector<office::sheet::SavedCell> &cells = snap.changes.sheets[0].cells;
            // (the cells are in order of place, and each is a cell typed in that is alive, at its place, with its value, or a formula of the file's that is alive)
            int constants = 0, aliveTyped = 0;
            for (size_t i = 1; i < cells.size(); ++i) QVERIFY(cells[i - 1].row < cells[i].row || (cells[i - 1].row == cells[i].row && cells[i - 1].col < cells[i].col));
            for (const Typed &t : typed) aliveTyped += t.alive ? 1 : 0;
            for (const office::sheet::SavedCell &c : cells) {
                if (c.isFormula) {
                    bool known = false;
                    for (const Typed &f : formulas) known = known || (f.alive && f.row == c.row && f.col == c.col);
                    QVERIFY2(known, qPrintable(QStringLiteral("seed %1: a formula at %2 that is none of the file's").arg(seed).arg(cellName(c.row, c.col))));
                    continue;
                }
                ++constants;
                const Typed *at = nullptr;
                for (const Typed &t : typed)
                    if (t.alive && t.row == c.row && t.col == c.col) at = &t;
                QVERIFY2(at, qPrintable(QStringLiteral("seed %1: a cell at %2 that was not typed in").arg(seed).arg(cellName(c.row, c.col))));
                QCOMPARE(c.value.number, at->value);
            }
            QCOMPARE(constants, aliveTyped);
            QVERIFY(aliveTyped > 0);
            dead += int(typed.size()) - aliveTyped;
            // and the file made of them is what the window shows
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved, 20000), "the save failed");
            Window second;
            QVERIFY(second.open(e.copyOfFile()));
            QVERIFY(differences(e.win.w.calculation(), second.w.calculation(), {}).isEmpty());
            for (const Typed &t : typed)
                if (t.alive) QCOMPARE(second.w.calculation()->content(0, t.row, t.col).value.number(), t.value);
        }
        QVERIFY(dead > 0);   // (some of the cells were deleted with their rows or columns, and are not in the file)
    }

    // The order of the columns is put back to one run and the ids are renumbered (small limits make it happen in a few edits): the values the window holds are on the cells they were for (at once, before
    // the worker has told anything again), the marks of the cells typed in are on the cells they were for, and the rest of what was held by the ids is renumbered with them.
    void valuesAndMarksSurviveACompaction()
    {
        using office::sheet::StructEdit;
        EditWindow e;
        QStringList specs;
        for (int r = 1; r <= 6; ++r) specs << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1==%2+0.5").arg(r).arg(r * 10);   // (formulas that read nothing: each has a value of its own)
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        jp::calc::LineLimits cols = jp::calc::LineLimits::cols();
        cols.idLimit = cols.firstNew + 40;
        cols.rebaseRuns = 8;
        cols.maxRuns = 100;
        s->setLineLimitsForTest(0, jp::calc::LineLimits::rows(), cols);
        struct Typed {
            quint32 row, col;
            bool alive;
            double value;
        };
        std::vector<Typed> typed;
        for (quint32 r = 0; r < 6; r += 2)
            for (quint32 c = 2; c < 4; ++c) {
                typed.push_back({r, c, true, double(10 * r + c)});
                QVERIFY(s->type(0, r, c, QString::number(typed.back().value)).ok);
            }
        QVERIFY(e.settle());
        Lcg rng(20261103);
        int compactions = 0, edits = 0, formulasSeen = 0;
        quint32 lastNext = s->calculation()->sheet(0).cols().nextId().value();
        for (int step = 0; step < 120; ++step) {
            const quint32 at = quint32(rng.bounded(7));
            const bool insert = rng.bounded(10) < 7;   // (more are inserted than deleted: the cells typed in and the formulas drift right, out of the way of the deletes, and stay)
            const StructEdit edit = insert ? StructEdit::insertColumns(0, at, 1) : StructEdit::deleteColumns(0, at, 1);
            const crunch::EditSession::Outcome o = insert ? s->insertColumns(0, at, 1) : s->deleteColumns(0, at, 1);
            if (!o.ok) continue;
            ++edits;
            for (Typed &t : typed) {
                int sheet = 0;
                if (t.alive && office::sheet::adjustCell(edit, &sheet, &t.row, &t.col) == office::sheet::RefFate::Gone) t.alive = false;
            }
            const quint32 next = s->calculation()->sheet(0).cols().nextId().value();
            const bool compacted = next < lastNext;
            lastNext = next;
            compactions += compacted ? 1 : 0;
            // (no event is processed: what the window holds is what the edit and the compaction left, with nothing the worker told since)
            const office::sheet::Calculation *calc = s->calculation();
            calc->sheet(0).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &c) {
                if (!c.hasFormula()) return;
                calc::Value held;
                QVERIFY2(e.win.w.values().find(0, c.rowId, c.colId, &held), qPrintable(QStringLiteral("step %1: no value for %2").arg(step).arg(cellName(row, col))));
                QVERIFY2(sameValue(held, calc->cell(0, row, col)), qPrintable(QStringLiteral("step %1: %2 shows %3").arg(step).arg(cellName(row, col), held.debug())));
                ++formulasSeen;
            });
            if (QTest::currentTestFailed()) return;
            if (compacted) {
                // the cells typed in are where they were, with the marks they had
                crunch::EditSession::Snapshot snap;
                QString why;
                QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
                int constants = 0, alive = 0;
                for (const Typed &t : typed) alive += t.alive ? 1 : 0;
                for (const office::sheet::SavedCell &c : snap.changes.sheets[0].cells) {
                    if (c.isFormula) continue;
                    ++constants;
                    bool found = false;
                    for (const Typed &t : typed) found = found || (t.alive && t.row == c.row && t.col == c.col && t.value == c.value.number);
                    QVERIFY2(found, qPrintable(QStringLiteral("step %1: a marked cell at %2").arg(step).arg(cellName(c.row, c.col))));
                }
                QCOMPARE(constants, alive);
            }
            QVERIFY(e.settle());
            QVERIFY(storeDiffers(e).isEmpty());
        }
        QVERIFY2(compactions >= 2, qPrintable(QString::number(compactions)));
        QVERIFY(edits > 60);
        QVERIFY2(formulasSeen > 60 * 3, qPrintable(QString::number(formulasSeen)));   // (the formulas were there to be looked at)
    }

    // Undo and Redo of edits of rows and columns put no mark and no value on another cell: after each, the cells typed in are where the edits done so far put them (adjusting each place edit by edit), the
    // file made from them is those cells, and the window's values are the calculation's.
    void undoAndRedoOfRowAndColumnEditsKeepValuesAndMarksOnTheirCells()
    {
        using office::sheet::StructEdit;
        EditWindow e;
        QStringList specs;
        for (int r = 1; r <= 8; ++r) specs << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1==A%1+C%1").arg(r);
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        struct Typed {
            quint32 row, col;
            double value;
        };
        std::vector<Typed> typed;
        for (quint32 r = 0; r < 8; r += 3)
            for (quint32 c = 2; c < 4; ++c) {
                typed.push_back({r, c, double(10 * r + c)});
                QVERIFY(s->type(0, r, c, QString::number(typed.back().value)).ok);
            }
        QVERIFY(e.settle());
        std::vector<StructEdit> done, undone;
        Lcg rng(11);
        auto check = [&](int step) -> void {
            QVERIFY(e.settle());
            std::vector<Typed> where = typed;
            std::vector<bool> alive(where.size(), true);
            for (const StructEdit &edit : done)
                for (size_t i = 0; i < where.size(); ++i) {
                    int sheet = 0;
                    if (alive[i] && office::sheet::adjustCell(edit, &sheet, &where[i].row, &where[i].col) == office::sheet::RefFate::Gone) alive[i] = false;
                }
            crunch::EditSession::Snapshot snap;
            QString why;
            QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
            int constants = 0, expected = 0;
            for (size_t i = 0; i < where.size(); ++i) expected += alive[i] ? 1 : 0;
            for (const office::sheet::SavedCell &c : snap.changes.sheets[0].cells) {
                if (c.isFormula) {
                    // (the formulas the edits wrote again are written whole, and only those: with every edit undone, none is)
                    QVERIFY2(!done.empty(), qPrintable(QStringLiteral("step %1: a formula at %2 is marked with no edit done").arg(step).arg(cellName(c.row, c.col))));
                    continue;
                }
                ++constants;
                bool found = false;
                for (size_t i = 0; i < where.size(); ++i) found = found || (alive[i] && where[i].row == c.row && where[i].col == c.col && where[i].value == c.value.number);
                QVERIFY2(found, qPrintable(QStringLiteral("step %1: a marked cell at %2 holding %3").arg(step).arg(cellName(c.row, c.col)).arg(c.value.number)));
            }
            QCOMPARE(constants, expected);
            const QString stale = storeDiffers(e);
            QVERIFY2(stale.isEmpty(), qPrintable(QStringLiteral("step %1: %2").arg(step).arg(stale)));
        };
        for (int step = 0; step < 60; ++step) {
            const int roll = rng.bounded(10);
            if (roll < 3 && !done.empty()) {
                e.win.w.undoStack()->undo();
                undone.push_back(done.back());
                done.pop_back();
            } else if (roll < 5 && !undone.empty()) {
                e.win.w.undoStack()->redo();
                done.push_back(undone.back());
                undone.pop_back();
            } else {
                const quint32 at = quint32(rng.bounded(10));
                StructEdit edit;
                crunch::EditSession::Outcome o;
                switch (rng.bounded(4)) {
                case 0: edit = StructEdit::insertRows(0, at, 1); o = s->insertRows(0, at, 1); break;
                case 1: edit = StructEdit::deleteRows(0, at, 1); o = s->deleteRows(0, at, 1); break;
                case 2: edit = StructEdit::insertColumns(0, at % 5, 1); o = s->insertColumns(0, at % 5, 1); break;
                default: edit = StructEdit::deleteColumns(0, at % 5, 1); o = s->deleteColumns(0, at % 5, 1); break;
                }
                if (!o.ok) continue;
                done.push_back(edit);
                undone.clear();
            }
            check(step);
            if (QTest::currentTestFailed()) return;
        }
        QVERIFY(!done.empty());
        // and every edit undone, then done again
        while (!done.empty()) {
            e.win.w.undoStack()->undo();
            undone.push_back(done.back());
            done.pop_back();
        }
        check(100);
        if (QTest::currentTestFailed()) return;
        while (!undone.empty()) {
            e.win.w.undoStack()->redo();
            done.push_back(undone.back());
            undone.pop_back();
        }
        check(101);
    }
};

JP_TEST_CLASS(CrunchCellsTests)
#include "tst_crunch_cells.moc"
