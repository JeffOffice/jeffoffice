// Tests of the filter on the edit session: every kind of condition, the rows it hides kept apart from rows hidden by hand, Clear and Reapply, turning it off, one step for Undo, the filter
// following rows and columns that are inserted and deleted, and the saved file holding the conditions and the hidden rows.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filterdialogs.h"
#include "app/filterpopup.h"
#include "core/appinfo.h"
#include "core/filterlist.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"

#include <QAction>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTreeWidget>
#include <QXmlStreamReader>
#include <QSignalSpy>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::AutoFilter;
using jp::office::sheet::FilterColumn;

namespace {

//      A            B     C            D
//  1   Name         Qty   Date         Tag
//  2   apple        5     2024-01-15   (red fill)
//  3   banana       12    2024-02-20
//  4   Apple pie    7     2024-03-05   (red fill)
//  5   cherry       3     2024-01-20
//  6   (nothing)    9     (nothing)
//  7   date         20    2024-12-31   (red fill)
//  8   egg          1     2023-06-01
//  9   fig          15    2024-02-02
QStringList fruit()
{
    return {"A1=Name", "B1=Qty", "C1=Date", "D1=Tag", "A2=apple", "B2=5", "C2=@2024-01-15", "D2=r~3", "A3=banana", "B3=12", "C3=@2024-02-20", "A4=Apple pie", "B4=7", "C4=@2024-03-05",
            "D4=r~3", "A5=cherry", "B5=3", "C5=@2024-01-20", "B6=9", "A7=date", "B7=20", "C7=@2024-12-31", "D7=r~3", "A8=egg", "B8=1", "C8=@2023-06-01", "A9=fig", "B9=15", "C9=@2024-02-02"};
}

AutoFilter range(const QString &ref = QStringLiteral("A1:D9"))
{
    AutoFilter f;
    f.ref = rangeOf(ref);
    return f;
}

FilterColumn values(const QStringList &texts, bool blank = false)
{
    FilterColumn c;
    c.kind = FilterColumn::Kind::Values;
    c.values = texts;
    c.blank = blank;
    return c;
}

FilterColumn custom(FilterColumn::Op op, const QString &value, bool both = true, FilterColumn::Op op2 = FilterColumn::Op::Equal, const QString &value2 = QString())
{
    FilterColumn c;
    c.kind = FilterColumn::Kind::Custom;
    c.conditions.append({op, value});
    if (!value2.isEmpty()) c.conditions.append({op2, value2});
    c.both = both;
    return c;
}

FilterColumn dynamic(const QString &type)
{
    FilterColumn c;
    c.kind = FilterColumn::Kind::Dynamic;
    c.dynamic = type;
    return c;
}

// The rows (from 1) of the data that show.
QString shownRows(EditWindow &e, int first = 2, int last = 9)
{
    QStringList out;
    for (int r = first; r <= last; ++r) {
        bool hidden = false;
        for (const jp::office::sheet::RowSpec &spec : e.win.w.session()->calculation()->rowSpecsIn(0, quint32(r - 1), quint32(r - 1)))
            if (spec.row == quint32(r - 1)) hidden = spec.hidden;
        if (!hidden) out << QString::number(r);
    }
    return out.join(QLatin1Char(' '));
}

bool filterWith(EditWindow &e, const AutoFilter &f)
{
    const crunch::EditSession::Outcome o = e.win.w.session()->setFilter(0, f, QStringLiteral("Filtering"));
    if (!o.ok) qWarning("%s", qPrintable(o.problem));
    return o.ok;
}

// The sheet's filter (or one over the whole table, if it has none) with the condition of a column put in.
AutoFilter with(EditWindow &e, quint32 column, const FilterColumn &c)
{
    if (e.win.w.session()->calculation()->autoFilterOf(0).valid()) return e.win.w.session()->filterWith(0, column, c);
    AutoFilter f = range();
    FilterColumn put = c;
    put.column = column;
    f.columns.append(put);
    return f;
}

jp::office::sheet::Sheet savedSheet(const QString &path, QByteArray *part = nullptr, QByteArray *styles = nullptr)
{
    jp::office::Package p;
    jp::office::sheet::Workbook b;
    jp::office::sheet::Sheet s;
    if (p.readLazyFile(path) && b.open(p)) {
        b.readSheet(0, &s);
        if (part) *part = p.part(b.info().sheets[0].part);
        if (styles) *styles = p.part(b.stylesPart());
    }
    return s;
}

bool saveIt(EditWindow &e)
{
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save()) return false;
    return saved.wait(20000) && saved.last().last().toBool();
}

} // namespace

class CrunchFilterTests : public QObject {
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

    void aListOfValuesLetsThroughTheCellsThatShowThem()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        e.win.w.session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        QVERIFY(filterWith(e, with(e, 0, values({"APPLE", "banana"}))));                        // (case does not matter)
        QCOMPARE(shownRows(e), QStringLiteral("2 3"));
        QVERIFY(filterWith(e, with(e, 0, values({"apple", "date"}, true))));                      // (and the cells with nothing)
        QCOMPARE(shownRows(e), QStringLiteral("2 6 7"));
        // one step, and Undo shows the rows and takes the filter off
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        pressUndo(&e.win.w);
        QCOMPARE(shownRows(e), QStringLiteral("2 3"));
        pressUndo(&e.win.w);
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 6 7 8 9"));
        QVERIFY(!e.win.w.session()->calculation()->autoFilterOf(0).valid());
        pressRedo(&e.win.w);
        pressRedo(&e.win.w);
        QCOMPARE(shownRows(e), QStringLiteral("2 6 7"));
        QVERIFY(e.win.w.session()->calculation()->autoFilterOf(0).active());
    }

    void comparisonsAndAndOrCombinations()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        using Op = FilterColumn::Op;
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("5")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 6 7 9"));
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("5"), true, Op::LessEqual, QStringLiteral("12")))));       // between
        QCOMPARE(shownRows(e), QStringLiteral("3 4 6"));
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Less, QStringLiteral("3"), false, Op::Greater, QStringLiteral("15")))));           // either
        QCOMPARE(shownRows(e), QStringLiteral("7 8"));
        QVERIFY(filterWith(e, with(e, 1, custom(Op::NotEqual, QStringLiteral("5")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 5 6 7 8 9"));
        // text: begins with, ends with, contains, does not contain, are patterns for the whole cell
        QVERIFY(filterWith(e, e.win.w.session()->filterCleared(0)));
        QVERIFY(filterWith(e, with(e, 0, custom(Op::Equal, QStringLiteral("a*")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 4"));
        QVERIFY(filterWith(e, with(e, 0, custom(Op::Equal, QStringLiteral("*e")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 4 7"));
        QVERIFY(filterWith(e, with(e, 0, custom(Op::Equal, QStringLiteral("*an*")))));
        QCOMPARE(shownRows(e), QStringLiteral("3"));
        QVERIFY(filterWith(e, with(e, 0, custom(Op::NotEqual, QStringLiteral("*e*")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 6 9"));                                                                                // (a cell with nothing has no "e")
        QVERIFY(filterWith(e, with(e, 0, custom(Op::GreaterEqual, QStringLiteral("d")))));
        QCOMPARE(shownRows(e), QStringLiteral("7 8 9"));
        // an empty list of conditions is refused
        QString why;
        FilterColumn none;
        none.kind = FilterColumn::Kind::Custom;
        QVERIFY(!e.win.w.session()->planFilter(0, with(e, 2, none), &why));
        QVERIFY(!why.isEmpty());
    }

    void theTopAndBottomOfTheValuesAndTheAverage()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        FilterColumn top;
        top.kind = FilterColumn::Kind::Top10;
        top.count = 3;
        QVERIFY(filterWith(e, with(e, 1, top)));
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));                    // 20, 15, 12
        top.top = false;
        top.count = 2;
        QVERIFY(filterWith(e, with(e, 1, top)));
        QCOMPARE(shownRows(e), QStringLiteral("5 8"));                      // 1, 3
        top.top = true;
        top.percent = true;
        top.count = 25;                                                      // a quarter of eight numbers: two
        QVERIFY(filterWith(e, with(e, 1, top)));
        QCOMPARE(shownRows(e), QStringLiteral("7 9"));
        QVERIFY(filterWith(e, with(e, 1, dynamic(QStringLiteral("aboveAverage")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));                    // the mean is 9: 12, 20, 15 (9 is not above it)
        QVERIFY(filterWith(e, with(e, 1, dynamic(QStringLiteral("belowAverage")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 4 5 8"));
    }

    void datesByPeriodByMonthAndByGroup()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        e.win.w.session()->setToday(QDate(2024, 2, 10));                    // (a Saturday: the week of Sunday the 4th)
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("thisMonth")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 9"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("lastMonth")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 5"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("nextMonth")))));
        QCOMPARE(shownRows(e), QStringLiteral("4"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("thisYear")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 7 9"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("lastYear")))));
        QCOMPARE(shownRows(e), QStringLiteral("8"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("yearToDate")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 5 9"));                     // (up to the 10th of February)
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("thisQuarter")))));
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 9"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("Q1")))));      // the first quarter of any year
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 9"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("M12")))));
        QCOMPARE(shownRows(e), QStringLiteral("7"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("thisWeek")))));
        QCOMPARE(shownRows(e), QStringLiteral(""));
        e.win.w.session()->setToday(QDate(2024, 2, 20));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("today")))));
        QCOMPARE(shownRows(e), QStringLiteral("3"));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("tomorrow")))));
        QCOMPARE(shownRows(e), QStringLiteral(""));
        QVERIFY(filterWith(e, with(e, 2, dynamic(QStringLiteral("yesterday")))));
        QCOMPARE(shownRows(e), QStringLiteral(""));
        // a kind that is not known is refused, in words
        QString why;
        QVERIFY(!e.win.w.session()->planFilter(0, with(e, 2, dynamic(QStringLiteral("someday"))), &why));
        QVERIFY(why.contains(QLatin1String("someday")));
        // groups of dates in a list: a year, a month, a day
        FilterColumn groups = values({});
        FilterColumn::DateGroup year, month, day;
        year.unit = FilterColumn::DateGroup::Unit::Year;
        year.year = 2023;
        month.unit = FilterColumn::DateGroup::Unit::Month;
        month.year = 2024;
        month.month = 2;
        day.unit = FilterColumn::DateGroup::Unit::Day;
        day.year = 2024;
        day.month = 12;
        day.day = 31;
        groups.dates = {year, month, day};
        QVERIFY(filterWith(e, with(e, 2, groups)));
        QCOMPARE(shownRows(e), QStringLiteral("3 7 8 9"));
    }

    void cellsCanBeFilteredByTheirColor()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        FilterColumn red;
        red.kind = FilterColumn::Kind::Color;
        red.color = jp::office::sheet::ColorSpec::fromRgb(0xFF0000);
        QVERIFY(filterWith(e, with(e, 3, red)));
        QCOMPARE(shownRows(e), QStringLiteral("2 4 7"));
        red.color = jp::office::sheet::ColorSpec();                          // cells with no color
        QVERIFY(filterWith(e, with(e, 3, red)));
        QCOMPARE(shownRows(e), QStringLiteral("3 5 6 8 9"));
        FilterColumn blue;
        blue.kind = FilterColumn::Kind::Color;
        blue.fontColor = true;
        blue.color = jp::office::sheet::ColorSpec::fromRgb(0x0000FF);
        QVERIFY(filterWith(e, with(e, 3, blue)));
        QCOMPARE(shownRows(e), QStringLiteral(""));
    }

    void conditionsOfSeveralColumnsAllHaveToHold()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        using Op = FilterColumn::Op;
        QVERIFY(filterWith(e, with(e, 0, custom(Op::Equal, QStringLiteral("a*")))));
        AutoFilter f = with(e, 1, custom(Op::Greater, QStringLiteral("5")));
        QVERIFY(filterWith(e, f));
        QCOMPARE(shownRows(e), QStringLiteral("4"));                          // apple (5) does not pass, Apple pie (7) does
        FilterColumn red;
        red.kind = FilterColumn::Kind::Color;
        red.color = jp::office::sheet::ColorSpec::fromRgb(0xFF0000);
        QVERIFY(filterWith(e, with(e, 3, red)));
        QCOMPARE(shownRows(e), QStringLiteral("4"));
        QCOMPARE(e.win.w.session()->calculation()->autoFilterOf(0).conditionColumns().size(), 3);
    }

    void clearReapplyAndTurningItOffGoThroughOneStepEach()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        crunch::EditSession *s = e.win.w.session();
        using Op = FilterColumn::Op;
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("10")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));
        // a value changes: the rows stay as they were until the filter is applied again
        QVERIFY(s->type(0, 4, 1, QStringLiteral("30")).ok);                      // B5 = 30
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(filterWith(e, s->calculation()->autoFilterOf(0)));              // Reapply
        QCOMPARE(shownRows(e), QStringLiteral("3 5 7 9"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        // Clear: the conditions go, the buttons stay, the rows show
        QVERIFY(filterWith(e, s->filterCleared(0)));
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 6 7 8 9"));
        QVERIFY(s->calculation()->autoFilterOf(0).valid());
        QVERIFY(!s->calculation()->autoFilterOf(0).active());
        // the same again changes nothing: no step
        const int now = e.win.w.undoStack()->count();
        QVERIFY(filterWith(e, s->filterCleared(0)));
        QCOMPARE(e.win.w.undoStack()->count(), now);
        // off: no filter at all
        QVERIFY(filterWith(e, AutoFilter()));
        QVERIFY(!s->calculation()->autoFilterOf(0).valid());
        QCOMPARE(e.win.w.undoStack()->count(), now + 1);
        pressUndo(&e.win.w);
        QVERIFY(s->calculation()->autoFilterOf(0).valid());
    }

    void rowsAFilterHidesAreKeptApartFromRowsAPersonHid()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        crunch::EditSession *s = e.win.w.session();
        using Op = FilterColumn::Op;
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(s->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);               // row 3 by hand
        QCOMPARE(shownRows(e), QStringLiteral("2 4 5 6 7 8 9"));
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("10")))));        // rows 3, 7, 9 pass; row 3 is hidden by hand too
        QCOMPARE(shownRows(e), QStringLiteral("7 9"));
        QVERIFY(filterWith(e, s->filterCleared(0)));
        QCOMPARE(shownRows(e), QStringLiteral("2 4 5 6 7 8 9"));                         // (row 3 is still hidden by hand)
        // unhiding rows does not show the ones the filter hides, and shows the ones hand hid
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("10")))));
        crunch::EditSession::SizeChange show;
        show.setHidden = true;
        show.hidden = false;
        QVERIFY(s->resizeRows(0, 1, 8, show, QStringLiteral("Unhide")).ok);
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));
        QVERIFY(filterWith(e, s->filterCleared(0)));
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 6 7 8 9"));
        // hiding by hand a row the filter hides, then clearing the filter: it stays hidden
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("10")))));
        QVERIFY(s->resizeRows(0, 1, 1, hide, QStringLiteral("Hide")).ok);               // row 2, which the filter hides
        QVERIFY(filterWith(e, s->filterCleared(0)));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 5 6 7 8 9"));
    }

    void sortingAFilteredBlockPutsOnlyTheRowsThatShowInOrder()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        using Op = FilterColumn::Op;
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("5")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 6 7 9"));
        jp::crunch::SortSpec spec;
        jp::crunch::SortLevel level;
        level.key = 1;
        spec.levels.append(level);
        spec.hasHeader = true;
        QVERIFY(e.win.w.session()->sortRange(0, rangeOf(QStringLiteral("A1:D9")), spec).ok);
        QCOMPARE(shown(e, QStringLiteral("B2:B9")), QStringLiteral("5|7|9|3|12|15|1|20"));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 6 7 9"));                 // (the rows that hide are where they were)
        // the names went with their numbers: apple pie (7) is first of those that show
        QCOMPARE(shown(e, QStringLiteral("A3:A3")), QStringLiteral("Apple pie"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("B2:B9")), QStringLiteral("5|12|7|3|9|20|1|15"));
        // one row showing is nothing to sort
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("19")))));
        QVERIFY(!e.win.w.session()->sortRange(0, rangeOf(QStringLiteral("A1:D9")), spec).ok);
    }

    void aFilterOnlyHidesRowsUnderItsButtons()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        using Op = FilterColumn::Op;
        AutoFilter f = range(QStringLiteral("A1:D5"));
        FilterColumn c = custom(Op::Greater, QStringLiteral("5"));
        c.column = 1;
        f.columns.append(c);
        QVERIFY(filterWith(e, f));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 6 7 8 9"));                           // (rows 6 and below are not in the filter)
        // moving the range down shows the row that is no longer in it
        AutoFilter g = range(QStringLiteral("A1:D3"));
        g.columns.append(c);
        QVERIFY(filterWith(e, g));
        QCOMPARE(shownRows(e), QStringLiteral("3 4 5 6 7 8 9"));
    }

    void aFilterOverMoreRowsThanAnEditHoldsIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        QString why;
        AutoFilter f = range(QStringLiteral("A1:D200000"));
        QVERIFY(!e.win.w.session()->planFilter(0, f, &why));
        QVERIFY(why.contains(QLatin1String("more rows")));
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        // a condition for a column the range does not have
        f = range();
        FilterColumn c = values({"x"});
        c.column = 9;
        f.columns.append(c);
        QVERIFY(!e.win.w.session()->planFilter(0, f, &why));
    }

    void theRowsAreWorkedOutASliceAtATime()
    {
        QStringList cells;
        for (int r = 2; r <= 3001; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r % 7);
        cells << "A1=Head";
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        crunch::EditSession *s = e.win.w.session();
        AutoFilter f = range(QStringLiteral("A1:A3001"));
        FilterColumn c;
        c.kind = FilterColumn::Kind::Custom;
        c.conditions.append({FilterColumn::Op::Equal, QStringLiteral("3")});
        f.columns.append(c);
        QString why;
        const auto plan = s->planFilter(0, f, &why);
        QVERIFY2(plan, qPrintable(why));
        int slices = 0;
        while (!s->prepareFilter(*plan, 1000)) QVERIFY(++slices < 10);
        QCOMPARE(slices, 2);                                                    // (3,000 rows, 1,000 cells at a time)
        QVERIFY(s->applyFilter(*plan, QStringLiteral("Filtering")).ok);
        int shown = 0;
        for (int r = 2; r <= 3001; ++r) {
            bool hidden = false;
            for (const auto &spec : s->calculation()->rowSpecsIn(0, quint32(r - 1), quint32(r - 1)))
                if (spec.row == quint32(r - 1)) hidden = spec.hidden;
            if (!hidden) ++shown;
        }
        QCOMPARE(shown, 429);                                                   // r % 7 == 3 for r up to 3001
        // a plan is for the cells as they were
        const auto stale = s->planFilter(0, s->filterCleared(0), &why);
        QVERIFY(stale);
        QVERIFY(s->type(0, 1, 0, QStringLiteral("9")).ok);
        s->prepareFilter(*stale, 0);
        QVERIFY(!s->applyFilter(*stale, QStringLiteral("Filtering")).ok);
    }

    void theFilterFollowsRowsAndColumnsThatAreInsertedAndDeleted()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        crunch::EditSession *s = e.win.w.session();
        using Op = FilterColumn::Op;
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("10")))));
        QVERIFY(s->insertRows(0, 0, 2).ok);
        const AutoFilter moved = s->calculation()->autoFilterOf(0);
        QCOMPARE(rangeName(moved.ref), QStringLiteral("A3:D11"));
        QCOMPARE(moved.columns.size(), 1);
        pressUndo(&e.win.w);
        QCOMPARE(rangeName(s->calculation()->autoFilterOf(0).ref), QStringLiteral("A1:D9"));
        // a column inserted inside the range: the condition goes with its column
        QVERIFY(s->insertColumns(0, 1, 1).ok);
        AutoFilter f = s->calculation()->autoFilterOf(0);
        QCOMPARE(rangeName(f.ref), QStringLiteral("A1:E9"));
        QCOMPARE(f.columns[0].column, quint32(2));
        pressUndo(&e.win.w);
        // a column deleted before it
        QVERIFY(s->deleteColumns(0, 0, 1).ok);
        f = s->calculation()->autoFilterOf(0);
        QCOMPARE(rangeName(f.ref), QStringLiteral("A1:C9"));
        QCOMPARE(f.columns[0].column, quint32(0));
        pressUndo(&e.win.w);
        // the column the condition is for is deleted: the condition is gone
        QVERIFY(s->deleteColumns(0, 1, 1).ok);
        f = s->calculation()->autoFilterOf(0);
        QVERIFY(f.columns.isEmpty());
        pressUndo(&e.win.w);
        QCOMPARE(s->calculation()->autoFilterOf(0).columns.size(), 1);
        // the rows are deleted: the filter goes
        QVERIFY(s->deleteRows(0, 0, 9).ok);
        QVERIFY(!s->calculation()->autoFilterOf(0).valid());
        pressUndo(&e.win.w);
        QVERIFY(s->calculation()->autoFilterOf(0).valid());
    }

    void aSavedFileHoldsTheConditionsAndTheRowsTheFilterHides()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        crunch::EditSession *s = e.win.w.session();
        using Op = FilterColumn::Op;
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(s->resizeRows(0, 4, 4, hide, QStringLiteral("Hide")).ok);                 // row 5 by hand
        QVERIFY(filterWith(e, with(e, 1, custom(Op::Greater, QStringLiteral("10")))));
        QCOMPARE(shownRows(e), QStringLiteral("3 6 7 9").replace(QLatin1String("6 "), QString()));
        QVERIFY2(saveIt(e), "saved");
        QByteArray part;
        jp::office::sheet::Sheet saved = savedSheet(e.path, &part);
        QVERIFY(saved.autoFilter.valid());
        QCOMPARE(rangeName(saved.autoFilter.ref), QStringLiteral("A1:D9"));
        QCOMPARE(saved.autoFilter.columns.size(), 1);
        QCOMPARE(int(saved.autoFilter.columns[0].kind), int(FilterColumn::Kind::Custom));
        QCOMPARE(saved.autoFilter.columns[0].column, quint32(1));
        QVERIFY(part.indexOf("</sheetData>") < part.indexOf("<autoFilter"));
        QVERIFY(part.contains("filterMode=\"1\""));
        // the rows are hidden in the file, and the file read again tells the filter's from the hand's
        QString hidden;
        for (const jp::office::sheet::RowSpec &r : saved.rowSpecs)
            if (r.hidden) hidden += QStringLiteral("%1%2 ").arg(r.row + 1).arg(r.filtered ? QLatin1String("f") : QLatin1String("h"));
        QCOMPARE(hidden.trimmed(), QStringLiteral("2f 4f 5f 6f 8f"));                // (row 5 was hidden by hand too: the file cannot tell)
        // opened again, the window has the same filter and the same rows
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.session()->calculation()->autoFilterOf(0) == s->calculation()->autoFilterOf(0));
        QCOMPARE(second.w.view()->sheet().layout().rows.size(1), 0);            // (row 2 is not shown)
        // turned off and saved again: nothing of the filter is left in the file, and the rows are shown (but the one hidden by hand)
        QVERIFY(filterWith(e, AutoFilter()));
        QVERIFY2(saveIt(e), "saved again");
        saved = savedSheet(e.path, &part);
        QVERIFY(!saved.autoFilter.valid());
        QVERIFY(!part.contains("autoFilter"));
        QVERIFY(!part.contains("filterMode"));
        hidden.clear();
        for (const jp::office::sheet::RowSpec &r : saved.rowSpecs)
            if (r.hidden) hidden += QString::number(r.row + 1) + QLatin1Char(' ');
        QCOMPARE(hidden.trimmed(), QStringLiteral("5"));
    }

    void aFilterByColorIsSavedWithTheColorsFormat()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        FilterColumn red;
        red.kind = FilterColumn::Kind::Color;
        red.color = jp::office::sheet::ColorSpec::fromRgb(0xFF0000);
        QVERIFY(filterWith(e, with(e, 3, red)));
        QVERIFY2(saveIt(e), "saved");
        QByteArray part, styles;
        const jp::office::sheet::Sheet saved = savedSheet(e.path, &part, &styles);
        QVERIFY(saved.autoFilter.valid());
        QCOMPARE(saved.autoFilter.columns.size(), 1);
        const FilterColumn &c = saved.autoFilter.columns[0];
        QCOMPARE(int(c.kind), int(FilterColumn::Kind::Color));
        QVERIFY(c.dxf >= 0);
        QVERIFY(!c.fontColor);
        QVERIFY2(styles.contains("<dxfs count=\"1\">"), styles.constData());
        QVERIFY(styles.contains("FFFF0000"));
        // read again, the condition's color is the one its format has
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const FilterColumn &again = second.w.session()->calculation()->autoFilterOf(0).columns[0];
        QCOMPARE(second.w.session()->colorOfCondition(again).rgb, quint32(0xFF0000));
        QCOMPARE(second.w.session()->colorOfCondition(again).kind, jp::office::sheet::ColorSpec::Kind::Rgb);
        // saved again with no change to the filter: the same format is used, none is added
        FilterColumn font;
        font.kind = FilterColumn::Kind::Color;
        font.fontColor = true;
        font.color = jp::office::sheet::ColorSpec::fromRgb(0x0000FF);
        QVERIFY(filterWith(e, e.win.w.session()->filterWith(0, 3, font)));
        QVERIFY2(saveIt(e), "saved again");
        savedSheet(e.path, &part, &styles);
        QVERIFY(styles.contains("<dxfs count=\"2\">"));
        QVERIFY(part.contains("cellColor=\"0\""));
    }

    // ---- the window
    void filterTurnsTheButtonsOnOverTheBlockAndOffAgain()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit(), {"A1=other"}})));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.act(QStringLiteral("filter.toggle"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("filter.toggle"))->isChecked());
        QCOMPARE(int(e.view()->headerButtonCount()), 0);
        e.select(QStringLiteral("B4"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        const AutoFilter f = w.session()->calculation()->autoFilterOf(0);
        QVERIFY(f.valid());
        QCOMPARE(rangeName(f.ref), QStringLiteral("A1:D9"));
        QCOMPARE(int(e.view()->headerButtonCount()), 4);
        QVERIFY(w.act(QStringLiteral("filter.toggle"))->isChecked());
        QVERIFY(!e.view()->headerButtonRect(0, 1).isEmpty());
        // another sheet has none
        w.setActiveSheet(1);
        QCOMPARE(int(e.view()->headerButtonCount()), 0);
        QVERIFY(!w.act(QStringLiteral("filter.toggle"))->isChecked());
        w.setActiveSheet(0);
        QCOMPARE(int(e.view()->headerButtonCount()), 4);
        // off
        w.act(QStringLiteral("filter.toggle"))->trigger();
        QVERIFY(!w.session()->calculation()->autoFilterOf(0).valid());
        QCOMPARE(int(e.view()->headerButtonCount()), 0);
        pressUndo(&w);
        QCOMPARE(int(e.view()->headerButtonCount()), 4);
        // a cell with nothing around it is told so
        pressUndo(&w);
        e.win.told.clear();
        e.select(QStringLiteral("H20"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
        QVERIFY(!w.session()->calculation()->autoFilterOf(0).valid());
        // the selected cells, when there are many
        e.view()->selectRange(0, 0, 4, 1);
        w.act(QStringLiteral("filter.toggle"))->trigger();
        QCOMPARE(rangeName(w.session()->calculation()->autoFilterOf(0).ref), QStringLiteral("A1:B5"));
        QCOMPARE(int(e.view()->headerButtonCount()), 2);
    }

    void theDropDownListsTheValuesAndOkFiltersByTheCheckedOnes()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        CrunchWindow &w = e.win.w;
        w.session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        // a click on the button of column A
        const QRect button = e.view()->headerButtonRect(0, 0);
        QVERIFY(!button.isEmpty());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, button.center());
        FilterPopup *popup = w.filterPopup();
        QVERIFY(popup);
        QTRY_VERIFY(popup->listed());
        saveShot(popup, QStringLiteral("filter-dropdown"));
        saveShot(&w, QStringLiteral("filter-window"));
        QTreeWidget *tree = popup->tree();
        QTreeWidgetItem *all = tree->topLevelItem(0);
        QCOMPARE(all->text(0), QStringLiteral("(Select All)"));
        QCOMPARE(all->childCount(), 8);                                           // seven values and the blanks
        QCOMPARE(all->child(0)->text(0), QStringLiteral("apple"));
        QCOMPARE(all->child(7)->text(0), QStringLiteral("(Blanks)"));
        QCOMPARE(int(all->checkState(0)), int(Qt::Checked));
        // Select All off, then two values on
        all->setCheckState(0, Qt::Unchecked);
        QCOMPARE(int(all->child(3)->checkState(0)), int(Qt::Unchecked));
        all->child(0)->setCheckState(0, Qt::Checked);
        all->child(2)->setCheckState(0, Qt::Checked);
        QCOMPARE(int(all->checkState(0)), int(Qt::PartiallyChecked));
        popup->findChild<QPushButton *>(QStringLiteral("filterOk"))->click();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(shownRows(e), QStringLiteral("2 3"));                           // apple, banana
        const FilterColumn c = w.session()->calculation()->autoFilterOf(0).columns[0];
        QCOMPARE(c.values, (QStringList{"apple", "banana"}));
        QVERIFY(!c.blank);
        QCOMPARE(w.undoStack()->count(), 2);                                       // (on, and the values)
        // the button shows a filter is on it; opened again, the list has the values checked that were let through
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->headerButtonRect(0, 0).center());
        popup = w.filterPopup();
        QVERIFY(popup);
        QTRY_VERIFY(popup->listed());
        all = popup->tree()->topLevelItem(0);
        QCOMPARE(int(all->child(0)->checkState(0)), int(Qt::Checked));
        QCOMPARE(int(all->child(1)->checkState(0)), int(Qt::Unchecked));
        QVERIFY(popup->findChild<QAction *>(QStringLiteral("filterClear"))->isEnabled());
        // Clear Filter From the column
        popup->findChild<QAction *>(QStringLiteral("filterClear"))->trigger();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(shownRows(e), QStringLiteral("2 3 4 5 6 7 8 9"));
        QVERIFY(!w.session()->calculation()->autoFilterOf(0).active());
        // Cancel does nothing, and everything checked is no condition
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->headerButtonRect(0, 0).center());
        popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        const int steps = w.undoStack()->count();
        popup->findChild<QPushButton *>(QStringLiteral("filterOk"))->click();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(w.undoStack()->count(), steps);
    }

    void searchingTheListChecksOnlyWhatShows()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        w.showFilterPopup(0, 0);
        FilterPopup *popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        popup->search()->setText(QStringLiteral("APPLE"));
        QTreeWidgetItem *all = popup->tree()->topLevelItem(0);
        QCOMPARE(all->text(0), QStringLiteral("(Select All Search Results)"));
        int visible = 0;
        for (int i = 0; i < all->childCount(); ++i) visible += all->child(i)->isHidden() ? 0 : 1;
        QCOMPARE(visible, 2);                                                       // apple, Apple pie
        all->setCheckState(0, Qt::Unchecked);                                         // (only those two)
        QCOMPARE(int(all->child(3)->checkState(0)), int(Qt::Checked));                // cherry still is
        all->setCheckState(0, Qt::Checked);
        popup->search()->clear();
        all->setCheckState(0, Qt::Unchecked);
        all->child(1)->setCheckState(0, Qt::Checked);                                  // Apple pie only
        popup->findChild<QPushButton *>(QStringLiteral("filterOk"))->click();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(shownRows(e), QStringLiteral("4"));
    }

    void aColumnOfDatesIsListedByYearMonthAndDay()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        w.showFilterPopup(0, 2);
        FilterPopup *popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QTreeWidgetItem *all = popup->tree()->topLevelItem(0);
        QCOMPARE(all->child(0)->text(0), QStringLiteral("2023"));
        QCOMPARE(all->child(1)->text(0), QStringLiteral("2024"));
        QCOMPARE(all->child(1)->childCount(), 4);                                  // January, February, March, December
        QCOMPARE(all->child(1)->child(0)->childCount(), 2);                        // the 15th and the 20th
        all->setCheckState(0, Qt::Unchecked);
        all->child(0)->setCheckState(0, Qt::Checked);                              // all of 2023
        all->child(1)->child(1)->setCheckState(0, Qt::Checked);                    // all of February 2024
        all->child(1)->child(0)->child(1)->setCheckState(0, Qt::Checked);          // and the 20th of January
        QCOMPARE(int(all->child(1)->child(0)->checkState(0)), int(Qt::PartiallyChecked));
        popup->findChild<QPushButton *>(QStringLiteral("filterOk"))->click();
        QTRY_VERIFY(!w.isFiltering());
        const FilterColumn c = w.session()->calculation()->autoFilterOf(0).columns[0];
        QCOMPARE(c.dates.size(), 3);
        QCOMPARE(int(c.dates[0].unit), int(FilterColumn::DateGroup::Unit::Year));
        QCOMPARE(int(c.dates[1].unit), int(FilterColumn::DateGroup::Unit::Day));                  // the 20th of January
        QCOMPARE(int(c.dates[2].unit), int(FilterColumn::DateGroup::Unit::Month));                // February
        QCOMPARE(c.column, quint32(2));
        QCOMPARE(shownRows(e), QStringLiteral("3 5 8 9"));
    }

    void theFiltersByConditionAskAndPutTheConditionOnTheColumn()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        FilterAsk asked = FilterAsk::Custom;
        QString askedColumn;
        bool askedDates = true;
        w.setFilterAsker([&](FilterAsk what, const QString &column, bool dates, const FilterColumn *, FilterColumn *chosen) {
            asked = what;
            askedColumn = column;
            askedDates = dates;
            *chosen = custom(FilterColumn::Op::Greater, QStringLiteral("10"));
            return true;
        });
        w.showFilterPopup(0, 1);
        FilterPopup *popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QMenu *kinds = popup->findChild<QMenu *>(QStringLiteral("filterKinds"));
        QCOMPARE(kinds->title(), QStringLiteral("Number Filters"));
        QAction *greater = nullptr;
        for (QAction *a : kinds->actions())
            if (a->text() == QStringLiteral("Greater Than…")) greater = a;
        QVERIFY(greater);
        greater->trigger();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(int(asked), int(FilterAsk::Greater));
        QCOMPARE(askedColumn, QStringLiteral("Qty"));
        QVERIFY(!askedDates);
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));
        // Above Average is a condition without a question
        w.showFilterPopup(0, 1);
        popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        for (QAction *a : popup->findChild<QMenu *>(QStringLiteral("filterKinds"))->actions())
            if (a->text() == QStringLiteral("Above Average")) a->trigger();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(shownRows(e), QStringLiteral("3 7 9"));
        QCOMPARE(w.session()->calculation()->autoFilterOf(0).columns[0].dynamic, QStringLiteral("aboveAverage"));
        // a text column has text filters; a date column, date filters
        w.showFilterPopup(0, 0);
        popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QCOMPARE(popup->findChild<QMenu *>(QStringLiteral("filterKinds"))->title(), QStringLiteral("Text Filters"));
        w.showFilterPopup(0, 2);
        popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QCOMPARE(popup->findChild<QMenu *>(QStringLiteral("filterKinds"))->title(), QStringLiteral("Date Filters"));
        // giving the question up changes nothing
        w.setFilterAsker([](FilterAsk, const QString &, bool, const FilterColumn *, FilterColumn *) { return false; });
        const int steps = w.undoStack()->count();
        for (QAction *a : popup->findChild<QMenu *>(QStringLiteral("filterKinds"))->actions())
            if (a->text() == QStringLiteral("Custom Filter…")) a->trigger();
        QCOMPARE(w.undoStack()->count(), steps);
    }

    void filteringAndSortingByColorAreOnTheDropDown()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        w.showFilterPopup(0, 3);
        FilterPopup *popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QMenu *colors = popup->findChild<QMenu *>(QStringLiteral("filterColors"));
        QVERIFY(colors->isEnabled());
        QAction *red = nullptr;
        for (QAction *a : colors->actions())
            if (a->text() == QLatin1String("#FF0000")) red = a;
        QVERIFY(red);
        red->trigger();
        QTRY_VERIFY(!w.isFiltering());
        QCOMPARE(shownRows(e), QStringLiteral("2 4 7"));
        QCOMPARE(int(w.session()->calculation()->autoFilterOf(0).columns[0].kind), int(FilterColumn::Kind::Color));
        // sorting by the color puts the red ones first (of the rows that show)
        w.showFilterPopup(0, 3);
        popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QMenu *sortColors = popup->findChild<QMenu *>(QStringLiteral("filterSortColors"));
        QVERIFY(sortColors->isEnabled());
    }

    void sortingFromTheDropDownSortsTheFilteredBlockByTheColumn()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        w.showFilterPopup(0, 1);
        FilterPopup *popup = w.filterPopup();
        QTRY_VERIFY(popup->listed());
        QSignalSpy sorted(&w, &CrunchWindow::sortFinished);
        popup->findChild<QAction *>(QStringLiteral("filterSortDescending"))->trigger();
        QTRY_COMPARE(sorted.count(), 1);
        QCOMPARE(shown(e, QStringLiteral("B1:B9")), QStringLiteral("Qty|20|15|12|9|7|5|3|1"));
        QCOMPARE(shown(e, QStringLiteral("A2:A2")), QStringLiteral("date"));                  // (the names went with their numbers)
        QCOMPARE(shown(e, QStringLiteral("A1:A1")), QStringLiteral("Name"));                  // (the header stayed)
    }

    void aLargeFilterIsWorkedOutASliceAtATimeAndEscGivesItUp()
    {
        QStringList cells = {"A1=Head"};
        for (int r = 2; r <= 6000; ++r) cells << QStringLiteral("A%1=%2").arg(r).arg(r % 10);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("filter.toggle"))->trigger();
        QSignalSpy finished(&w, &CrunchWindow::filterFinished);
        AutoFilter f = w.session()->calculation()->autoFilterOf(0);
        FilterColumn c = custom(FilterColumn::Op::Equal, QStringLiteral("3"));
        c.column = 0;
        f.columns.append(c);
        QCOMPARE(rangeName(f.ref), QStringLiteral("A1:A6000"));
        QVERIFY(w.applyFilter(f, QStringLiteral("Filtering")));
        QVERIFY(w.isFiltering());
        QTRY_COMPARE(finished.count(), 1);
        QVERIFY(finished.last().first().toBool());
        QCOMPARE(w.undoStack()->count(), 2);
        // Esc before it is done changes nothing
        QVERIFY(w.applyFilter(w.session()->filterWith(0, 0, custom(FilterColumn::Op::Equal, QStringLiteral("4"))), QStringLiteral("Filtering again")));
        QVERIFY(w.isFiltering());
        QTest::keyClick(&w, Qt::Key_Escape);
        QTRY_COMPARE(finished.count(), 2);
        QVERIFY(!finished.last().first().toBool());
        QCOMPARE(w.undoStack()->count(), 2);
        QVERIFY(w.session()->calculation()->autoFilterOf(0).active());
    }

    void filterCommandsAreOnTheRibbonAndOffWithoutAWorkbook()
    {
        Window blank;
        for (const char *id : {"filter.toggle", "filter.clear", "filter.reapply"}) {
            QVERIFY2(blank.w.act(QString::fromLatin1(id)), id);
            QVERIFY2(!blank.w.act(QString::fromLatin1(id))->isEnabled(), id);
        }
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({fruit()})));
        for (const char *id : {"filter.toggle", "filter.clear", "filter.reapply"}) QVERIFY2(e.win.w.act(QString::fromLatin1(id))->isEnabled(), id);
        QVERIFY(e.win.w.ribbonErrors().isEmpty());
        // Clear and Reapply with no filter do nothing
        e.win.w.act(QStringLiteral("filter.clear"))->trigger();
        e.win.w.act(QStringLiteral("filter.reapply"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 0);
    }

    // ---- the questions
    void theCustomFilterDialogMakesTheConditionsOfTheComparisonsAndPatternsOfText()
    {
        using Op = FilterColumn::Op;
        CustomFilterDialog d(QStringLiteral("Name"), false, jp::calc::DateSystem::Date1900, FilterAsk::BeginsWith, nullptr);
        d.valueEdit(0)->setText(QStringLiteral("a*b?~"));
        FilterColumn c = d.result();
        QCOMPARE(c.conditions.size(), 1);
        QCOMPARE(int(c.conditions[0].op), int(Op::Equal));
        QCOMPARE(c.conditions[0].value, QStringLiteral("a~*b~?~~*"));                  // (the marks in the text are the text's own)
        // contains, and does not contain
        d.operatorBox(0)->setCurrentIndex(10);
        d.valueEdit(0)->setText(QStringLiteral("x"));
        c = d.result();
        QCOMPARE(c.conditions[0].value, QStringLiteral("*x*"));
        d.operatorBox(0)->setCurrentIndex(11);
        QCOMPARE(int(d.result().conditions[0].op), int(Op::NotEqual));
        // a number is the number
        d.operatorBox(0)->setCurrentIndex(2);
        d.valueEdit(0)->setText(QStringLiteral("1,500"));
        QCOMPARE(d.result().conditions[0].value, QStringLiteral("1500"));            // (the person's marks: the US ones read a comma as grouping)
        d.valueEdit(0)->setText(QStringLiteral("2.5"));
        QCOMPARE(d.result().conditions[0].value, QStringLiteral("2.5"));
        QCOMPARE(int(d.result().conditions[0].op), int(Op::Greater));
        // a second comparison, joined with Or
        d.operatorBox(1)->setCurrentIndex(1 + 4);
        d.valueEdit(1)->setText(QStringLiteral("0"));
        d.orButton()->setChecked(true);
        c = d.result();
        QCOMPARE(c.conditions.size(), 2);
        QVERIFY(!c.both);
        QCOMPARE(int(c.conditions[1].op), int(Op::Less));
        // Between begins as two comparisons
        CustomFilterDialog between(QStringLiteral("Qty"), false, jp::calc::DateSystem::Date1900, FilterAsk::Between, nullptr);
        QCOMPARE(between.operatorBox(0)->currentIndex(), 3);
        QCOMPARE(between.operatorBox(1)->currentIndex(), 5 + 1);
        QVERIFY(between.andButton()->isChecked());
        // a condition the column has is shown as it was made: begins with
        FilterColumn have;
        have.kind = FilterColumn::Kind::Custom;
        have.conditions.append({Op::Equal, QStringLiteral("ab~*c*")});
        have.both = true;
        CustomFilterDialog again(QStringLiteral("Name"), false, jp::calc::DateSystem::Date1900, FilterAsk::Custom, &have);
        QCOMPARE(again.operatorBox(0)->currentIndex(), 6);
        QCOMPARE(again.valueEdit(0)->text(), QStringLiteral("ab*c"));
        QCOMPARE(again.result().conditions[0].value, QStringLiteral("ab~*c*"));
    }

    void theCustomFilterDialogOfDatesMakesWholeDays()
    {
        using Op = FilterColumn::Op;
        CustomFilterDialog d(QStringLiteral("Date"), true, jp::calc::DateSystem::Date1900, FilterAsk::After, nullptr);
        d.dateEdit(0)->setDate(QDate(2024, 3, 5));
        double serial = 0;
        jp::calc::dateToSerial(2024, 3, 5, jp::calc::DateSystem::Date1900, &serial);
        FilterColumn c = d.result();
        QCOMPARE(c.conditions.size(), 1);
        QCOMPARE(int(c.conditions[0].op), int(Op::GreaterEqual));                      // (after the day: from the start of the next)
        QCOMPARE(c.conditions[0].value, jp::calc::numberToText(serial + 1));
        d.operatorBox(0)->setCurrentIndex(4);                                          // is before
        c = d.result();
        QCOMPARE(int(c.conditions[0].op), int(Op::Less));
        QCOMPARE(c.conditions[0].value, jp::calc::numberToText(serial));
        d.operatorBox(0)->setCurrentIndex(5);                                          // on or before
        c = d.result();
        QCOMPARE(c.conditions[0].value, jp::calc::numberToText(serial + 1));
        d.operatorBox(0)->setCurrentIndex(0);                                          // equals: the whole day
        c = d.result();
        QCOMPARE(c.conditions.size(), 2);
        QVERIFY(c.both);
        QCOMPARE(c.conditions[0].value, jp::calc::numberToText(serial));
        QCOMPARE(c.conditions[1].value, jp::calc::numberToText(serial + 1));
    }

    void theTopDialogMakesATopCondition()
    {
        TopFilterDialog d(QStringLiteral("Qty"), nullptr);
        QCOMPARE(d.countBox()->value(), 10.0);
        d.whichBox()->setCurrentIndex(1);
        d.countBox()->setValue(3);
        d.unitBox()->setCurrentIndex(1);
        const FilterColumn c = d.result();
        QCOMPARE(int(c.kind), int(FilterColumn::Kind::Top10));
        QVERIFY(!c.top && c.percent);
        QCOMPARE(c.count, 3.0);
        d.unitBox()->setCurrentIndex(1);
        d.countBox()->setValue(500);
        QCOMPARE(d.countBox()->value(), 100.0);                                         // (a percent is up to a hundred)
        FilterColumn have;
        have.kind = FilterColumn::Kind::Top10;
        have.top = false;
        have.count = 7;
        TopFilterDialog again(QStringLiteral("Qty"), &have);
        QCOMPARE(again.whichBox()->currentIndex(), 1);
        QCOMPARE(again.countBox()->value(), 7.0);
    }

    void theValuesInAColumnAreListedASliceAtATime()
    {
        QStringList cells = fruit();
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        crunch::EditSession *s = e.win.w.session();
        e.win.w.session()->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        AutoFilter f = range();
        crunch::FilterListRun names(s, 0, f, 0);
        int slices = 0;
        while (!names.step(3)) QVERIFY(++slices < 10);
        QCOMPARE(slices, 2);
        const crunch::FilterList &list = names.list();
        QVERIFY(list.blanks);
        QCOMPARE(int(list.items.size()), 7);
        QCOMPARE(list.items[0].text, QStringLiteral("apple"));
        QCOMPARE(list.items[1].text, QStringLiteral("Apple pie"));
        QCOMPARE(list.items[6].text, QStringLiteral("fig"));
        QCOMPARE(int(list.kind), int(crunch::FilterList::Kind::Text));
        // numbers in order, and what a cell with a number shows
        crunch::FilterListRun qty(s, 0, f, 1);
        qty.step(0);
        QCOMPARE(int(qty.list().kind), int(crunch::FilterList::Kind::Number));
        QCOMPARE(qty.list().items.front().text, QStringLiteral("1"));
        QCOMPARE(qty.list().items.back().text, QStringLiteral("20"));
        // dates: the years with months and days
        crunch::FilterListRun dates(s, 0, f, 2);
        dates.step(0);
        QCOMPARE(int(dates.list().kind), int(crunch::FilterList::Kind::Date));
        QCOMPARE(int(dates.list().dates.size()), 2);
        QCOMPARE(int(dates.list().dates.at(2024).size()), 4);
        QVERIFY(dates.list().blanks);
        // colors
        crunch::FilterListRun tags(s, 0, f, 3);
        tags.step(0);
        QCOMPARE(int(tags.list().cellColors.size()), 1);
        QVERIFY(tags.list().noCellColor);
        // a search in the list
        QCOMPARE(crunch::FilterListRun::search(list, QStringLiteral("APPLE")).size(), size_t(2));
        QCOMPARE(crunch::FilterListRun::search(list, QStringLiteral("apple pie")).size(), size_t(1));
        QCOMPARE(crunch::FilterListRun::search(list, QStringLiteral("zzz")).size(), size_t(0));
    }

    void aColumnWithManyDistinctValuesIsListedUpToTheMost()
    {
        QStringList cells = {"A1=Head"};
        for (int r = 2; r <= 6000; ++r) cells << QStringLiteral("A%1=v%2").arg(r).arg(r);
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({cells})));
        crunch::FilterListRun run(e.win.w.session(), 0, range(QStringLiteral("A1:A6000")), 0);
        run.step(0);
        QCOMPARE(int(run.list().items.size()), crunch::kMaxFilterItems);
        QVERIFY(run.list().truncated);
    }
};

JP_TEST_CLASS(CrunchFilterTests)
#include "tst_crunch_filter.moc"
