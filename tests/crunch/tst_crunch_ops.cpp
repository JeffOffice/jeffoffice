// Tests of what a person does with a block of cells besides typing in it: AutoSum and the functions of its menu, fill, the clipboard, inserting and deleting, the sizes of rows and
// columns, and the rules of validation. Every workbook is built here, in a temporary folder; each step is one on the undo stack.
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
#include <QClipboard>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// Numbers to total:
//        A    B    C    D
//   1    1    2    x
//   2    3    4
//   3    5    6
//   4
//   5    7    8
QByteArray numbersBook()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\" t=\"inlineStr\"><is><t>x</t></is></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>3</v></c><c r=\"B2\"><v>4</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\"><v>5</v></c><c r=\"B3\"><v>6</v></c></row>"
                                        "<row r=\"5\"><c r=\"A5\"><v>7</v></c><c r=\"B5\"><v>8</v></c></row>");
    return buildXlsx({worksheet(rows)});
}

} // namespace

class CrunchOpsTests : public QObject {
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

    // ---- fill

    // The cells past a block go on as the block does: numbers by their step (a trend through several), a single number copied, text that ends in a number counting up (with its zeros),
    // the names of weekdays and months in turn (in their capitals), and a formula with its references moved; up and left go the other way.
    void fillGoesOnAsTheBlockDoes()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1=5", "C2=3", "D1=7", "E1=Item 1", "E2=Item 2", "F1=Item 009", "G1=Mon", "G2=Tue", "H1=january", "I1=Dec", "J1=x", "K1==A1*2+$A$1+B$1", "L1=1", "L2=2",
                                 "L3=4", "M1=MONDAY", "N1=Sat", "N2=Sun", "O1=1.5", "O2=2", "A10=3", "A11=4", "P20=1", "Q20=2", "R1=@2024-01-15", "R2=@2024-02-15", "S1=@2024-03-01", "S2=@2025-03-01",
                                 "T1=@2024-03-01", "U1=%0.5"})));
        using Dir = crunch::EditSession::FillDirection;
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        auto fill = [&](const QString &ref, Dir d, quint32 n) {
            const crunch::EditSession::Outcome o = s->fill(0, rangeOf(ref), d, n);
            QVERIFY2(o.ok, qPrintable(o.problem));
        };
        fill(QStringLiteral("A1:A2"), Dir::Down, 3);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|2|3|4|5"));
        fill(QStringLiteral("C1:C2"), Dir::Down, 3);
        QCOMPARE(shown(e, QStringLiteral("C1:C5")), QStringLiteral("5|3|1|-1|-3"));
        fill(QStringLiteral("D1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("D1:D3")), QStringLiteral("7|7|7"));                                  // (a single number is copied)
        fill(QStringLiteral("E1:E2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("E1:E4")), QStringLiteral("Item 1|Item 2|Item 3|Item 4"));
        fill(QStringLiteral("F1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("F1:F3")), QStringLiteral("Item 009|Item 010|Item 011"));
        fill(QStringLiteral("G1:G2"), Dir::Down, 3);
        QCOMPARE(shown(e, QStringLiteral("G1:G5")), QStringLiteral("Mon|Tue|Wed|Thu|Fri"));
        fill(QStringLiteral("H1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("H1:H3")), QStringLiteral("january|february|march"));
        fill(QStringLiteral("I1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("I1:I3")), QStringLiteral("Dec|Jan|Feb"));
        fill(QStringLiteral("J1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("J1:J3")), QStringLiteral("x|x|x"));
        fill(QStringLiteral("K1"), Dir::Down, 2);
        QCOMPARE(s->editText(0, 1, 10), QStringLiteral("=A2*2+$A$1+B$1"));
        QCOMPARE(s->editText(0, 2, 10), QStringLiteral("=A3*2+$A$1+B$1"));
        fill(QStringLiteral("L1:L3"), Dir::Down, 1);                                                         // 1, 2, 4: the trend through them
        QCOMPARE(s->editText(0, 3, 11), QStringLiteral("5.33333333333333"));
        fill(QStringLiteral("M1"), Dir::Down, 1);
        QCOMPARE(shown(e, QStringLiteral("M1:M2")), QStringLiteral("MONDAY|TUESDAY"));
        fill(QStringLiteral("N1:N2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("N1:N4")), QStringLiteral("Sat|Sun|Mon|Tue"));
        fill(QStringLiteral("O1:O2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("O1:O4")), QStringLiteral("1.5|2|2.5|3"));
        fill(QStringLiteral("A10:A11"), Dir::Up, 2);                                                         // up goes the other way
        QCOMPARE(shown(e, QStringLiteral("A8:A11")), QStringLiteral("1|2|3|4"));
        fill(QStringLiteral("P20:Q20"), Dir::Right, 2);
        QCOMPARE(shown(e, QStringLiteral("P20:S20")), QStringLiteral("1|2|3|4"));
        fill(QStringLiteral("Q20:P20"), Dir::Left, 1);                                                       // (the block P20:Q20 again: left of it)
        QCOMPARE(shown(e, QStringLiteral("O20:P20")), QStringLiteral("0|1"));                                  // (1 and 2 going left: 0)
        // dates: months, years, and a single date by days
        fill(QStringLiteral("R1:R2"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("R1:R4")), QStringLiteral("1/15/2024|2/15/2024|3/15/2024|4/15/2024"));
        fill(QStringLiteral("S1:S2"), Dir::Down, 1);
        QCOMPARE(shown(e, QStringLiteral("S3")), QStringLiteral("3/1/2026"));
        fill(QStringLiteral("T1"), Dir::Down, 2);
        QCOMPARE(shown(e, QStringLiteral("T1:T3")), QStringLiteral("3/1/2024|3/2/2024|3/3/2024"));
        // formats follow the cells
        fill(QStringLiteral("U1"), Dir::Down, 1);
        QCOMPARE(shown(e, QStringLiteral("U1:U2")), QStringLiteral("50%|50%"));
        // one step each
        QCOMPARE(e.win.w.undoStack()->count(), 21);
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("U2")), QString());
    }

    // The names of weekdays and months go in turn in the person's language (the locale is set here), and the English ones too.
    void fillNamesInTheUsersLanguage()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=Montag", "B1=Januar", "C1=Mon", "D1=Dezember"})));
        using Dir = crunch::EditSession::FillDirection;
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::German, QLocale::Germany));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("Montag|Dienstag|Mittwoch"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("B1:B3")), QStringLiteral("Januar|Februar|M\u00e4rz"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1")), Dir::Down, 2).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("Mon|Tue|Wed"));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("D1")), Dir::Down, 1).ok);
        QCOMPARE(shown(e, QStringLiteral("D1:D2")), QStringLiteral("Dezember|Januar"));
        s->setLocale(QLocale(QLocale::French, QLocale::France));
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A3")), Dir::Down, 1).ok);          // (a German name in a French locale: not a name there; copied)
        QCOMPARE(shown(e, QStringLiteral("A3:A4")), QStringLiteral("Mittwoch|Mittwoch"));
    }

    // Ctrl+D and Ctrl+R copy the first row (column) of the selection over the rest, and a single cell takes what is above (left of) it; one step each; the keys are the platform's.
    void controlDAndControlRFillWhatIsSelected()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=5", "B1=6", "A2=x"})));
        QAction *down = e.win.w.act(QStringLiteral("fill.down")), *right = e.win.w.act(QStringLiteral("fill.right"));
        QVERIFY(down && right && down->isEnabled() && right->isEnabled());
        QCOMPARE(down->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_D).toString(QKeySequence::NativeText));
        QCOMPARE(right->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_R).toString(QKeySequence::NativeText));
        e.view()->selectRange(0, 0, 3, 1);                                      // A1:B4
        const QKeyCombination d = down->shortcut()[0];
        QTest::keyClick(e.view(), d.key(), d.keyboardModifiers());
        QCOMPARE(shown(e, QStringLiteral("A1:B4")), QStringLiteral("5|6|5|6|5|6|5|6"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("D1"));                                          // a single cell takes the one above it (here none: nothing)
        down->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.select(QStringLiteral("A6"));
        down->trigger();                                                         // (A5 is empty: A6 is emptied of nothing)
        e.select(QStringLiteral("A5"));
        down->trigger();                                                         // A5 takes A4's 5
        QCOMPARE(shown(e, QStringLiteral("A5")), QStringLiteral("5"));
        e.view()->selectRange(0, 0, 0, 3);                                       // A1:D1
        const QKeyCombination r = right->shortcut()[0];
        QTest::keyClick(e.view(), r.key(), r.keyboardModifiers());
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("5|5|5|5"));
        e.select(QStringLiteral("E2"));                                          // a single cell takes the one to its left (D2: empty)
        right->trigger();
        QCOMPARE(shown(e, QStringLiteral("D2:E2")), QStringLiteral("|"));
        pressUndo(&e.win.w);
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("5|6||"));
        // up and left copy from the last row and column
        e.view()->selectRange(5, 0, 7, 0);                                       // A6:A8
        e.win.w.act(QStringLiteral("fill.up"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A6:A8")), QStringLiteral("||"));
        e.select(QStringLiteral("B8"));
        e.win.w.act(QStringLiteral("fill.left"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("A8:B8")), QStringLiteral("|"));
    }

    // The fill handle: a small square at the corner of the selection; dragging it along a column (a row) and letting go fills the series the block makes, or a copy with Ctrl; the
    // selection is then the block and what was filled.
    void theFillHandleFillsASeries()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "C1=Item 1", "E1=10", "E2=20"})));
        e.win.w.resize(1100, 640);
        e.view()->selectRange(0, 0, 1, 0);                                       // A1:A2
        QRect handle = e.view()->fillHandleRect();
        QVERIFY(!handle.isEmpty());
        QSignalSpy requested(e.view(), &grid::GridView::fillRequested);
        auto drag = [&](const QString &to, Qt::KeyboardModifiers modifiers) {
            QRect h = e.view()->fillHandleRect();
            QTest::mousePress(e.view()->viewport(), Qt::LeftButton, modifiers, h.center());
            quint32 row = 0, col = 0;
            parseCellRef(to, &row, &col);
            const QPoint at = e.view()->cellRect(row, col).center();
            QMouseEvent move(QEvent::MouseMove, QPointF(at), QPointF(e.view()->viewport()->mapToGlobal(at)), Qt::NoButton, Qt::LeftButton, modifiers);
            QApplication::sendEvent(e.view()->viewport(), &move);
            QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, modifiers, at);
        };
        drag(QStringLiteral("A5"), Qt::NoModifier);
        QCOMPARE(requested.count(), 1);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|2|3|4|5"));
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:A5"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // a single text that ends in a number counts up; with Ctrl it is copied
        e.select(QStringLiteral("C1"));
        drag(QStringLiteral("C3"), Qt::NoModifier);
        QCOMPARE(shown(e, QStringLiteral("C1:C3")), QStringLiteral("Item 1|Item 2|Item 3"));
        e.select(QStringLiteral("C1"));
        drag(QStringLiteral("D1"), Qt::NoModifier);                              // (to the right: along the row)
        QCOMPARE(shown(e, QStringLiteral("C1:D1")), QStringLiteral("Item 1|Item 2"));
        e.view()->selectRange(0, 4, 1, 4);                                       // E1:E2
        drag(QStringLiteral("E4"), Qt::ControlModifier);
        QCOMPARE(shown(e, QStringLiteral("E1:E4")), QStringLiteral("10|20|10|20"));
        // a drag that does not leave the block asks for nothing; a selection of whole columns has no handle
        const int count = requested.count();
        e.select(QStringLiteral("H1"));
        drag(QStringLiteral("H1"), Qt::NoModifier);
        QCOMPARE(requested.count(), count);
        e.view()->selectColumns(0, 1);
        QVERIFY(e.view()->fillHandleRect().isEmpty());
    }

    // Fill > Series asks how the series goes (kind, step, stop, unit) and fills the selection from its first row (column).
    void fillSeriesDialogFillsTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C1=2", "D2=@2024-03-01"})));
        using Kind = crunch::EditSession::FillKind;
        CrunchWindow::SeriesRequest wanted;
        int asked = 0;
        e.win.w.setSeriesAsker([&](CrunchWindow::SeriesRequest *r) {
            ++asked;
            const bool inRows = r->inRows;
            *r = wanted;
            r->inRows = inRows;
            return true;
        });
        wanted.options.kind = Kind::Linear;
        wanted.options.hasStep = true;
        wanted.options.step = 2;
        e.view()->selectRange(0, 0, 4, 0);                                       // A1:A5
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(asked, 1);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|3|5|7|9"));
        wanted.options.kind = Kind::Growth;
        wanted.options.step = 3;
        e.view()->selectRange(0, 2, 0, 5);                                       // C1:F1 (a row: the series is in rows)
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("C1:F1")), QStringLiteral("2|6|18|54"));
        wanted = CrunchWindow::SeriesRequest();
        wanted.options.kind = Kind::Date;
        wanted.options.unit = crunch::EditSession::DateUnit::Month;
        e.view()->selectRange(1, 3, 3, 3);                                       // D2:D4
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(shown(e, QStringLiteral("D2:D4")), QStringLiteral("3/1/2024|4/1/2024|5/1/2024"));
        // one cell selected: nothing to fill, and it says so
        const int steps = e.win.w.undoStack()->count();
        e.select(QStringLiteral("A9"));
        e.win.w.act(QStringLiteral("fill.series"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.told.isEmpty());
    }

    // The options of Fill Series: a single number counting up (or by a step), growth, a stop value, dates by weekdays and months (clamped to the month's end), and a copy.
    void fillSeriesOptions()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=7", "B1=2", "B2=4", "C1=1", "C2=2", "D1=@2024-03-01", "E1=@2024-01-31", "F1=1", "F2=2", "G1=10"})));
        using Dir = crunch::EditSession::FillDirection;
        using Kind = crunch::EditSession::FillKind;
        crunch::EditSession *s = e.win.w.session();
        s->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        crunch::EditSession::FillOptions linear;
        linear.kind = Kind::Linear;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2, linear).ok);
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("7|8|9"));
        crunch::EditSession::FillOptions stepped = linear;
        stepped.hasStep = true;
        stepped.step = 5;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("G1")), Dir::Down, 2, stepped).ok);
        QCOMPARE(shown(e, QStringLiteral("G1:G3")), QStringLiteral("10|15|20"));
        crunch::EditSession::FillOptions growth;
        growth.kind = Kind::Growth;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1:B2")), Dir::Down, 2, growth).ok);
        QCOMPARE(shown(e, QStringLiteral("B1:B4")), QStringLiteral("2|4|8|16"));
        crunch::EditSession::FillOptions stop = linear;
        stop.hasStop = true;
        stop.stop = 5;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("C1:C2")), Dir::Down, 10, stop).ok);
        QCOMPARE(shown(e, QStringLiteral("C1:C7")), QStringLiteral("1|2|3|4|5||"));                          // (it stops at the stop value)
        crunch::EditSession::FillOptions weekdays;
        weekdays.kind = Kind::Date;
        weekdays.unit = crunch::EditSession::DateUnit::Weekday;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("D1")), Dir::Down, 3, weekdays).ok);                       // a Friday: then Monday, Tuesday, Wednesday
        QCOMPARE(shown(e, QStringLiteral("D1:D4")), QStringLiteral("3/1/2024|3/4/2024|3/5/2024|3/6/2024"));
        crunch::EditSession::FillOptions months;
        months.kind = Kind::Date;
        months.unit = crunch::EditSession::DateUnit::Month;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("E1")), Dir::Down, 2, months).ok);
        QCOMPARE(shown(e, QStringLiteral("E1:E3")), QStringLiteral("1/31/2024|2/29/2024|3/31/2024"));
        crunch::EditSession::FillOptions copy;
        copy.kind = Kind::Copy;
        QVERIFY(s->fill(0, rangeOf(QStringLiteral("F1:F2")), Dir::Down, 3, copy).ok);
        QCOMPARE(shown(e, QStringLiteral("F1:F5")), QStringLiteral("1|2|1|2|1"));
    }

    // A fill that would change a cell that can't be changed (a locked cell of a protected sheet, a table's header) changes nothing and says why; a fill past the edge of the sheet stops
    // at it.
    void fillRefusesWhatCannotBeChanged()
    {
        using Dir = crunch::EditSession::FillDirection;
        {
            EditWindow e;
            const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"0\" applyProtection=\"1\"><protection locked=\"0\"/></xf>"));
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" s=\"1\"><v>2</v></c></row><row r=\"2\"><c r=\"B2\" s=\"1\"/></row>");
            QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\"/>"))}, QByteArray(), QString(), QString(), styles)));
            crunch::EditSession *s = e.win.w.session();
            const crunch::EditSession::Outcome locked = s->fill(0, rangeOf(QStringLiteral("A1")), Dir::Down, 2);   // A2 is a locked cell
            QVERIFY(!locked.ok);
            QVERIFY2(locked.problem.contains(QLatin1String("protected")), qPrintable(locked.problem));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1")), Dir::Down, 1).ok);                                   // B2 is not locked
            QCOMPARE(e.win.w.undoStack()->count(), 1);
            // the edge of the sheet: nothing past it
            QVERIFY(s->fill(0, rangeOf(QStringLiteral("B1048576")), Dir::Down, 5).ok);
            QCOMPARE(e.win.w.undoStack()->count(), 1);
        }
        {
            EditWindow e;
            const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Region</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>Q1</t></is></c></row><row r=\"3\"><c r=\"B3\"><v>1</v></c></row>");
            const QString table = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"T\" displayName=\"T\" ref=\"A1:B3\">"
                                                 "<tableColumns count=\"2\"><tableColumn id=\"1\" name=\"Region\"/><tableColumn id=\"2\" name=\"Q1\"/></tableColumns></table>");
            QVERIFY(e.open(withTables(buildXlsx({worksheet(rows)}), {table})));
            const crunch::EditSession::Outcome o = e.win.w.session()->fill(0, rangeOf(QStringLiteral("B3")), Dir::Up, 2);   // up to B1, a header
            QVERIFY(!o.ok);
            QVERIFY2(o.problem.contains(QLatin1String("names of its columns")), qPrintable(o.problem));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
        }
    }

    // ---- AutoSum

    // AutoSum on one cell below numbers puts =SUM over the numbers above it in the cell to be edited (Enter keeps it); with none above it takes the numbers to its left; with neither it
    // gives =SUM() with the cursor inside the brackets. The keys are the platform's.
    void autoSumGuessesTheRangeOfNumbers()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        QAction *sum = e.win.w.act(QStringLiteral("edit.autosum"));
        QVERIFY(sum && sum->isEnabled());
        QCOMPARE(sum->shortcut().toString(QKeySequence::NativeText), crunch::autoSumKey().toString(QKeySequence::NativeText));
        e.select(QStringLiteral("A4"));
        sum->trigger();
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A1:A3)"));
        QCOMPARE(e.selected(), QStringLiteral("A4"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:9"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // the numbers above stop at a blank cell or a text: A5 has the blank A4 above it... now filled, so A6 sums A1:A5
        e.select(QStringLiteral("A6"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A1:A5)"));
        e.key(Qt::Key_Escape);
        // a cell with numbers to its left only
        e.select(QStringLiteral("C5"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A5:B5)"));
        e.key(Qt::Key_Escape);
        // a text above stops the guess, and the numbers to the left are taken
        e.select(QStringLiteral("C2"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A2:B2)"));
        e.key(Qt::Key_Escape);
        // nothing to guess: the brackets, with the cursor between them
        e.select(QStringLiteral("F9"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM()"));
        QCOMPARE(e.win.w.cellEditor()->cursorPosition(), 5);
        e.key(Qt::Key_Escape);
        // and the keys do it
        e.select(QStringLiteral("B4"));
        const QKeyCombination combo = crunch::autoSumKey()[0];
        QTest::keyClick(&e.win.w, combo.key(), combo.keyboardModifiers());
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(B1:B3)"));
        e.key(Qt::Key_Escape);
    }

    // The menu of AutoSum: Average, Count Numbers, Max, and Min work the same way.
    void autoSumsMenuHasTheOtherFunctions()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        const QList<QPair<QString, QString>> functions = {{QStringLiteral("edit.autosum.average"), QStringLiteral("=AVERAGE(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.count"), QStringLiteral("=COUNT(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.max"), QStringLiteral("=MAX(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.min"), QStringLiteral("=MIN(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.sum"), QStringLiteral("=SUM(B1:B3)")}};
        for (const auto &f : functions) {
            e.select(QStringLiteral("B4"));
            QAction *a = e.win.w.act(f.first);
            QVERIFY2(a && a->isEnabled(), qPrintable(f.first));
            a->trigger();
            QVERIFY(e.win.w.isEditing());
            QCOMPARE(e.win.w.cellEditor()->text(), f.second);
            e.key(Qt::Key_Escape);
        }
        e.select(QStringLiteral("B4"));
        e.win.w.act(QStringLiteral("edit.autosum.average"))->trigger();
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:4"));
    }

    // A block that is selected gets its totals under it (a column each) in one step, or to the right when it is one row; a last row (column) of the selection that is empty is where they go.
    void autoSumOfABlockPutsTheTotalsAtItsEnd()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        e.view()->selectRange(0, 0, 2, 1);                    // A1:B3
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(!e.win.w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:9"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:12"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("AutoSum"));
        pressUndo(&e.win.w);
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("-"));
        // a selection whose last row is empty: the totals go there
        e.view()->selectRange(0, 0, 3, 1);                    // A1:B4
        e.win.w.act(QStringLiteral("edit.autosum.max"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:5"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:6"));
        // one row: the total goes to the right
        e.view()->selectRange(4, 0, 4, 1);                    // A5:B5
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C5")), QStringLiteral("n:15"));
        // a column of text and nothing to total changes nothing
        const int steps = e.win.w.undoStack()->count();
        e.view()->selectRange(0, 2, 0, 3);                    // C1:D1 (a text, and nothing)
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }
};

JP_TEST_CLASS(CrunchOpsTests)
#include "tst_crunch_ops.moc"
