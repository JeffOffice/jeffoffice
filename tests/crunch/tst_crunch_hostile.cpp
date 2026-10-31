// Hostile sessions on rows and columns put in and taken out (Round 9 of docs/jeffcrunch-rowids.md): caps that refuse with their words and change nothing, ids that run out and are renumbered, the whole sheet
// deleted and brought back, ten thousand edits at the top and a save, a hundred thousand rows put in and filled, a long session of random edits under a small Undo budget. Every workbook is built here,
// in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"

#include <QElapsedTimer>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

#include <map>

using namespace jp;
using namespace crunchtest;

namespace {

using office::sheet::Calculation;
using office::sheet::StructEdit;

// A generator whose numbers are the same on every Qt.
struct Lcg {
    quint64 state;
    explicit Lcg(quint64 seed) : state(seed * 2862933555777941757ull + 3037000493ull) {}
    quint32 next()
    {
        state = state * 6364136223846793005ull + 1442695040888963407ull;
        return quint32(state >> 33);
    }
    quint32 bounded(quint32 n) { return next() % n; }
};

bool sameValue(const calc::Value &a, const calc::Value &b)
{
    if (a.isNumber() && b.isNumber()) return calc::sameTo15Digits(a.number(), b.number());
    if ((a.isEmpty() && b.isNumber() && b.number() == 0) || (b.isEmpty() && a.isNumber() && a.number() == 0)) return true;
    return a == b;
}

// What the window holds for the formulas against what the calculation has (the grid shows the first). Empty if they agree.
QString storeDiffers(EditWindow &e)
{
    if (!e.settle()) return QStringLiteral("the worker did not finish");
    const Calculation *calc = e.win.w.calculation();
    QString problem;
    for (int s = 0; s < calc->sheetCount() && problem.isEmpty(); ++s)
        calc->sheet(s).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &c) {
            if (!problem.isEmpty() || !c.hasFormula()) return;
            calc::Value held;
            if (!e.win.w.values().find(s, c.rowId, c.colId, &held))
                problem = QStringLiteral("%1!%2 has no value").arg(s).arg(cellName(row, col));
            else if (!sameValue(held, calc->cell(s, row, col)))
                problem = QStringLiteral("%1!%2 shows %3 and is %4").arg(s).arg(cellName(row, col), held.debug(), calc->cell(s, row, col).debug());
        });
    return problem;
}

// Every cell of the first sheet as "A1:formula-or-value", in order of place: what a person would see, for comparing two states.
QStringList cellsOf(const Calculation *calc)
{
    QStringList out;
    calc->sheet(0).cells.forEach([&](quint32 row, quint32 col, const office::sheet::Cell &) {
        const Calculation::CellContent c = calc->content(0, row, col);
        if (!c.exists) return;
        out << QStringLiteral("%1:%2=%3").arg(cellName(row, col), c.isFormula ? QLatin1Char('=') + c.formula : QString(), calc->cell(0, row, col).debug());
    });
    return out;
}

// Whether the save the spy was made for worked (a save of a small sheet may be done before it is waited for).
bool saveDone(QSignalSpy &spy)
{
    if (spy.isEmpty()) spy.wait(30000);
    return !spy.isEmpty() && spy.last().value(1).toBool();
}

// A sheet of `rows` rows: A a number, B a formula that doubles it, C a running total of B, and D a formula that reads the whole of A.
QByteArray tallBook(int rows)
{
    QStringList specs;
    for (int r = 1; r <= rows; ++r)
        specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r) << QStringLiteral("C%1==SUM(B$1:B%1)").arg(r);
    specs << QStringLiteral("D1==SUM(A:A)");
    return sheetOf(specs);
}

} // namespace

class CrunchHostileTests : public QObject {
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

    // An edit of rows or columns that the order cannot take (it has as many runs as it may, and putting it back to one run is not allowed) is refused in words a person can act on, before anything is
    // done: nothing moves and no step is pushed.
    void anEditPastTheCapOnRunsIsRefusedWithTheMessageAndChangesNothing()
    {
        for (const bool rows : {true, false}) {
            EditWindow e;   // (a window for each: limits are set on an order nothing has edited)
            QVERIFY(e.open(tallBook(20)));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            jp::calc::LineLimits tight = rows ? jp::calc::LineLimits::rows() : jp::calc::LineLimits::cols();
            tight.maxRuns = 12;
            tight.rebaseRuns = 1;   // (a rebase is wanted at once, and refused: the order is over that cap whatever it is renumbered to)
            s->setLineLimitsForTest(0, rows ? tight : jp::calc::LineLimits::rows(), rows ? jp::calc::LineLimits::cols() : tight);
            auto edit = [&](quint32 at) { return rows ? s->insertRows(0, at, 1) : s->insertColumns(0, at, 1); };
            int taken = 0;
            while (taken < 20 && edit(3).ok) ++taken;
            QVERIFY2(taken >= 1 && taken < 20, qPrintable(QStringLiteral("%1 edits were taken").arg(taken)));
            const QStringList before = cellsOf(s->calculation());
            const int steps = e.win.w.undoStack()->count();
            const crunch::EditSession::Outcome o = edit(3);
            QVERIFY2(!o.ok, "an edit past the cap was taken");
            QVERIFY(!o.confirm);
            QCOMPARE(o.problem, QCoreApplication::translate("LineMap", "This sheet has been edited too many times. Save it and open it again."));
            QCOMPARE(cellsOf(s->calculation()), before);
            QCOMPARE(e.win.w.undoStack()->count(), steps);
            QVERIFY(storeDiffers(e).isEmpty());
            QVERIFY(e.settle());
        }
    }

    // Column ids run short at the real caps (49,152 spare ids, rebased at four fifths of them): a hundred columns put in and taken out again, over and over, takes 200 ids each time, so the sheet is put back
    // to one run three times in 600 pairs. No edit is refused, every edit lands where it should, what came before the rebase can no longer be undone (and says so by not being offered), and what came
    // after can.
    void columnIdsRunShortAndTheSheetIsRebasedAtTheRealCaps()
    {
        EditWindow e;
        QVERIFY(e.open(tallBook(30)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const QStringList original = cellsOf(s->calculation());
        quint32 lastNext = s->calculation()->sheet(0).cols().nextId().value();
        quint32 mostNext = 0;
        int rebases = 0, pairs = 0;
        for (; pairs < 650 && rebases < 3; ++pairs) {
            const crunch::EditSession::Outcome in = s->insertColumns(0, 0, 100);
            QVERIFY2(in.ok && !in.confirm, qPrintable(QStringLiteral("pair %1: insert refused: %2").arg(pairs).arg(in.problem)));
            // (what was in column A is in column CW now)
            QCOMPARE(s->calculation()->content(0, 0, 100).value.number(), 1.0);
            QVERIFY(!s->calculation()->content(0, 0, 0).exists);
            const quint32 next = s->calculation()->sheet(0).cols().nextId().value();
            if (next < lastNext) ++rebases;
            lastNext = next;
            mostNext = std::max(mostNext, next);
            const crunch::EditSession::Outcome out = s->deleteColumns(0, 0, 100);
            QVERIFY2(out.ok && !out.confirm, qPrintable(QStringLiteral("pair %1: delete refused: %2").arg(pairs).arg(out.problem)));
            const quint32 after = s->calculation()->sheet(0).cols().nextId().value();
            if (after < lastNext) ++rebases;
            lastNext = after;
            mostNext = std::max(mostNext, after);
        }
        QVERIFY2(rebases >= 3, qPrintable(QStringLiteral("%1 rebases in %2 pairs").arg(rebases).arg(pairs)));
        QVERIFY2(mostNext < (1u << 16), qPrintable(QString::number(mostNext)));
        QVERIFY(e.settle());
        QCOMPARE(cellsOf(s->calculation()), original);
        QVERIFY(storeDiffers(e).isEmpty());
        // Undo goes back over what was kept and stops where the ids were renumbered: not over all 2 x pairs steps
        int undone = 0;
        while (s->canUndo() && undone < 3000) {
            e.win.w.undoStack()->undo();
            ++undone;
        }
        QVERIFY2(undone >= 1 && undone < 2 * pairs, qPrintable(QStringLiteral("%1 undone of %2").arg(undone).arg(2 * pairs)));
        QVERIFY(e.settle());
        QVERIFY(storeDiffers(e).isEmpty());
        while (e.win.w.undoStack()->canRedo()) e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(cellsOf(s->calculation()), original);
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // The whole sheet deleted (every row, then every column) and brought back by Undo: every cell, formula, and value is as it was, a save of it is what the cells were, Redo empties it again, and the sheet
    // takes edits after. The step holds the cells that are dead, not copies: the bytes it counts are small.
    void aWholeSheetDeletedIsBroughtBackByUndo()
    {
        EditWindow e;
        QVERIFY(e.open(tallBook(300)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const QStringList original = cellsOf(s->calculation());
        QVERIFY(original.size() > 900);
        const qint64 base = s->undoBytes();
        const crunch::EditSession::Outcome rows = s->deleteRows(0, 0, 1048576);
        QVERIFY2(rows.ok && !rows.confirm, qPrintable(rows.problem));
        const crunch::EditSession::Outcome cols = s->deleteColumns(0, 0, 16384);
        QVERIFY2(cols.ok && !cols.confirm, qPrintable(cols.problem));
        QVERIFY(e.settle());
        QVERIFY(cellsOf(s->calculation()).isEmpty());
        QVERIFY2(s->undoBytes() - base < 300 * 4 * 200 * 2 + 64 * 1024, qPrintable(QString::number(s->undoBytes() - base)));
        QVERIFY(storeDiffers(e).isEmpty());
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(cellsOf(s->calculation()), original);
        QVERIFY(storeDiffers(e).isEmpty());
        QSignalSpy spy(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saveDone(spy));
        QCOMPARE(savedFormula(e.path, QStringLiteral("B300")), QStringLiteral("A300*2"));
        QCOMPARE(savedFormula(e.path, QStringLiteral("C300")), QStringLiteral("SUM(B$1:B300)"));
        QCOMPARE(savedFormula(e.path, QStringLiteral("D1")), QStringLiteral("SUM(A:A)"));
        e.win.w.undoStack()->redo();
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QVERIFY(cellsOf(s->calculation()).isEmpty());
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(cellsOf(s->calculation()), original);
        QVERIFY(s->type(0, 0, 0, QStringLiteral("5")).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->calculation()->cell(0, 0, 1).number(), 10.0);
        QVERIFY(storeDiffers(e).isEmpty());
    }

    // Ten thousand rows put in at the top, one after another, and a save: no edit costs more work than the first (counted), the order has one run for each, and the file has every formula where it belongs, read
    // from the file and not from the window.
    void tenThousandEditsAtTheTopAndASave()
    {
        EditWindow e;
        QVERIFY(e.open(tallBook(60)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const Calculation *calc = s->calculation();
        const qint64 textsBefore = calc->sheet(0).formulaTexts.size();
        qint64 first = 0, last = 0;
        const int edits = 10000;
        for (int i = 0; i < edits; ++i) {
            const qint64 before = calc->editWork();
            const crunch::EditSession::Outcome o = s->insertRows(0, 0, 1);
            QVERIFY2(o.ok && !o.confirm, qPrintable(QStringLiteral("edit %1: %2").arg(i).arg(o.problem)));
            const qint64 work = calc->editWork() - before;
            if (i < 200) first = std::max(first, work);
            if (i >= edits - 200) last = std::max(last, work);
        }
        QVERIFY(e.settle());
        QVERIFY2(last <= 2 * first + 200, qPrintable(QStringLiteral("work of an edit: %1 at first, %2 at the end").arg(first).arg(last)));
        QVERIFY2(calc->sheet(0).cells.rows().runCount() >= size_t(edits) && calc->sheet(0).cells.rows().runCount() <= size_t(edits) + 4, qPrintable(QString::number(calc->sheet(0).cells.rows().runCount())));
        QVERIFY2(calc->sheet(0).formulaTexts.size() <= textsBefore + 4, qPrintable(QStringLiteral("%1 texts, %2 before").arg(calc->sheet(0).formulaTexts.size()).arg(textsBefore)));   // (the formulas that moved were not written again)
        QCOMPARE(calc->content(0, quint32(edits), 1).formula, QStringLiteral("A%1*2").arg(edits + 1));
        QVERIFY(storeDiffers(e).isEmpty());
        QSignalSpy spy(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saveDone(spy));
        QCOMPARE(savedFormula(e.path, QStringLiteral("B%1").arg(edits + 1)), QStringLiteral("A%1*2").arg(edits + 1));
        QCOMPARE(savedFormula(e.path, QStringLiteral("B%1").arg(edits + 60)), QStringLiteral("A%1*2").arg(edits + 60));
        QCOMPARE(savedFormula(e.path, QStringLiteral("C%1").arg(edits + 30)), QStringLiteral("SUM(B$%1:B%2)").arg(edits + 1).arg(edits + 30));
        QCOMPARE(savedFormula(e.path, QStringLiteral("D%1").arg(edits + 1)), QStringLiteral("SUM(A:A)"));
        QVERIFY(savedFormula(e.path, QStringLiteral("B1")) == QStringLiteral("-"));
    }

    // Rows put in and filled cost in proportion to their number: the formulas the graph names as readers of what changed, to work out 16,000 filled rows, are about four times those for 4,000 (a graph asked, for each of the
    // cells, for every formula in the rows put in names sixteen times as many). Counted, not timed.
    void filledRowsPutInCostInProportionToTheirNumber()
    {
        qint64 looked[2] = {0, 0};
        const quint32 sizes[2] = {qEnvironmentVariableIsSet("HA") ? quint32(qEnvironmentVariableIntValue("HA")) : 4000u, qEnvironmentVariableIsSet("HB") ? quint32(qEnvironmentVariableIntValue("HB")) : 16000u};
        for (int k = 0; k < 2; ++k) {
            EditWindow e;
            QStringList specs;
            for (int r = 1; r <= 20; ++r) specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
            QVERIFY(e.open(sheetOf(specs)));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            const Calculation *calc = s->calculation();
            QVERIFY(s->insertRows(0, 5, sizes[k]).ok);
            QVERIFY(s->type(0, 5, 0, QStringLiteral("7")).ok);
            QVERIFY(s->type(0, 5, 1, QStringLiteral("=A6*2")).ok);
            const qint64 before = calc->readersFound();
            for (const quint32 col : {0u, 1u}) {
                office::sheet::CellRange source;
                source.valid = true;
                source.row1 = source.row2 = 5;
                source.col1 = source.col2 = col;
                crunch::EditSession::FillOptions options;
                options.kind = crunch::EditSession::FillKind::Copy;
                QVERIFY(s->fill(0, source, crunch::EditSession::FillDirection::Down, sizes[k] - 1, options).ok);
            }
            QVERIFY(e.settle());
            QCOMPARE(calc->cell(0, 5 + sizes[k] - 1, 1).number(), 14.0);
            looked[k] = calc->readersFound() - before;
            qInfo("N %u: readers found %lld, graph work %lld", sizes[k], looked[k], calc->graph().queryWork());
        }
        QVERIFY2(looked[0] > 0 && looked[1] <= 6 * looked[0], qPrintable(QStringLiteral("%1 readers named for 4,000 rows and %2 for 16,000").arg(looked[0]).arg(looked[1])));
    }

    // A hundred thousand rows put in and filled: the formulas in them are held by the ids of the rows they are in (none in places; one shared text for the family), and an edit after writes none of them
    // again and visits next to none of them (counted).
    void aHundredThousandRowsPutInAndFilledAreHeldById()
    {
        EditWindow e;
        QStringList specs;
        for (int r = 1; r <= 20; ++r) specs << QStringLiteral("A%1=%1").arg(r) << QStringLiteral("B%1==A%1*2").arg(r);
        QVERIFY(e.open(sheetOf(specs)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const Calculation *calc = s->calculation();
        const quint32 count = 100000;
        QVERIFY(s->insertRows(0, 5, count).ok);
        QVERIFY(s->type(0, 5, 0, QStringLiteral("7")).ok);
        QVERIFY(s->type(0, 5, 1, QStringLiteral("=A6*2")).ok);
        for (const quint32 col : {0u, 1u}) {   // (a fill changes up to 100,000 cells: one column at a time)
            office::sheet::CellRange source;
            source.valid = true;
            source.row1 = source.row2 = 5;
            source.col1 = source.col2 = col;
            crunch::EditSession::FillOptions options;
            options.kind = crunch::EditSession::FillKind::Copy;
            const crunch::EditSession::Outcome filled = s->fill(0, source, crunch::EditSession::FillDirection::Down, count - 1, options);
            QVERIFY2(filled.ok, qPrintable(filled.problem));
        }
        QVERIFY(e.settle());
        QCOMPARE(calc->cell(0, 5 + count - 1, 1).number(), 14.0);
        QCOMPARE(calc->content(0, 5 + count - 1, 1).formula, QStringLiteral("A%1*2").arg(5 + count));
        QCOMPARE(calc->positionalFormulas(), qint64(0));
        QVERIFY2(calc->sheet(0).formulaTexts.size() < 50, qPrintable(QString::number(calc->sheet(0).formulaTexts.size())));
        for (const StructEdit &edit : {StructEdit::insertRows(0, 0, 1), StructEdit::deleteRows(0, 2, 1), StructEdit::insertColumns(0, 0, 1)}) {
            const Calculation::EditStats before = calc->editStats();
            const crunch::EditSession::Outcome o = edit.kind == StructEdit::Kind::InsertRows ? s->insertRows(0, edit.first, edit.count)
                : edit.kind == StructEdit::Kind::DeleteRows ? s->deleteRows(0, edit.first, edit.count)
                : s->insertColumns(0, edit.first, edit.count);
            QVERIFY2(o.ok && !o.confirm, qPrintable(o.problem));
            const Calculation::EditStats d = calc->editStats().since(before);
            QVERIFY2(d.formulasVisited <= 2, qPrintable(QStringLiteral("%1 formulas visited").arg(d.formulasVisited)));
            QCOMPARE(d.formulasRewritten, qint64(0));
            QCOMPARE(d.textsMade, qint64(0));
            QCOMPARE(d.eagerFormulas, qint64(0));
            QCOMPARE(d.nodesRemade, qint64(0));
            QCOMPARE(d.textsToIds, qint64(0));
        }
        QVERIFY(e.settle());
        // after those three (a row at the top, one out of the third, a column at the left) the last formula is at row 100,025 - 1 + 1 - 1 and column C
        QCOMPARE(calc->cell(0, 5 + count - 1, 2).number(), 14.0);
        QVERIFY(storeDiffers(e).isEmpty());
        // all of it undone, and done again
        for (int i = 0; i < 3; ++i) e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(calc->cell(0, 5 + count - 1, 1).number(), 14.0);
        QCOMPARE(calc->positionalFormulas(), qint64(0));
    }

    // A long session of random edits under a small Undo budget (60 KB): rows and columns put in and taken out, numbers and formulas typed, Undo and Redo. The bytes the steps hold stay under the budget (and
    // the newest step), the dead cells held are what the budget allows, the window's values are the calculation's after every step, and each step Undo goes back over gives the state recorded before it.
    void aSessionOfRandomEditsStaysUnderItsMemoryBudget()
    {
        EditWindow e;
        QVERIFY(e.open(tallBook(40)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const Calculation *calc = s->calculation();
        const qint64 budget = 60 * 1024;
        s->setUndoByteLimit(budget);
        Lcg rng(20261101);
        // the state before each step that was taken, by the index of the undo stack after it
        std::map<int, QStringList> stateAfter;
        stateAfter[e.win.w.undoStack()->index()] = cellsOf(calc);
        qint64 mostBytes = 0, mostDead = 0;
        int taken = 0, refused = 0, asked = 0, undone = 0, redone = 0;
        for (int i = 0; i < 400; ++i) {
            const quint32 roll = rng.bounded(100);
            const quint32 count = 1 + rng.bounded(roll < 10 ? 60 : 4);
            QUndoStack *stack = e.win.w.undoStack();
            crunch::EditSession::Outcome o;
            if (roll < 15 && s->canUndo()) {
                stack->undo();
                ++undone;
            } else if (roll < 27 && stack->canRedo()) {
                stack->redo();
                ++redone;
            } else if (roll < 40) {
                o = s->insertRows(0, rng.bounded(60), count);
            } else if (roll < 55) {
                o = s->deleteRows(0, rng.bounded(60), count);
            } else if (roll < 63) {
                o = s->insertColumns(0, rng.bounded(7), count);
            } else if (roll < 70) {
                o = s->deleteColumns(0, rng.bounded(7), count);
            } else if (roll < 85) {
                o = s->type(0, rng.bounded(60), 4 + rng.bounded(3), QString::number(double(rng.bounded(1000))));
            } else {
                o = s->type(0, rng.bounded(60), 4 + rng.bounded(3), QStringLiteral("=SUM(A%1:A%2)+B%3").arg(1 + rng.bounded(20)).arg(21 + rng.bounded(20)).arg(1 + rng.bounded(40)));
            }
            if (o.confirm) {
                ++asked;   // (a step that would hold more than the budget asks first; it is not taken here)
            } else if (!o.ok && !o.problem.isEmpty()) {
                ++refused;
            } else {
                ++taken;
            }
            QVERIFY(e.settle());
            QVERIFY2(storeDiffers(e).isEmpty(), qPrintable(QStringLiteral("step %1: %2").arg(i).arg(storeDiffers(e))));
            mostBytes = std::max(mostBytes, s->undoBytes());
            mostDead = std::max(mostDead, calc->sheet(0).cells.deadCount());
            QVERIFY2(s->undoBytes() <= budget + 80 * 1024, qPrintable(QStringLiteral("step %1: %2 bytes held").arg(i).arg(s->undoBytes())));
            QVERIFY2(calc->sheet(0).cells.deadCount() * office::sheet::Calculation::kUndoBytesPerDeadCell <= 2 * budget + 200 * 1024, qPrintable(QStringLiteral("step %1: %2 dead cells held").arg(i).arg(calc->sheet(0).cells.deadCount())));
            stateAfter[stack->index()] = cellsOf(calc);
        }
        QVERIFY2(taken > 200 && undone > 5 && redone >= 2, qPrintable(QStringLiteral("%1 taken, %2 undone, %3 redone, %4 refused, %5 asked").arg(taken).arg(undone).arg(redone).arg(refused).arg(asked)));
        // each step still held, undone, gives the state it was done from
        const int top = e.win.w.undoStack()->index();
        int back = 0;
        std::map<int, QStringList> recorded = stateAfter;
        for (int index = top; s->canUndo(); --index) {
            e.win.w.undoStack()->undo();
            ++back;
            QVERIFY(e.settle());
            QVERIFY(storeDiffers(e).isEmpty());
            const auto want = recorded.find(e.win.w.undoStack()->index());
            if (want != recorded.end()) QVERIFY2(cellsOf(calc) == want->second, qPrintable(QStringLiteral("undo to step %1 differs").arg(e.win.w.undoStack()->index())));
            if (back > 5000) break;
        }
        QVERIFY(back >= 3);
        QVERIFY2(mostBytes > 0, "the steps held nothing");
    }
};

JP_TEST_CLASS(CrunchHostileTests)
#include "tst_crunch_hostile.moc"
