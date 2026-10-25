// Tests of Find and Replace on the edit session: what is looked in (formulas, values, notes), the order, the scope, the options, the work a search does a slice at a time, and Replace as one
// step that never changes a cell it must not. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/finddialog.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/findrun.h"
#include "core/gotospecial.h"
#include "sheet/notes.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QToolButton>
#include <QTreeWidget>
#include <QUndoStack>
#include "../localepin.h"

#include <memory>

using namespace jp;
using namespace crunchtest;
using jp::crunch::FindOptions;
using jp::crunch::FindRun;
using jp::crunch::FoundCell;

namespace {

// Two sheets:
//   Sheet1  A1 apple   B1 Apple pie   C1 =A1&"!"
//           A2 banana  B2 25% (0.25)  C2 =B2*2
//           A3 cherry  B3 7           C3 a*b   D1 1234.5 (shown 1,234.50)
//   Sheet2  A1 apple   B2 apple tart
QByteArray fruitBook()
{
    return bookOfSheets({{"A1=apple", "B1=Apple pie", "C1==A1&\"!\"", "A2=banana", "B2=%0.25", "C2==B2*2", "A3=cherry", "B3=7", "C3=a*b", "D1=1234.5~5"}, {"A1=apple", "B2=apple tart"}});
}

// A sheet to go to cells of:
//      A    B    C         D
//  1   1    x    =A1+1     =1/0
//  2   2    y    =B2
//  3
//  4   5
//  5   6    z
QByteArray goBook(const QMap<int, QString> &rowAttrs = QMap<int, QString>(), const QString &after = QString())
{
    return bookOfSheets({{"A1=1", "B1=x", "C1==A1+1", "D1==1/0", "A2=2", "B2=y", "C2==B2", "A4=5", "A5=6", "B5=z"}, {"A1=other"}}, {{0, after}}, rowAttrs,
                        QStringLiteral("<definedName name=\"Sales\">Sheet1!$A$1:$B$2</definedName><definedName name=\"Total\">Sheet1!$A$1+1</definedName><definedName name=\"Local\" localSheetId=\"1\">Sheet2!$A$1</definedName>"));
}

QString rangesOf(const jp::crunch::SpecialResult &r)
{
    QStringList out;
    for (const CellRange &c : r.ranges) out << rangeName(c);
    return out.join(QLatin1Char(' '));
}

jp::crunch::SpecialResult special(EditWindow &e, jp::crunch::SpecialKind kind, const QString &selection = QString(), const QString &active = QStringLiteral("A1"), bool numbers = true, bool text = true,
                                  bool logicals = true, bool errors = true)
{
    jp::crunch::SpecialOptions o;
    o.kind = kind;
    o.numbers = numbers;
    o.text = text;
    o.logicals = logicals;
    o.errors = errors;
    quint32 r = 0, c = 0;
    parseCellRef(active, &r, &c);
    return jp::crunch::goToSpecial(*e.win.w.session(), 0, selection.isEmpty() ? CellRange() : rangeOf(selection), r, c, o);
}

QStringList names(const std::vector<FoundCell> &hits)
{
    QStringList out;
    for (const FoundCell &h : hits) out << QStringLiteral("%1!%2").arg(h.sheet + 1).arg(cellName(h.row, h.col));
    return out;
}

FindOptions options(const QString &text)
{
    FindOptions o;
    o.pattern.text = text;
    return o;
}

std::vector<FoundCell> findAll(EditWindow &e, const FindOptions &o)
{
    FindRun run(e.win.w.session(), o);
    run.step(0);
    return run.hits();
}

} // namespace

class CrunchFindTests : public QObject {
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
        m_localeBefore = QLocale();
        m_pin = std::make_unique<PinnedLocale>();   // (put back when the class is done)
    }

    void cleanupTestCase()
    {
        // the computer's language is back, whatever this class pinned (a class that is run after this one in the same process is not told it was English)
        m_pin.reset();
        QCOMPARE(QLocale().name(), m_localeBefore.name());
    }

    // ---- what is found
    void findAllLooksInTheSheetOrTheWorkbookByRowsOrColumns()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        e.win.w.session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        FindOptions o = options(QStringLiteral("a"));
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1", "1!B1", "1!C1", "1!A2", "1!C3"}));   // (formulas and entries: the formula in C1 reads A1)
        o.order = FindOptions::Order::ByColumns;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1", "1!A2", "1!B1", "1!C1", "1!C3"}));
        o.scope = FindOptions::Scope::Workbook;
        o.order = FindOptions::Order::ByRows;
        o.pattern.text = QStringLiteral("apple");
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1", "1!B1", "2!A1", "2!B2"}));
        // a search within a block
        o.scope = FindOptions::Scope::Sheet;
        o.within = rangeOf(QStringLiteral("B1:C3"));
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B1"}));
        // and in the other sheet
        o.within = CellRange();
        o.sheet = 1;
        QCOMPARE(names(findAll(e, o)), (QStringList{"2!A1", "2!B2"}));
    }

    void formulasAndValuesAreDifferentPlacesToLook()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        e.win.w.session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        FindOptions o = options(QStringLiteral("1,234"));
        QCOMPARE(names(findAll(e, o)), (QStringList{}));                       // (the cell holds 1234.5)
        o.lookIn = FindOptions::LookIn::Values;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!D1"}));                 // (and shows 1,234.50)
        o.pattern.text = QStringLiteral("1234.5");
        QCOMPARE(names(findAll(e, o)), (QStringList{}));
        o.lookIn = FindOptions::LookIn::Formulas;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!D1"}));
        o.lookIn = FindOptions::LookIn::Values;
        o.pattern.text = QStringLiteral("apple!");
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!C1"}));                 // (a formula shows its result)
        o.lookIn = FindOptions::LookIn::Formulas;
        QCOMPARE(names(findAll(e, o)), (QStringList{}));
        o.pattern.text = QStringLiteral("A1&");
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!C1"}));                 // (and holds its formula)
        o.lookIn = FindOptions::LookIn::Values;
        QCOMPARE(names(findAll(e, o)), (QStringList{}));
    }

    void caseWholeCellAndWildcardsAreOptions()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        FindOptions o = options(QStringLiteral("apple"));
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1", "1!B1"}));
        o.pattern.matchCase = true;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1"}));
        o.pattern.matchCase = false;
        o.pattern.wholeCell = true;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1"}));
        o.pattern.text = QStringLiteral("a*e");
        o.pattern.wholeCell = true;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A1", "1!B1"}));         // (apple, and Apple pie: it begins with an A and ends in an e)
        o.pattern.text = QStringLiteral("?anana");
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A2"}));
        o.pattern.wholeCell = false;
        o.pattern.text = QStringLiteral("a~*b");                               // the cell C3 holds the three characters a*b
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!C3"}));
        o.pattern.text = QStringLiteral("a*b");
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!C3"}));                 // (an a, then anything, then a b: banana has its b first)
    }

    void anEmptyPatternSearchesNothingAndSaysWhy()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        FindRun run(e.win.w.session(), options(QString()));
        QVERIFY(run.done());
        QVERIFY(!run.problem().isEmpty());
        QVERIFY(run.hits().empty());
    }

    // ---- Find Next
    void findNextGoesOnInTheOrderAndWrapsRound()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        FindOptions o = options(QStringLiteral("apple"));
        o.scope = FindOptions::Scope::Workbook;
        FoundCell at;
        QStringList seen;
        for (int i = 0; i < 5; ++i) {
            FindRun run(e.win.w.session(), o, FindRun::Want::Next, at);
            run.step(0);
            QVERIFY(run.found());
            at = run.hits().front();
            seen << names(run.hits());
        }
        QCOMPARE(seen, (QStringList{"1!B1" /* after A1 (the start) */, "2!A1", "2!B2", "1!A1", "1!B1"}));
        // by columns: down column A first
        o.order = FindOptions::Order::ByColumns;
        o.scope = FindOptions::Scope::Sheet;
        o.pattern.text = QStringLiteral("a");
        FoundCell a2;
        a2.row = 1;
        a2.col = 0;
        FindRun run(e.win.w.session(), o, FindRun::Want::Next, a2);
        run.step(0);
        QCOMPARE(names(run.hits()), (QStringList{"1!B1"}));
        // nothing to find
        o.pattern.text = QStringLiteral("zzz");
        FindRun none(e.win.w.session(), o, FindRun::Want::Next, a2);
        none.step(0);
        QVERIFY(none.done() && !none.found());
    }

    void aSearchIsDoneASliceAtATimeAndCountsTheCellsItLooksAt()
    {
        QStringList cells;
        for (int r = 1; r <= 300; ++r) cells << QStringLiteral("A%1=item %2").arg(r).arg(r) << QStringLiteral("B%1=%2").arg(r).arg(r);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        FindOptions o = options(QStringLiteral("item 2"));
        FindRun run(e.win.w.session(), o);
        int slices = 0;
        while (!run.step(100)) {
            QVERIFY(++slices < 100);
            QCOMPARE(run.cellsLooked(), qint64(slices) * 100);
        }
        QCOMPARE(run.cellsLooked(), qint64(600));
        QVERIFY(slices >= 5);
        QCOMPARE(int(run.hits().size()), 111);   // item 2, item 20 to 29, item 200 to 299
        // the same hits as a search made at once
        QCOMPARE(names(run.hits()), names(findAll(e, o)));
    }

    void aSearchStopsAtTheMostHitsItKeeps()
    {
        QStringList cells;
        for (int r = 1; r <= 200; ++r) cells << QStringLiteral("A%1=x").arg(r);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        FindRun run(e.win.w.session(), options(QStringLiteral("x")));
        run.step(0);
        QCOMPARE(int(run.hits().size()), 200);
        QVERIFY(!run.truncated());
    }

    void notesAreSearchedWhenTheFileHasThem()
    {
        const QByteArray comments = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>A</author></authors><commentList>"
                                    "<comment ref=\"B2\" authorId=\"0\"><text><r><t>Check the </t></r><r><t>invoice date</t></r></text></comment>"
                                    "<comment ref=\"C4\" authorId=\"0\"><text><t>fine</t></text></comment></commentList></comments>";
        ExtraPart part;
        part.name = QStringLiteral("xl/comments1.xml");
        part.data = comments;
        part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
        part.sheet = 0;
        part.relId = QStringLiteral("rIdC");
        part.relType = QStringLiteral("comments");
        part.relTarget = QStringLiteral("../comments1.xml");
        EditWindow e;
        QVERIFY(e.open(withParts(bookOfSheets({{"A1=x", "B2=invoice date", "C4=5"}}), {part})));
        FindOptions o = options(QStringLiteral("invoice"));
        o.lookIn = FindOptions::LookIn::Notes;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B2"}));
        o.pattern.text = QStringLiteral("fine");
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!C4"}));
        // a cell's own text is not a note
        o.pattern.text = QStringLiteral("x");
        QCOMPARE(names(findAll(e, o)), (QStringList{}));
        // after rows are inserted the notes are where their cells went
        o.pattern.text = QStringLiteral("invoice");
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B3"}));
        // a note on a cell that was deleted is gone, and comes back with an Undo
        QVERIFY(e.win.w.session()->deleteRows(0, 2, 1).ok);
        QCOMPARE(names(findAll(e, o)), (QStringList{}));
        pressUndo(&e.win.w);
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B3"}));
        // (the notes are read from the file once for the sheet, and kept as long as nothing moves them)
        const int reads = jp::office::sheet::noteReads();
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B3"}));
        QCOMPARE(jp::office::sheet::noteReads(), reads);
    }

    void findNextByColumnsStopsAtTheFirstHitInsteadOfLookingAtEverything()
    {
        QStringList cells;
        for (int r = 1; r <= 2000; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r == 3 || r == 6 ? "x" : "p") << QStringLiteral("B%1=%2").arg(r).arg(r == 1 ? "x" : "p");
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        FindOptions o = options(QStringLiteral("x"));
        o.order = FindOptions::Order::ByColumns;
        auto next = [&](quint32 row, quint32 col, qint64 *looked) {
            FoundCell from;
            from.sheet = 0;
            from.row = row;
            from.col = col;
            FindRun run(e.win.w.session(), o, FindRun::Want::Next, from);
            run.step(0);
            if (looked) *looked = run.cellsLooked();
            return names(run.hits());
        };
        qint64 looked = 0;
        QCOMPARE(next(3, 0, &looked), (QStringList{"1!A6"}));                    // (from A4: A5, A6)
        QCOMPARE(looked, qint64(2));
        QCOMPARE(next(5, 0, &looked), (QStringList{"1!B1"}));                    // (down the rest of column A, and B1)
        QCOMPARE(looked, qint64(1995));
        QCOMPARE(next(0, 1, &looked), (QStringList{"1!A3"}));                    // (round to the start)
        QCOMPARE(looked, qint64(1999 + 3));
    }

    void theNumberFormatOfAStyleIsMadeOnceForAllTheCellsThatHaveIt()
    {
        QStringList cells;
        for (int r = 1; r <= 3000; ++r) cells << QStringLiteral("A%1=%2~5").arg(r).arg(r);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        FindOptions o = options(QStringLiteral("2,9"));
        o.lookIn = FindOptions::LookIn::Values;
        const qint64 before = e.win.w.session()->formatsMade();
        FindRun run(e.win.w.session(), o);
        run.step(0);
        QVERIFY(run.hits().size() > 10);
        QVERIFY2(e.win.w.session()->formatsMade() - before <= 1, qPrintable(QString::number(e.win.w.session()->formatsMade() - before)));
    }

    void aLongTextCostsMoreOfASliceThanAShortOne()
    {
        QStringList longTexts, shortTexts;
        for (int r = 1; r <= 200; ++r) {
            longTexts << QStringLiteral("A%1=%2").arg(r).arg(QString(30000, QLatin1Char('a')));
            shortTexts << QStringLiteral("A%1=ab").arg(r);
        }
        FindOptions o = options(QStringLiteral("a*c"));
        auto slicesOf = [&](const QStringList &cells) {
            EditWindow e;
            if (!e.open(bookOfSheets({cells}))) return -1;
            FindRun run(e.win.w.session(), o);
            int slices = 0;
            while (!run.step(1000)) ++slices;
            return slices;
        };
        QCOMPARE(slicesOf(shortTexts), 0);                                      // (200 cells, one unit each)
        QVERIFY2(slicesOf(longTexts) >= 20, qPrintable(QString::number(slicesOf(longTexts))));
    }

    void notesFollowASortAndSayWhenTheyCannotBeFollowed()
    {
        const QByteArray comments = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>A</author></authors><commentList>"
                                    "<comment ref=\"B2\" authorId=\"0\"><text><t>invoice date</t></text></comment><comment ref=\"C4\" authorId=\"0\"><text><t>fine</t></text></comment></commentList></comments>";
        ExtraPart part;
        part.name = QStringLiteral("xl/comments1.xml");
        part.data = comments;
        part.contentType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml");
        part.sheet = 0;
        part.relId = QStringLiteral("rIdC");
        part.relType = QStringLiteral("comments");
        part.relTarget = QStringLiteral("../comments1.xml");
        EditWindow e;
        QVERIFY(e.open(withParts(bookOfSheets({{"A1=x", "B2=invoice date", "C4=5", "B3=a"}}), {part})));
        crunch::EditSession *s = e.win.w.session();
        FindOptions o = options(QStringLiteral("invoice"));
        o.lookIn = FindOptions::LookIn::Notes;
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B2"}));
        const qint64 reads = jp::office::sheet::noteReads();                      // (the file's notes are read once; the changes after this are followed, not read again)
        // a sort of rows 2 to 4 by column B puts the row of the note (B2, "invoice date"... after "a") second, and the note goes with it
        jp::crunch::SortSpec spec;
        jp::crunch::SortLevel level;
        level.key = 1;
        spec.levels.append(level);
        QVERIFY(s->sortRange(0, rangeOf(QStringLiteral("A2:C4")), spec).ok);
        QCOMPARE(s->displayText(0, 1, 1), QStringLiteral("a"));
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B3"}));
        pressUndo(&e.win.w);
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!B2"}));
        // too many changes to follow the notes through: the person is asked to save
        for (int i = 0; i < 70; ++i) QVERIFY(s->insertRows(0, 20, 1).ok);
        FindRun many(s, o);
        QVERIFY(many.done());
        QVERIFY2(many.problem().contains(QLatin1String("too many")), qPrintable(many.problem()));
        QCOMPARE(jp::office::sheet::noteReads(), reads);
    }

    void notesOfAWorkbookFromATextFileAreNotSearchedAndTheReasonIsGiven()
    {
        EditWindow e;
        QVERIFY(e.open("a,b\n1,2\n", QStringLiteral("data.csv")));
        FindOptions o = options(QStringLiteral("x"));
        o.lookIn = FindOptions::LookIn::Notes;
        FindRun run(e.win.w.session(), o);
        QVERIFY(run.done());
        QVERIFY2(run.problem().contains(QLatin1String(".xlsx")), qPrintable(run.problem()));
    }

    // ---- Replace
    void replaceAllIsOneStepThatUndoAndRedoGoThrough()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        FindOptions o = options(QStringLiteral("a"));
        o.scope = FindOptions::Scope::Workbook;
        std::vector<crunch::EditSession::CellSpot> spots;
        for (const FoundCell &h : findAll(e, o)) spots.push_back({h.sheet, h.row, h.col});
        QCOMPARE(int(spots.size()), 7);
        const int before = e.win.w.undoStack()->count();
        crunch::EditSession::ReplaceResult result;
        const crunch::EditSession::Outcome out = s->replaceInCells(spots, o.pattern, QStringLiteral("o"), &result);
        QVERIFY2(out.ok, qPrintable(out.problem));
        QCOMPARE(e.win.w.undoStack()->count(), before + 1);
        QCOMPARE(s->editText(0, 0, 0), QStringLiteral("opple"));
        QCOMPARE(s->editText(0, 1, 0), QStringLiteral("bonono"));
        QCOMPARE(s->editText(0, 0, 2), QStringLiteral("=o1&\"!\""));      // (a formula: A1 read as a name, so a formula that reads another cell, now the text o1)
        QCOMPARE(s->editText(1, 1, 1), QStringLiteral("opple tort"));
        QCOMPARE(result.cells, 7);
        QVERIFY(result.occurrences > result.cells);
        pressUndo(&e.win.w);
        QCOMPARE(s->editText(0, 0, 0), QStringLiteral("apple"));
        QCOMPARE(s->editText(0, 0, 2), QStringLiteral("=A1&\"!\""));
        QCOMPARE(s->editText(1, 1, 1), QStringLiteral("apple tart"));
        pressRedo(&e.win.w);
        QCOMPARE(s->editText(0, 1, 0), QStringLiteral("bonono"));
    }

    void replacingInAFormulaMakesAFormulaAgainAndAWholeCellMatchTakesTheCell()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        jp::office::sheet::TextPattern p;
        p.text = QStringLiteral("B2*2");
        QVERIFY(s->replaceInCells({{0, 1, 2}}, p, QStringLiteral("B2*3")).ok);
        QCOMPARE(s->editText(0, 1, 2), QStringLiteral("=B2*3"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:0.75"));
        // a replacement that makes a number: the cell holds a number
        p.text = QStringLiteral("7");
        p.wholeCell = true;
        QVERIFY(s->replaceInCells({{0, 2, 1}}, p, QStringLiteral("8")).ok);
        QCOMPARE(e.valueAt(QStringLiteral("B3")), QStringLiteral("n:8"));
        // a replacement that makes a formula that is none: refused, nothing changed, and the cell is named
        jp::office::sheet::TextPattern q;
        q.text = QStringLiteral("B2*3");
        const int steps = e.win.w.undoStack()->count();
        const crunch::EditSession::Outcome bad = s->replaceInCells({{0, 1, 2}}, q, QStringLiteral("B2*("));
        QVERIFY(!bad.ok);
        QVERIFY(bad.problem.contains(QLatin1String("C2")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(s->editText(0, 1, 2), QStringLiteral("=B2*3"));
    }

    void replaceRefusesWhatItMustNotChangeAndChangesNothing()
    {
        // a protected sheet with a locked cell and an unlocked one, an array, and a table header
        const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>cat</t></is></c><c r=\"B1\" s=\"1\" t=\"inlineStr\"><is><t>cat</t></is></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><f t=\"array\" ref=\"A2:A3\">{1;2}</f><v>1</v></c></row><row r=\"3\"><c r=\"A3\"><v>2</v></c></row>"
                                            "<row r=\"4\"><c r=\"A4\" t=\"inlineStr\"><is><t>cat</t></is></c></row>");
        EditWindow plain;
        QVERIFY(plain.open(buildXlsx({worksheet(rows, QString())}, QByteArray(), QString(), QString(), styles)));
        jp::office::sheet::TextPattern p;
        p.text = QStringLiteral("cat");
        crunch::EditSession *s = plain.win.w.session();
        // an array formula's text is a formula: replacing in it is refused, in words
        const crunch::EditSession::Outcome array = s->replaceInCells({{0, 1, 0}}, jp::office::sheet::TextPattern{QStringLiteral("1"), false, false, true}, QStringLiteral("5"));
        QVERIFY(!array.ok);
        QVERIFY(array.problem.contains(QLatin1String("A2")));
        EditWindow locked;
        QVERIFY(locked.open(buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\"/>"))}, QByteArray(), QString(), QString(), styles)));
        crunch::EditSession *l = locked.win.w.session();
        const int steps = locked.win.w.undoStack()->count();
        const crunch::EditSession::Outcome refused = l->replaceInCells({{0, 0, 1}, {0, 0, 0}, {0, 3, 0}}, p, QStringLiteral("dog"));
        QVERIFY(!refused.ok);
        QVERIFY(refused.problem.contains(QLatin1String("A1")));
        QCOMPARE(locked.win.w.undoStack()->count(), steps);
        QCOMPARE(l->editText(0, 0, 1), QStringLiteral("cat"));           // (the unlocked cell was not changed either: all or nothing)
        QCOMPARE(l->editText(0, 3, 0), QStringLiteral("cat"));
        // the unlocked cell alone
        QVERIFY(l->replaceInCells({{0, 0, 1}}, p, QStringLiteral("dog")).ok);
        QCOMPARE(l->editText(0, 0, 1), QStringLiteral("dog"));
    }

    void replaceIsWorkedOutASliceAtATimeAndAStalePlanIsNotApplied()
    {
        QStringList cells;
        for (int r = 1; r <= 500; ++r) cells << QStringLiteral("A%1=item %2").arg(r).arg(r);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        crunch::EditSession *s = e.win.w.session();
        std::vector<crunch::EditSession::CellSpot> spots;
        for (quint32 r = 0; r < 500; ++r) spots.push_back({0, r, 0});
        jp::office::sheet::TextPattern p;
        p.text = QStringLiteral("item");
        QString why;
        const auto plan = s->planReplace(spots, p, QStringLiteral("thing"), &why);
        QVERIFY2(plan, qPrintable(why));
        int slices = 0;
        while (!s->prepareReplace(*plan, 100)) QVERIFY(++slices < 20);
        QCOMPARE(slices, 4);                                                    // (500 cells, 100 at a time)
        QCOMPARE(e.win.w.undoStack()->count(), 0);                              // (nothing is changed until the plan is applied)
        crunch::EditSession::ReplaceResult result;
        QVERIFY(s->applyReplace(*plan, &result).ok);
        QCOMPARE(result.cells, 500);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(s->editText(0, 499, 0), QStringLiteral("thing 500"));
        // a plan made before another edit is not applied
        const auto stale = s->planReplace(spots, p, QStringLiteral("x"), &why);
        QVERIFY(stale);
        QVERIFY(s->type(0, 0, 1, QStringLiteral("1")).ok);
        s->prepareReplace(*stale, 0);
        const crunch::EditSession::Outcome o = s->applyReplace(*stale);
        QVERIFY(!o.ok);
        QVERIFY(o.problem.contains(QLatin1String("changed")));
        // a cell that cannot be changed is told, when the plan is applied
        const auto bad = s->planReplace({{0, 0, 0}}, jp::office::sheet::TextPattern{QStringLiteral("zzz"), false, false, true}, QStringLiteral("x"), &why);
        QVERIFY(bad);
        s->prepareReplace(*bad, 0);
        QVERIFY(s->applyReplace(*bad).ok);                                      // (nothing matches: nothing to change)
    }

    void whatASearchLooksInIsWhatACellHoldsAndATextThatLooksLikeAFormulaIsTheText()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1==2+3", "A2=hello", "A3=7", "B1==A3*2"}})));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        // a text that begins with a mark that would make it a formula when typed is found as it is
        QVERIFY(s->type(0, 3, 0, QStringLiteral("'=sum")).ok);
        QCOMPARE(s->searchText(0, 3, 0), QStringLiteral("=sum"));
        QCOMPARE(s->editText(0, 3, 0), QStringLiteral("'=sum"));                 // (but typed, it needs the apostrophe)
        QCOMPARE(s->searchText(0, 0, 0), QStringLiteral("=2+3"));
        QCOMPARE(s->searchText(0, 1, 0), QStringLiteral("hello"));
        QCOMPARE(s->searchText(0, 2, 0), QStringLiteral("7"));
        QCOMPARE(s->searchText(0, 0, 1), QStringLiteral("=A3*2"));
        QCOMPARE(s->searchText(0, 9, 9), QString());
        FindOptions o = options(QStringLiteral("=sum"));
        QCOMPARE(names(findAll(e, o)), (QStringList{"1!A4"}));
        // and a replace in it keeps it a text
        QVERIFY(s->replaceInCells({{0, 3, 0}}, o.pattern, QStringLiteral("=total")).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("s:=total"));
    }

    void replaceRefusesMoreCellsThanAnEditHolds()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        std::vector<crunch::EditSession::CellSpot> many(size_t(crunch::kMaxCellsPerEdit) + 1);
        jp::office::sheet::TextPattern p;
        p.text = QStringLiteral("a");
        const crunch::EditSession::Outcome out = e.win.w.session()->replaceInCells(many, p, QStringLiteral("b"));
        QVERIFY(!out.ok);
        QVERIFY(!out.problem.isEmpty());
    }

    // ---- Go To
    void goToFindsWhatIsTypedByReferenceOrName()
    {
        EditWindow e;
        QVERIFY(e.open(goBook()));
        const crunch::EditSession &s = *e.win.w.session();
        auto go = [&](const QString &text, int sheet = 0) { return crunch::resolveGoTo(s, text, sheet); };
        QVERIFY(go(QStringLiteral("B3")).ok);
        QCOMPARE(rangeName(go(QStringLiteral("$b$3")).range), QStringLiteral("B3"));
        QCOMPARE(rangeName(go(QStringLiteral("c5:a2")).range), QStringLiteral("A2:C5"));
        QCOMPARE(go(QStringLiteral("Sheet2!A1")).sheet, 1);
        QCOMPARE(go(QStringLiteral("'sheet2'!b2")).sheet, 1);
        QVERIFY(go(QStringLiteral("B:C")).wholeColumns);
        QCOMPARE(go(QStringLiteral("B:C")).range.col2, quint32(2));
        QVERIFY(go(QStringLiteral("3:5")).wholeRows);
        QCOMPARE(go(QStringLiteral("3:5")).range.row2, quint32(4));
        QCOMPARE(rangeName(go(QStringLiteral("sales")).range), QStringLiteral("A1:B2"));      // (a name, in any case)
        QCOMPARE(go(QStringLiteral("Local"), 1).sheet, 1);                                       // (a name of the sheet in view)
        QVERIFY(!go(QStringLiteral("Local"), 0).ok);                                             // (and not of another's)
        const crunch::GoToTarget notRange = go(QStringLiteral("Total"));
        QVERIFY(!notRange.ok);
        QVERIFY(notRange.problem.contains(QLatin1String("Total")));
        QVERIFY(!go(QStringLiteral("Sheet9!A1")).ok);
        QVERIFY(!go(QStringLiteral("A1048577")).ok);
        QVERIFY(!go(QString()).ok);
        QCOMPARE(crunch::goToNames(s, 0).size(), 1);                                           // (Sales: Total is no range, Local is of the other sheet)
        QCOMPARE(crunch::goToNames(s, 1).size(), 2);
    }

    void goToSpecialFindsCellsByWhatTheyHold()
    {
        using K = crunch::SpecialKind;
        EditWindow e;
        QVERIFY(e.open(goBook()));
        QCOMPARE(rangesOf(special(e, K::Constants, QString(), QStringLiteral("A1"), true, false, false, false)), QStringLiteral("A1:A2 A4:A5"));
        QCOMPARE(rangesOf(special(e, K::Constants, QString(), QStringLiteral("A1"), false, true, false, false)), QStringLiteral("B1:B2 B5"));
        QCOMPARE(rangesOf(special(e, K::Formulas)), QStringLiteral("C1:D1 C2"));
        QCOMPARE(rangesOf(special(e, K::Formulas, QString(), QStringLiteral("A1"), true, false, false, false)), QStringLiteral("C1"));
        QCOMPARE(rangesOf(special(e, K::Formulas, QString(), QStringLiteral("A1"), false, false, false, true)), QStringLiteral("D1"));
        QCOMPARE(rangesOf(special(e, K::Errors)), QStringLiteral("D1"));
        QCOMPARE(rangesOf(special(e, K::LastCell)), QStringLiteral("D5"));
        QCOMPARE(special(e, K::Constants, QString(), QStringLiteral("A1"), true, false, false, false).cells, qint64(4));
        // within a selection
        QCOMPARE(rangesOf(special(e, K::Constants, QStringLiteral("A1:B2"))), QStringLiteral("A1:B2"));
        QCOMPARE(rangesOf(special(e, K::Formulas, QStringLiteral("A1:C1"))), QStringLiteral("C1"));
        // a kind that finds nothing says so
        const crunch::SpecialResult none = special(e, K::Errors, QStringLiteral("A1:C5"));
        QVERIFY(none.ranges.isEmpty());
        QVERIFY(!none.problem.isEmpty());
    }

    void goToSpecialFindsBlanksAndTheCurrentRegion()
    {
        using K = crunch::SpecialKind;
        EditWindow e;
        QVERIFY(e.open(goBook()));
        // (the blanks of a block, in runs along the rows; a run under one of the same columns joins it)
        QCOMPARE(rangesOf(special(e, K::Blanks, QStringLiteral("A1:D5"))), QStringLiteral("D2 A3:D3 B4:D4 C5:D5"));
        QCOMPARE(special(e, K::Blanks, QStringLiteral("A1:D5")).cells, qint64(1 + 4 + 3 + 2));
        // the blanks of the cells the sheet uses
        QCOMPARE(special(e, K::Blanks).cells, qint64(1 + 4 + 3 + 2));
        QCOMPARE(rangesOf(special(e, K::CurrentRegion, QString(), QStringLiteral("B1"))), QStringLiteral("A1:D2"));
        QCOMPARE(rangesOf(special(e, K::CurrentRegion, QString(), QStringLiteral("B5"))), QStringLiteral("A4:B5"));
        QCOMPARE(rangesOf(special(e, K::CurrentRegion, QString(), QStringLiteral("A3"))), QStringLiteral("A1:D5"));      // (a blank row between: its neighbors above and below are in)
        // a cell with nothing around it is a region of itself
        QCOMPARE(rangesOf(special(e, K::CurrentRegion, QString(), QStringLiteral("H9"))), QStringLiteral("H9"));
    }

    void theCurrentRegionOfATallBlockOrADiagonalIsFoundWholeAndNotSlowly()
    {
        QStringList cells = {"A1=Head", "B1=Other"};
        for (int r = 2; r <= 8000; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r) << QStringLiteral("B%1=x").arg(r);
        for (int i = 0; i < 300; ++i) cells << QStringLiteral("%1%2=1").arg(QChar(QLatin1Char(char('H' + (i % 10))))).arg(10010 + i);   // (a column of a separate block)
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        bool truncated = true;
        const CellRange r = crunch::currentRegion(*e.win.w.session(), 0, 4000, 0, &truncated);
        QCOMPARE(rangeName(r), QStringLiteral("A1:B8000"));
        QVERIFY(!truncated);
        // cells that touch only at a corner are one block
        EditWindow d;
        QStringList diagonal;
        for (int i = 0; i < 40; ++i) diagonal << QStringLiteral("%1%2=1").arg(columnName(quint32(i))).arg(i + 1);
        QVERIFY(d.open(bookOfSheets({diagonal})));
        QCOMPARE(rangeName(crunch::currentRegion(*d.win.w.session(), 0, 0, 0)), QStringLiteral("A1:AN40"));
    }

    void goToSpecialFindsTheCellsThatShow()
    {
        using K = crunch::SpecialKind;
        EditWindow e;
        QVERIFY(e.open(goBook({{2, QStringLiteral("hidden=\"1\"")}})));
        QCOMPARE(rangesOf(special(e, K::VisibleCells)), QStringLiteral("A1:D1 A3:D5"));
        QCOMPARE(rangesOf(special(e, K::VisibleCells, QStringLiteral("A1:B2"))), QStringLiteral("A1:B1"));
    }

    void goToSpecialFindsConditionalFormatsAndRulesOfValidation()
    {
        using K = crunch::SpecialKind;
        EditWindow e;
        const QString after = QStringLiteral("<conditionalFormatting sqref=\"A1:A2 C5\"><cfRule type=\"cellIs\" priority=\"1\" operator=\"greaterThan\"><formula>1</formula></cfRule></conditionalFormatting>"
                                             "<dataValidations count=\"2\"><dataValidation type=\"whole\" operator=\"between\" sqref=\"B1\"><formula1>1</formula1><formula2>9</formula2></dataValidation>"
                                             "<dataValidation type=\"whole\" operator=\"between\" sqref=\"B5\"><formula1>1</formula1><formula2>9</formula2></dataValidation></dataValidations>");
        QVERIFY(e.open(goBook(QMap<int, QString>(), after)));
        QCOMPARE(rangesOf(special(e, K::ConditionalFormats)), QStringLiteral("A1:A2 C5"));
        QCOMPARE(rangesOf(special(e, K::ConditionalFormats, QStringLiteral("A2:B5"))), QStringLiteral("A2"));
        QCOMPARE(rangesOf(special(e, K::DataValidation)), QStringLiteral("B1 B5"));
        jp::crunch::SpecialOptions same;
        same.kind = K::DataValidation;
        same.sameRule = true;
        QCOMPARE(rangesOf(jp::crunch::goToSpecial(*e.win.w.session(), 0, CellRange(), 4, 1, same)), QStringLiteral("B1 B5"));
        const crunch::SpecialResult none = jp::crunch::goToSpecial(*e.win.w.session(), 0, CellRange(), 0, 0, same);
        QVERIFY(none.ranges.isEmpty() && !none.problem.isEmpty());
    }

    // ---- the dialog and the window
    void findNextAndFindAllWorkFromTheDialog()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.act(QStringLiteral("find.find"))->isEnabled());
        QCOMPARE(w.act(QStringLiteral("find.find"))->shortcut(), QKeySequence(QKeySequence::Find));
        w.act(QStringLiteral("find.find"))->trigger();
        FindDialog *d = w.findDialog();
        QVERIFY(d && d->isVisible());
        QCOMPARE(d->tab(), FindDialog::Tab::Find);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("apple"));
        auto click = [&](const char *name) { d->findChild<QPushButton *>(QString::fromLatin1(name))->click(); };
        e.select(QStringLiteral("A1"));
        click("findNext");
        QTRY_COMPARE(e.selected(), QStringLiteral("B1"));
        click("findNext");                                                       // (nothing more on this sheet: round to the first)
        QTRY_COMPARE(e.selected(), QStringLiteral("A1"));
        QVERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("Sheet1!A1")));
        // List All lists the cells, and a click on one goes to it
        click("findAll");
        QTreeWidget *list = d->results();
        QTRY_COMPARE(list->topLevelItemCount(), 2);
        QVERIFY(list->isVisible());
        d->setTab(FindDialog::Tab::Replace);
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("pear"));
        saveShot(d, QStringLiteral("find-and-replace"));
        d->setTab(FindDialog::Tab::Find);
        QCOMPARE(list->topLevelItem(1)->text(1), QStringLiteral("B1"));
        list->setCurrentItem(list->topLevelItem(1));
        QCOMPARE(e.selected(), QStringLiteral("B1"));
        // the whole workbook: the other sheet's cells too, and a click on one shows that sheet
        d->findChild<QComboBox *>(QStringLiteral("findWithin"))->setCurrentIndex(1);
        click("findAll");
        QTRY_COMPARE(list->topLevelItemCount(), 4);
        list->setCurrentItem(list->topLevelItem(3));
        QTRY_COMPARE(w.activeSheet(), 1);
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        // nothing found says so
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("zzz"));
        click("findAll");
        QTRY_VERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("No cell matches")));
        QVERIFY(!list->isVisible());
    }

    void withManyCellsSelectedTheSearchIsInThem()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("find.find"))->trigger();
        FindDialog *d = w.findDialog();
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("a"));
        e.view()->selectRange(1, 0, 1, 0 + 1);   // A2:B2
        d->findChild<QPushButton *>(QStringLiteral("findAll"))->click();
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);                      // (banana; B2 holds 25%)
        QCOMPARE(d->results()->topLevelItem(0)->text(1), QStringLiteral("A2"));
    }

    void replaceAllFromTheDialogIsOneStepAndSaysWhatItDid()
    {
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        QCOMPARE(d->tab(), FindDialog::Tab::Replace);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("apple"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("pear"));
        const int steps = w.undoStack()->count();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_COMPARE(w.undoStack()->count(), steps + 1);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.session()->editText(0, 0, 0), QStringLiteral("pear"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 1), QStringLiteral("pear pie"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 2), QStringLiteral("=A1&\"!\""));           // (A1 is a reference, not the word)
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("s:pear!"));                // (and the formula was worked out again)
        QVERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("Changed")));
        w.act(QStringLiteral("edit.undo"))->trigger();
        QCOMPARE(e.win.w.session()->editText(0, 0, 0), QStringLiteral("apple"));
        // Replace takes the cell with the cursor if it has the text, and goes on to the next
        e.select(QStringLiteral("A1"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("fig"));
        d->findChild<QPushButton *>(QStringLiteral("replaceOne"))->click();
        QTRY_COMPARE(e.selected(), QStringLiteral("B1"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 0), QStringLiteral("fig"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 1), QStringLiteral("Apple pie"));
        // Replace on the cell that has it changes it, and with nothing more to find says so
        d->findChild<QPushButton *>(QStringLiteral("replaceOne"))->click();
        QTRY_COMPARE(e.win.w.session()->editText(0, 0, 1), QStringLiteral("fig pie"));
        QTRY_VERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("Nothing matches")));
        // Replace on a cell that does not have it only finds (the replace text is not put anywhere)
        e.select(QStringLiteral("A2"));
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("cherry"));
        d->findChild<QPushButton *>(QStringLiteral("replaceOne"))->click();
        QTRY_COMPARE(e.selected(), QStringLiteral("A3"));
        QCOMPARE(e.win.w.session()->editText(0, 2, 0), QStringLiteral("cherry"));
    }

    void replaceAllToldWhyWhenItCannot()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>cat</t></is></c></row>"), QStringLiteral("<sheetProtection sheet=\"1\"/>"))})));
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("cat"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("dog"));
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY(e.win.told.last().contains(QLatin1String("locked")));
        QCOMPARE(e.win.w.session()->editText(0, 0, 0), QStringLiteral("cat"));
    }

    void aSearchIsGivenUpWhenTheWorkbookIsChanged()
    {
        QStringList cells;
        for (int r = 1; r <= 3000; ++r) cells << QStringLiteral("A%1=x").arg(r) << QStringLiteral("B%1=y").arg(r) << QStringLiteral("C%1=%2").arg(r).arg(r);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        CrunchWindow &w = e.win.w;
        w.setFixedSlices(1000);                                                  // (every slice the same size: how many turns it takes is counted, not timed)
        w.act(QStringLiteral("find.find"))->trigger();
        FindDialog *d = w.findDialog();
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("x"));
        d->findChild<QPushButton *>(QStringLiteral("findAll"))->click();
        QVERIFY(w.isSearching());                                                // (9,000 cells at 1,000 a turn are nine slices)
        QVERIFY(e.win.w.session()->type(0, 0, 3, QStringLiteral("5")).ok);
        QTRY_VERIFY(!w.isSearching());
        QVERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("stopped")));
        // and a new search is made in full
        d->findChild<QPushButton *>(QStringLiteral("findAll"))->click();
        QTRY_COMPARE(d->results()->topLevelItemCount(), 3000);
    }

    void goToAndTheNameBoxFindNamesAndOtherSheets()
    {
        EditWindow e;
        QVERIFY(e.open(goBook()));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.goToReference(QStringLiteral("B2:C3")));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("B2:C3"));
        QVERIFY(w.goToReference(QStringLiteral("sales")));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:B2"));
        QVERIFY(w.goToReference(QStringLiteral("Sheet2!A1")));
        QCOMPARE(w.activeSheet(), 1);
        QVERIFY(w.goToReference(QStringLiteral("Sheet1!C4")));
        QCOMPARE(w.activeSheet(), 0);
        QCOMPARE(e.selected(), QStringLiteral("C4"));
        // the Name box does the same
        w.nameBox()->setText(QStringLiteral("Sales"));
        QTest::keyClick(w.nameBox(), Qt::Key_Return);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:B2"));
        // what names nothing is said in words
        e.win.told.clear();
        QVERIFY(!w.goToReference(QStringLiteral("nothing here")));
        QVERIFY(!e.win.told.isEmpty());
        // Go To asks for a reference
        w.setGoToAsker([](QString *text) {
            *text = QStringLiteral("D5");
            return true;
        });
        w.act(QStringLiteral("find.goto"))->trigger();
        QCOMPARE(e.selected(), QStringLiteral("D5"));
        QVERIFY(w.act(QStringLiteral("find.goto"))->shortcuts().contains(QKeySequence(Qt::Key_F5)));
    }

    void goToSpecialSelectsTheFirstRangeAndTintsTheRest()
    {
        EditWindow e;
        QVERIFY(e.open(goBook()));
        CrunchWindow &w = e.win.w;
        w.setSpecialAsker([](crunch::SpecialOptions *o) {
            o->kind = crunch::SpecialKind::Constants;
            o->text = false;
            o->logicals = false;
            o->errors = false;
            return true;
        });
        w.act(QStringLiteral("find.gotoSpecial"))->trigger();
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:A2"));
        QCOMPARE(int(e.view()->marks().size()), 1);                               // (A4:A5)
        QVERIFY(w.statusText().contains(QLatin1String("tinted")));
        e.select(QStringLiteral("C3"));                                           // the next click takes the tint away
        QVERIFY(e.view()->marks().empty());
        // nothing found: the person is told, and nothing is tinted
        w.setSpecialAsker([](crunch::SpecialOptions *o) {
            o->kind = crunch::SpecialKind::ConditionalFormats;
            return true;
        });
        e.win.told.clear();
        w.act(QStringLiteral("find.gotoSpecial"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
    }

    void theFindMenuIsOnTheRibbonAndOffWithoutAWorkbook()
    {
        Window blank;
        for (const char *id : {"edit.find", "find.find", "find.replace", "find.next", "find.goto", "find.gotoSpecial"}) {
            QVERIFY2(blank.w.act(QString::fromLatin1(id)), id);
            QVERIFY2(!blank.w.act(QString::fromLatin1(id))->isEnabled(), id);
        }
        EditWindow e;
        QVERIFY(e.open(fruitBook()));
        for (const char *id : {"edit.find", "find.find", "find.replace", "find.next", "find.goto", "find.gotoSpecial"}) QVERIFY2(e.win.w.act(QString::fromLatin1(id))->isEnabled(), id);
        QVERIFY(e.win.w.ribbonErrors().isEmpty());
    }
private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchFindTests)
#include "tst_crunch_find.moc"
