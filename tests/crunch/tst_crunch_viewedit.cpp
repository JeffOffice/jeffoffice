// Tests of the grid's response to rows and columns put in and taken out (Round 8 of docs/jeffcrunch-rowids.md): the sizes of rows and columns, the hidden ones, the frozen panes, the
// selection, and the scroll go with the edit, in place, and only what changed is drawn again. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/axis.h"
#include "grid/gridsheet.h"
#include "grid/gridview.h"

#include <QApplication>
#include <QScrollBar>
#include <QSettings>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// `rows` rows of a number and a formula that doubles it. Row 3 is 30 points high, row 10 is 45 points, rows 12 and 13 are hidden (a run), row 20 is 60 points, and row 100 is 45 points. With
// `everyHeight` each row has a height of its own. Columns: B is 12 wide, D and E are hidden (a run), F is 20 wide, H is 30 wide.
QByteArray viewBook(int rows = 300, bool everyHeight = false)
{
    QString xml;
    for (int r = 1; r <= rows; ++r) {
        QString attrs;
        if (r == 3) attrs = QStringLiteral(" ht=\"30\" customHeight=\"1\"");
        else if (r == 10 || r == 100) attrs = QStringLiteral(" ht=\"45\" customHeight=\"1\"");
        else if (r == 20) attrs = QStringLiteral(" ht=\"60\" customHeight=\"1\"");
        else if (r == 12 || r == 13) attrs = QStringLiteral(" hidden=\"1\"");
        else if (everyHeight) attrs = QStringLiteral(" ht=\"%1\" customHeight=\"1\"").arg(16 + r % 5);
        xml += QStringLiteral("<row r=\"%1\"%2><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2</f><v>%3</v></c></row>").arg(r).arg(attrs).arg(r * 2);
    }
    const QString cols = QStringLiteral("<cols><col min=\"2\" max=\"2\" width=\"12\" customWidth=\"1\"/><col min=\"4\" max=\"5\" width=\"9\" hidden=\"1\"/><col min=\"6\" max=\"6\" width=\"20\" customWidth=\"1\"/>"
                                        "<col min=\"8\" max=\"8\" width=\"30\" customWidth=\"1\"/></cols>");
    return buildXlsx({worksheet(xml, QString(), cols)});
}

constexpr int kRowsLooked = 330, kColsLooked = 40;

struct Sizes {
    std::vector<int> rows, cols;
    bool operator==(const Sizes &o) const { return rows == o.rows && cols == o.cols; }
};

Sizes sizesOf(EditWindow &e)
{
    const grid::GridLayout &L = e.view()->sheet().layout();
    Sizes s;
    for (int r = 0; r < kRowsLooked; ++r) s.rows.push_back(L.rows.size(r));
    for (int c = 0; c < kColsLooked; ++c) s.cols.push_back(L.cols.size(c));
    return s;
}

// The geometry the sheet would be shown with if it were shown again: every size of rows and columns, as the sheet gives them.
bool sameAsShownAgain(EditWindow &e, QString *why)
{
    const grid::GridSheet &gs = e.view()->sheet();
    const auto fresh = grid::GridSheet::makeGeometry(*gs.data().sheet, *gs.data().styles);
    const auto same = [&](const grid::Axis &a, const grid::Axis &b, const char *what) {
        const auto pa = a.places(), pb = b.places();
        if (pa.size() != pb.size()) {
            *why = QStringLiteral("%1: %2 places, shown again %3").arg(QLatin1String(what)).arg(pa.size()).arg(pb.size());
            return false;
        }
        for (size_t i = 0; i < pa.size(); ++i)
            if (pa[i].index != pb[i].index || pa[i].size != pb[i].size) {
                *why = QStringLiteral("%1: place %2 is %3 of %4, shown again %5 of %6").arg(QLatin1String(what)).arg(i).arg(pa[i].index).arg(pa[i].size).arg(pb[i].index).arg(pb[i].size);
                return false;
            }
        return true;
    };
    return same(gs.layout().rows, fresh->layout.rows, "rows") && same(gs.layout().cols, fresh->layout.cols, "columns");
}

// The window draws what is waiting to be drawn: the events are taken until the view has drawn nothing for a few turns.
void flushPaints(grid::GridView *view)
{
    int quiet = 0;
    qint64 seen = view->paintedPixels();
    for (int turns = 0; quiet < 4 && turns < 400; ++turns) {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::UpdateRequest);
        QCoreApplication::processEvents();
        QTest::qWait(5);
        quiet = view->paintedPixels() == seen ? quiet + 1 : 0;
        seen = view->paintedPixels();
    }
}

} // namespace

class CrunchViewEditTests : public QObject {
    Q_OBJECT

    struct Edit {
        const char *name;
        bool rows, insert;
        int at, count;
        int newSize = -1;   // the size the lines put in have, when it is not the default (columns put in inside a run of hidden ones are hidden too: the run grows to hold them)
    };
    static crunch::EditSession::Outcome run(EditWindow &e, const Edit &d)
    {
        crunch::EditSession *s = e.win.w.session();
        if (d.rows) return d.insert ? s->insertRows(0, quint32(d.at), quint32(d.count)) : s->deleteRows(0, quint32(d.at), quint32(d.count));
        return d.insert ? s->insertColumns(0, quint32(d.at), quint32(d.count)) : s->deleteColumns(0, quint32(d.at), quint32(d.count));
    }
    static int whereTop(EditWindow &e, bool rows)
    {
        const grid::GridLayout &L = e.view()->sheet().layout();
        const grid::Axis &a = rows ? L.rows : L.cols;
        return int(a.indexAt(a.position(rows ? L.frozenRows() : L.frozenCols()) + (rows ? L.scrollY() : L.scrollX())));
    }
    // How far into the line at the top (left) of the part that scrolls the view is.
    static qint64 intoTop(EditWindow &e, bool rows)
    {
        const grid::GridLayout &L = e.view()->sheet().layout();
        const grid::Axis &a = rows ? L.rows : L.cols;
        const qint64 origin = a.position(rows ? L.frozenRows() : L.frozenCols());
        return origin + (rows ? L.scrollY() : L.scrollX()) - a.position(whereTop(e, rows));
    }
    static void scrollToLine(EditWindow &e, bool rows, int line)
    {
        grid::GridView::ViewState v = e.view()->viewState();
        const grid::GridLayout &L = e.view()->sheet().layout();
        const grid::Axis &a = rows ? L.rows : L.cols;
        const qint64 at = a.position(line) - a.position(rows ? L.frozenRows() : L.frozenCols());
        (rows ? v.scrollY : v.scrollX) = at;
        e.view()->setViewState(v);
    }

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

    // Rows and columns put in and taken out, at the top, in the middle, inside a run of hidden ones, and past the cells: every height and width goes with its line, the hidden ones stay hidden,
    // and the sizes are those the sheet would be shown with again. Undo and Redo of each give the same.
    void everyHeightWidthAndHiddenLineGoWithItsLine()
    {
        EditWindow e;
        QVERIFY(e.open(viewBook()));
        const grid::GridLayout &L = e.view()->sheet().layout();
        const int defaultRow = L.rows.defaultSize(), defaultCol = L.cols.defaultSize();
        const Edit edits[] = {
            {"insert row at the top", true, true, 0, 1},   {"insert two rows inside the hidden run", true, true, 12, 2}, {"insert rows below the tall ones", true, true, 150, 3},
            {"delete the tall row", true, false, 2, 2},    {"delete across tall and hidden rows", true, false, 9, 6},     {"delete rows far down", true, false, 250, 5},
            {"insert column at the left", false, true, 0, 1}, {"insert columns inside the hidden run", false, true, 4, 2, 0}, {"insert a column past the wide ones", false, true, 20, 2},
            {"delete the wide column", false, false, 5, 1},  {"delete across hidden and wide columns", false, false, 2, 6},  {"delete the first column", false, false, 0, 1},
        };
        for (const Edit &d : edits) {
            const Sizes before = sizesOf(e);
            QVERIFY2(run(e, d).ok, d.name);
            const Sizes after = sizesOf(e);
            // by hand: the lines move, the new ones are the default size, the ones at the end of the sheet are the default size
            Sizes want = before;
            std::vector<int> &v = d.rows ? want.rows : want.cols;
            const size_t total = v.size();
            if (d.insert) {
                v.insert(v.begin() + d.at, size_t(d.count), d.newSize >= 0 ? d.newSize : d.rows ? defaultRow : defaultCol);
                v.resize(total);
            } else {
                v.erase(v.begin() + d.at, v.begin() + d.at + d.count);
                v.insert(v.end(), size_t(d.count), d.rows ? defaultRow : defaultCol);
            }
            QVERIFY2(after == want, d.name);
            QString why;
            QVERIFY2(sameAsShownAgain(e, &why), qPrintable(QStringLiteral("%1: %2").arg(QLatin1String(d.name), why)));
            e.win.w.undoStack()->undo();
            QVERIFY2(sizesOf(e) == before, qPrintable(QStringLiteral("%1, undone").arg(QLatin1String(d.name))));
            QVERIFY2(sameAsShownAgain(e, &why), qPrintable(QStringLiteral("%1, undone: %2").arg(QLatin1String(d.name), why)));
            e.win.w.undoStack()->redo();
            QVERIFY2(sizesOf(e) == after, qPrintable(QStringLiteral("%1, redone").arg(QLatin1String(d.name))));
            QVERIFY2(sameAsShownAgain(e, &why), qPrintable(QStringLiteral("%1, redone: %2").arg(QLatin1String(d.name), why)));
            e.win.w.undoStack()->undo();   // (back to the sheet as it was, for the next)
            QVERIFY(sizesOf(e) == before);
        }
    }

    // The view is not made again: no showing of the sheet, no sum of an axis worked out from its first place, and, in a sheet where every row has a height of its own, none of those heights put in
    // a layout again.
    void anEditOfRowsOrColumnsMakesNoneOfTheViewAgain()
    {
        EditWindow e;
        QVERIFY(e.open(viewBook(3000, true)));
        QVERIFY(e.settle());
        const Edit edits[] = {{"insert row", true, true, 40, 2}, {"delete row", true, false, 2000, 3}, {"insert column", false, true, 1, 1}, {"delete column", false, false, 6, 1}};
        for (const Edit &d : edits) {
            const int shows = e.win.w.sheetShows(), inPlace = e.win.w.inPlaceUpdates();
            const qint64 builds = grid::Axis::buildsForTest(), cells = grid::GridSheet::viewCellsRebuiltForTest();
            QVERIFY2(run(e, d).ok, d.name);
            QVERIFY2(e.win.w.sheetShows() == shows, d.name);
            QVERIFY2(e.win.w.inPlaceUpdates() == inPlace + 1, d.name);
            e.view()->sheet().layout().rows.total();
            e.view()->sheet().layout().cols.total();
            QVERIFY2(grid::Axis::buildsForTest() == builds, qPrintable(QStringLiteral("%1: %2 axes built again").arg(QLatin1String(d.name)).arg(grid::Axis::buildsForTest() - builds)));
            QVERIFY2(grid::GridSheet::viewCellsRebuiltForTest() - cells < 500, qPrintable(QStringLiteral("%1: %2 cells of the view made again").arg(QLatin1String(d.name)).arg(grid::GridSheet::viewCellsRebuiltForTest() - cells)));
            e.win.w.undoStack()->undo();
            QVERIFY2(e.win.w.sheetShows() == shows, qPrintable(QStringLiteral("%1, undone").arg(QLatin1String(d.name))));
            QVERIFY2(grid::Axis::buildsForTest() == builds, qPrintable(QStringLiteral("%1, undone: %2 axes built again").arg(QLatin1String(d.name)).arg(grid::Axis::buildsForTest() - builds)));
            e.win.w.undoStack()->redo();
            QVERIFY2(e.win.w.sheetShows() == shows, qPrintable(QStringLiteral("%1, redone").arg(QLatin1String(d.name))));
            e.win.w.undoStack()->undo();
        }
        QVERIFY(e.settle());
    }

    // Frozen panes stay as many rows and columns as they were, and the sizes in them go with their lines.
    void frozenPanesStayWhereTheyWere()
    {
        EditWindow e;
        QVERIFY(e.open(viewBook()));
        e.view()->setFrozen(4, 2);
        const int inPlace = e.win.w.inPlaceUpdates();
        const Edit edits[] = {{"insert a row in the frozen rows", true, true, 1, 1}, {"delete a frozen row", true, false, 2, 1}, {"insert a row below them", true, true, 30, 2},
                              {"insert a column in the frozen columns", false, true, 0, 1}, {"delete a frozen column", false, false, 1, 1}, {"delete columns below the frozen ones", false, false, 5, 2}};
        for (const Edit &d : edits) {
            QVERIFY2(run(e, d).ok, d.name);
            QCOMPARE(e.view()->sheet().layout().frozenRows(), qint64(4));
            QCOMPARE(e.view()->sheet().layout().frozenCols(), qint64(2));
            QString why;
            QVERIFY2(sameAsShownAgain(e, &why), qPrintable(QStringLiteral("%1: %2").arg(QLatin1String(d.name), why)));
            e.win.w.undoStack()->undo();
            QCOMPARE(e.view()->sheet().layout().frozenRows(), qint64(4));
            QCOMPARE(e.view()->sheet().layout().frozenCols(), qint64(2));
            QVERIFY2(sameAsShownAgain(e, &why), qPrintable(QStringLiteral("%1, undone: %2").arg(QLatin1String(d.name), why)));
            e.win.w.undoStack()->redo();
            QCOMPARE(e.view()->sheet().layout().frozenRows(), qint64(4));
            QCOMPARE(e.view()->sheet().layout().frozenCols(), qint64(2));
            e.win.w.undoStack()->undo();
        }
        QVERIFY(e.win.w.inPlaceUpdates() >= inPlace + int(std::size(edits)));
    }

    // The selection goes with the cells it is on: lines put in or taken out above it move it; lines put in at it are selected; lines taken out of it leave it where the lines after them come; and
    // Undo and Redo give it back.
    void theSelectionFollowsTheCellsItIsOn()
    {
        struct Case {
            const char *name;
            bool rows;
            int r1, c1, r2, c2;   // the selection, as rows and columns from 0
            bool insert;
            int at, count;
            int wantR1, wantC1, wantR2, wantC2;
            bool undoGivesItBack;
        };
        const Case cases[] = {
            {"rows put in above", true, 9, 3, 9, 3, true, 3, 2, 11, 3, 11, 3, true},
            {"rows put in at the cell", true, 9, 3, 9, 3, true, 9, 1, 9, 3, 9, 3, true},
            {"rows put in inside the cells selected", true, 1, 3, 4, 4, true, 3, 2, 1, 3, 6, 4, true},
            {"rows put in below", true, 9, 3, 9, 3, true, 50, 2, 9, 3, 9, 3, true},
            {"rows taken out above", true, 9, 3, 9, 3, false, 3, 2, 7, 3, 7, 3, true},
            {"rows taken out below", true, 9, 3, 9, 3, false, 50, 2, 9, 3, 9, 3, true},
            {"rows taken out that hold the cells selected", true, 9, 3, 11, 4, false, 8, 6, 9, 3, 11, 4, true},
            {"rows taken out of the top of the cells selected", true, 9, 3, 20, 4, false, 4, 10, 4, 3, 10, 4, false},
            {"columns put in left", false, 9, 8, 9, 9, true, 1, 2, 9, 10, 9, 11, true},
            {"columns put in at the cell", false, 9, 8, 9, 8, true, 8, 1, 9, 8, 9, 8, true},
            {"columns taken out left", false, 9, 8, 9, 9, false, 2, 3, 9, 5, 9, 6, true},
            {"columns taken out that hold the cells selected", false, 9, 8, 9, 9, false, 7, 5, 9, 8, 9, 9, true},
            {"columns put in right", false, 9, 1, 9, 2, true, 6, 2, 9, 1, 9, 2, true},
        };
        for (const Case &c : cases) {
            EditWindow e;
            QVERIFY(e.open(viewBook()));
            e.view()->selectRange(c.r1, c.c1, c.r2, c.c2);
            const grid::Selection before = e.view()->selection();
            QVERIFY2(run(e, {c.name, c.rows, c.insert, c.at, c.count}).ok, c.name);
            const grid::Selection now = e.view()->selection();
            const QString text = QStringLiteral("%1: the selection is %2:%3 to %4:%5").arg(QLatin1String(c.name)).arg(now.range.row1).arg(now.range.col1).arg(now.range.row2).arg(now.range.col2);
            QVERIFY2(now.range.row1 == c.wantR1 && now.range.col1 == c.wantC1 && now.range.row2 == c.wantR2 && now.range.col2 == c.wantC2, qPrintable(text));
            QVERIFY2(now.activeRow >= now.range.row1 && now.activeRow <= now.range.row2 && now.activeCol >= now.range.col1 && now.activeCol <= now.range.col2, c.name);
            e.win.w.undoStack()->undo();
            if (c.undoGivesItBack) QVERIFY2(e.view()->selection() == before, qPrintable(QStringLiteral("%1, undone").arg(QLatin1String(c.name))));
            e.win.w.undoStack()->redo();
            QVERIFY2(e.view()->selection() == now, qPrintable(QStringLiteral("%1, redone").arg(QLatin1String(c.name))));
        }
    }

    // Whole rows or columns selected stay whole.
    void wholeRowsAndColumnsSelectedStayWhole()
    {
        EditWindow e;
        QVERIFY(e.open(viewBook()));
        e.view()->selectRows(5, 6);
        QVERIFY(run(e, {"insert at the rows", true, true, 5, 2}).ok);
        QCOMPARE(int(e.view()->selection().whole), int(grid::Selection::Whole::Rows));
        QCOMPARE(e.view()->selection().range.row1, qint64(5));
        QCOMPARE(e.view()->selection().range.row2, qint64(6));
        QCOMPARE(e.view()->selection().range.col2, qint64(16383));
        e.view()->selectColumns(2, 3);
        QVERIFY(run(e, {"delete the columns", false, false, 2, 2}).ok);
        QCOMPARE(int(e.view()->selection().whole), int(grid::Selection::Whole::Columns));
        QCOMPARE(e.view()->selection().range.col1, qint64(2));
        QCOMPARE(e.view()->selection().range.col2, qint64(3));
        QCOMPARE(e.view()->selection().range.row2, qint64(1048575));
        e.view()->selectColumns(2, 3);
        QVERIFY(run(e, {"insert rows with columns selected", true, true, 40, 3}).ok);
        QCOMPARE(e.view()->selection().range.row1, qint64(0));
        QCOMPARE(e.view()->selection().range.row2, qint64(1048575));
    }

    // The part that scrolls stays at the row (column) it began at, however the sizes above it changed, for an edit above it; an edit inside or below it leaves the scroll where it was. Undo and Redo too.
    void theScrollStaysAtTheRowAndColumnAtTheTop()
    {
        EditWindow e;
        QVERIFY(e.open(viewBook()));
        // (row 100 is 45 points high, and the view begins at row 101; the wide column H, 30 wide, is the eighth and the view begins at the ninth)
        scrollToLine(e, true, 100);
        scrollToLine(e, false, 8);
        QCOMPARE(whereTop(e, true), 100);
        QCOMPARE(whereTop(e, false), 8);
        QCOMPARE(intoTop(e, true), qint64(0));
        // a row put in above: the tall row 100 moves down, and the view still begins at row 100 (index), whole
        QVERIFY(run(e, {"insert above", true, true, 0, 1}).ok);
        QCOMPARE(whereTop(e, true), 100);
        QCOMPARE(intoTop(e, true), qint64(0));
        e.win.w.undoStack()->undo();
        QCOMPARE(whereTop(e, true), 100);
        QCOMPARE(intoTop(e, true), qint64(0));
        e.win.w.undoStack()->redo();
        QCOMPARE(whereTop(e, true), 100);
        QCOMPARE(intoTop(e, true), qint64(0));
        e.win.w.undoStack()->undo();
        // taken out above
        QVERIFY(run(e, {"delete above", true, false, 2, 3}).ok);
        QCOMPARE(whereTop(e, true), 100);
        QCOMPARE(intoTop(e, true), qint64(0));
        e.win.w.undoStack()->undo();
        // below the top: nothing about the scroll changes
        const qint64 y = e.view()->sheet().layout().scrollY();
        QVERIFY(run(e, {"insert below", true, true, 120, 4}).ok);
        QCOMPARE(e.view()->sheet().layout().scrollY(), y);
        e.win.w.undoStack()->undo();
        QCOMPARE(e.view()->sheet().layout().scrollY(), y);
        // columns: a column put in at the left moves the wide column (the eighth) to the ninth, and the view still begins at the ninth column, whole
        QVERIFY(run(e, {"insert left", false, true, 0, 1}).ok);
        QCOMPARE(whereTop(e, false), 8);
        QCOMPARE(intoTop(e, false), qint64(0));
        e.win.w.undoStack()->undo();
        QCOMPARE(whereTop(e, false), 8);
        QCOMPARE(intoTop(e, false), qint64(0));
        QVERIFY(run(e, {"delete left", false, false, 5, 2}).ok);
        QCOMPARE(whereTop(e, false), 8);
        QCOMPARE(intoTop(e, false), qint64(0));
    }

    // What is drawn again: an edit inside the window draws from the line before the first changed to the end, one below the window draws nothing, and one above the part in view draws the whole window.
    void anEditDrawsOnlyTheBandItChanged()
    {
        EditWindow e;
        QVERIFY(e.open(viewBook()));
        QVERIFY(e.settle());
        scrollToLine(e, true, 100);
        scrollToLine(e, false, 0);
        e.view()->selectCell(100, 0);
        flushPaints(e.view());
        const QSize vp = e.view()->viewport()->size();
        const qint64 whole = qint64(vp.width()) * vp.height();
        // (what an edit draws: when it is to draw something, the paint is waited for; when it is to draw nothing, the turns of the loop that follow show it)
        auto drawn = [&](const std::function<void()> &edit, bool something) {
            flushPaints(e.view());
            const qint64 before = e.view()->paintedPixels();
            edit();
            if (something) QTest::qWaitFor([&] { return e.view()->paintedPixels() > before; }, 10000);
            flushPaints(e.view());
            return e.view()->paintedPixels() - before;
        };
        // a row put in 12 rows down the window
        const grid::GridLayout &L = e.view()->sheet().layout();
        const int midRow = 112;
        const int bandTop = L.yOf(midRow - 1);
        qint64 px = drawn([&] { QVERIFY(run(e, {"insert in the window", true, true, midRow, 1}).ok); }, true);
        QVERIFY2(px > 0 && px < whole * 3 / 4, qPrintable(QStringLiteral("drew %1 of %2 pixels").arg(px).arg(whole)));
        QVERIFY2(e.view()->lastPaintRect().top() >= bandTop - 1 && e.view()->lastPaintRect().top() > vp.height() / 4, qPrintable(QStringLiteral("drew from %1, band from %2").arg(e.view()->lastPaintRect().top()).arg(bandTop)));
        QCOMPARE(e.view()->lastPaintRect().bottom() + 1, vp.height());
        px = drawn([&] { e.win.w.undoStack()->undo(); }, true);
        QVERIFY2(px > 0 && px < whole * 3 / 4, qPrintable(QStringLiteral("undone, drew %1 of %2 pixels").arg(px).arg(whole)));
        // a row put in far below the window
        px = drawn([&] { QVERIFY(run(e, {"insert below the window", true, true, 250, 1}).ok); }, false);
        QCOMPARE(px, qint64(0));
        px = drawn([&] { e.win.w.undoStack()->undo(); }, false);
        QCOMPARE(px, qint64(0));
        // a row put in above the part in view: all of it moves
        px = drawn([&] { QVERIFY(run(e, {"insert above the window", true, true, 5, 1}).ok); }, true);
        QVERIFY2(px >= whole * 3 / 4, qPrintable(QStringLiteral("above: drew %1 of %2 pixels").arg(px).arg(whole)));
        e.win.w.undoStack()->undo();
        // a column put in a few columns into the window
        scrollToLine(e, true, 0);
        const int bandLeft = L.xOf(5);
        px = drawn([&] { QVERIFY(run(e, {"insert a column", false, true, 6, 1}).ok); }, true);
        QVERIFY2(px > 0 && px < whole * 9 / 10, qPrintable(QStringLiteral("column: drew %1 of %2 pixels").arg(px).arg(whole)));
        QVERIFY2(e.view()->lastPaintRect().left() >= bandLeft - 1 && e.view()->lastPaintRect().left() > vp.width() / 5, qPrintable(QStringLiteral("drew from %1, band from %2").arg(e.view()->lastPaintRect().left()).arg(bandLeft)));
        QCOMPARE(e.view()->lastPaintRect().right() + 1, vp.width());
    }
};

JP_TEST_CLASS(CrunchViewEditTests)
#include "tst_crunch_viewedit.moc"
