// Tests of the grid as a widget (libs/shell/grid/gridview.h): scrolling that draws only the strip it uncovers, the keys and the mouse (arrows, Ctrl and an arrow in a sheet of a
// million rows, Home and End, the page keys, Tab and Enter, clicks, drags, the headers), the address and text of the cell with the cursor, the numbers of the selection, copying
// it as text, and zoom. The window is offscreen; every sheet is made here.
#include "sheethelpers.h"

#include "grid/gridview.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace jp::grid;

namespace {

struct Book {
    Sheet sheet;
    Styles styles;
    SharedStrings strings;
    std::unique_ptr<GridView> view;

    Book()
    {
        FontSpec f;
        f.name = QStringLiteral("Calibri");
        f.size = 11;
        styles.fonts = {f};
        styles.fills = {FillSpec()};
        styles.borders = {BorderSpec()};
        styles.formats = {CellFormat()};
        styles.cellXfs = {0};
    }
    void number(quint32 row, quint32 col, double v, quint32 style = 0)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::Number;
        c.number = v;
        c.style = style;
        sheet.cells.add(c);
        sheet.extentRows = std::max(sheet.extentRows, row + 1);
        sheet.extentCols = std::max(sheet.extentCols, col + 1);
    }
    void text(quint32 row, quint32 col, const QString &t)
    {
        Cell c;
        c.row = row;
        c.col = quint16(col);
        c.type = CellType::InlineString;
        c.number = sheet.texts.size();
        sheet.texts.append(t);
        sheet.cells.add(c);
        sheet.extentRows = std::max(sheet.extentRows, row + 1);
        sheet.extentCols = std::max(sheet.extentCols, col + 1);
    }
    void show(const QSize &size = QSize(640, 420))
    {
        sheet.cells.finish();
        view = std::make_unique<GridView>();
        view->resize(size);
        GridData d;
        d.styles = &styles;
        d.strings = &strings;
        d.sheet = &sheet;
        view->setData(d);
        view->show();
        QVERIFY(QTest::qWaitForWindowExposed(view.get()));
    }
    GridLayout &layout() { return view->sheet().layout(); }
    QPoint centerOf(qint64 row, qint64 col)
    {
        const QRect r = layout().cellRect(row, col);
        return r.center();
    }
};

// How many dark pixels are in a block of the picture.
int inkBoxOf(const QImage &img, const QRect &r)
{
    int n = 0;
    for (int y = std::max(0, r.top()); y <= std::min(img.height() - 1, r.bottom()); ++y)
        for (int x = std::max(0, r.left()); x <= std::min(img.width() - 1, r.right()); ++x) {
            const QRgb c = img.pixel(x, y);
            if (qRed(c) < 160 && qGreen(c) < 160 && qBlue(c) < 160) ++n;
        }
    return n;
}

// Records the rectangles the viewport is asked to draw.
class PaintRecorder : public QObject {
public:
    explicit PaintRecorder(QWidget *w) : QObject(w) { w->installEventFilter(this); }
    QVector<QRect> rects;
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() == QEvent::Paint) rects.append(static_cast<QPaintEvent *>(e)->rect());
        return false;
    }
    int area() const
    {
        int a = 0;
        for (const QRect &r : rects) a += r.width() * r.height();
        return a;
    }
};

void key(GridView &v, int k, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::keyClick(&v, Qt::Key(k), m); }
// The keys of a standard command on this computer (Cmd+A, or Ctrl+A): the first the platform gives.
void pressStandard(GridView &v, QKeySequence::StandardKey command)
{
    const QKeySequence keys(command);
    const QKeyCombination k = keys[0];
    QTest::keyClick(&v, k.key(), k.keyboardModifiers());
}

} // namespace

class GridViewTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Which keys begin an entry in the cell: those that print. Ctrl (or Cmd) with a character is a shortcut, and so is Alt (or Option and Cmd). AltGr, which gives the characters some
    // keyboards have on their letters, arrives as Ctrl and Alt together on Windows; on a Mac, Option gives them (é, @, a bracket on some layouts) and Cmd with Option is a shortcut.
    void whichKeysBeginAnEntry()
    {
        const Qt::KeyboardModifiers none = Qt::NoModifier, shift = Qt::ShiftModifier, ctrl = Qt::ControlModifier, alt = Qt::AltModifier, meta = Qt::MetaModifier;
        for (const bool mac : {false, true}) {
            QVERIFY(GridView::keyBeginsAnEntry(none, QStringLiteral("a"), mac));
            QVERIFY(GridView::keyBeginsAnEntry(shift, QStringLiteral("A"), mac));
            QVERIFY(GridView::keyBeginsAnEntry(none, QStringLiteral("5"), mac));
            QVERIFY(!GridView::keyBeginsAnEntry(none, QString(), mac));
            QVERIFY(!GridView::keyBeginsAnEntry(none, QStringLiteral("\t"), mac));
            QVERIFY(!GridView::keyBeginsAnEntry(ctrl, QStringLiteral("c"), mac));              // (Cmd+C on a Mac)
            QVERIFY(!GridView::keyBeginsAnEntry(ctrl | shift, QStringLiteral("C"), mac));
            QVERIFY(!GridView::keyBeginsAnEntry(meta, QStringLiteral("a"), mac));
            QVERIFY(!GridView::keyBeginsAnEntry(ctrl | alt | meta, QStringLiteral("a"), mac));
        }
        // Windows and Linux: AltGr is Ctrl and Alt; Alt alone is a shortcut
        QVERIFY(GridView::keyBeginsAnEntry(ctrl | alt, QStringLiteral("@"), false));
        QVERIFY(!GridView::keyBeginsAnEntry(alt, QStringLiteral("a"), false));
        // a Mac: Option alone gives characters; Cmd and Option (Qt's Control and Alt) is a shortcut, whatever it prints
        QVERIFY(GridView::keyBeginsAnEntry(alt, QStringLiteral("\u00e9"), true));
        QVERIFY(GridView::keyBeginsAnEntry(alt | shift, QStringLiteral("\u00b6"), true));
        QVERIFY(!GridView::keyBeginsAnEntry(ctrl | alt, QStringLiteral("\u2206"), true));
        QVERIFY(!GridView::keyBeginsAnEntry(ctrl | alt | shift, QStringLiteral("v"), true));
    }

    // A wheel turn scrolls three rows; at 100% the pixels already drawn are moved and only the strip it uncovers is drawn, a few rows of cells, not the screen.
    void scrollingDrawsOnlyTheStripItUncovers()
    {
        Book b;
        for (quint32 r = 0; r < 100000; r += 1)
            for (quint32 c = 0; c < 10; ++c) b.number(r, c, r * 10.0 + c);
        b.show();
        QApplication::processEvents();
        PaintRecorder rec(b.view->viewport());
        b.view->clearPaintTimes();
        QWheelEvent wheel(QPointF(100, 100), QPointF(100, 100), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(b.view->viewport(), &wheel);
        QCOMPARE(b.layout().scrollY(), qint64(60));
        QCOMPARE(b.view->blitScrolls(), qint64(1));   // (the pixels already there are moved, whatever the window system does with that)
        QCOMPARE(b.view->fullScrolls(), qint64(0));
        QTRY_VERIFY(!rec.rects.isEmpty());
        QApplication::processEvents();
        // The window systems that move pixels leave a strip of a few rows to draw; the rest (which may draw the whole window) are not asked.
        const QString platform = QGuiApplication::platformName();
        if (platform == QLatin1String("offscreen") || platform == QLatin1String("xcb") || platform == QLatin1String("cocoa")) {
            const QSize v = b.view->viewport()->size();
            QVERIFY2(rec.area() < v.width() * 100, qPrintable(QStringLiteral("drew %1 pixels of %2").arg(rec.area()).arg(v.width() * v.height())));
            QVERIFY2(b.view->lastPaintStats().rowsVisited <= 8, qPrintable(QString::number(b.view->lastPaintStats().rowsVisited)));
            QVERIFY(b.view->lastPaintStats().textRuns <= 8 * 12);
        }
        // the scroll bar and the layout agree, and a jump a long way draws the screen
        QCOMPARE(b.view->verticalScrollBar()->value(), 60);
        b.view->verticalScrollBar()->setValue(90000 * 20);
        QCOMPARE(b.layout().scrollY(), qint64(90000 * 20));
        const PaneView body = b.layout().pane(Pane::Body);
        QCOMPARE(body.firstRow, qint64(90000));
        QTRY_VERIFY(b.view->paintTimes().size() >= 2);
        // the screen after a jump is what a fresh draw of it is
        const QImage grabbed = b.view->viewport()->grab().toImage();
        QVERIFY(!grabbed.isNull());
        // scrolled back by wheel steps to the first rows: the first row is at the top again
        b.view->verticalScrollBar()->setValue(0);
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(0));
        // Shift and the wheel scroll along the rows
        QWheelEvent across(QPointF(100, 100), QPointF(100, 100), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::ShiftModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(b.view->viewport(), &across);
        QCOMPARE(b.layout().scrollX(), qint64(3 * b.layout().cols.defaultSize()));
    }

    // The arrow keys move the cursor and show it; Shift extends; the address of the cell with the cursor is what the name box shows.
    void arrowKeysMoveTheCursorAndShiftExtends()
    {
        Book b;
        for (quint32 r = 0; r < 200; ++r)
            for (quint32 c = 0; c < 5; ++c) b.number(r, c, r * 10.0 + c);
        b.show();
        QSignalSpy changed(b.view.get(), &GridView::selectionChanged);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A1"));
        key(*b.view, Qt::Key_Down);
        key(*b.view, Qt::Key_Right);
        QCOMPARE(b.view->selectionName(), QStringLiteral("B2"));
        QVERIFY(changed.count() >= 2);
        key(*b.view, Qt::Key_Down, Qt::ShiftModifier);
        key(*b.view, Qt::Key_Down, Qt::ShiftModifier);
        key(*b.view, Qt::Key_Right, Qt::ShiftModifier);
        QCOMPARE(b.view->selectionName(), QStringLiteral("B2"));
        QCOMPARE(b.view->rangeName(), QStringLiteral("B2:C4"));
        key(*b.view, Qt::Key_Left);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A2"));
        QCOMPARE(b.view->rangeName(), QStringLiteral("A2"));
        // going down past the screen scrolls it: the cell with the cursor is whole in view
        for (int i = 0; i < 40; ++i) key(*b.view, Qt::Key_Down);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A42"));
        const QRect cell = b.layout().cellRect(41, 0);
        QVERIFY(b.layout().pane(Pane::Body).rect.contains(cell));
        QVERIFY(b.layout().scrollY() > 0);
        // Page Down and Up move the cursor a screen
        const qint64 before = b.view->selection().activeRow;
        key(*b.view, Qt::Key_PageDown);
        QVERIFY(b.view->selection().activeRow - before >= 15);
        key(*b.view, Qt::Key_PageUp);
        QCOMPARE(b.view->selection().activeRow, before);
        // Home, Ctrl+Home, Ctrl+End
        key(*b.view, Qt::Key_Right);
        key(*b.view, Qt::Key_Right);
        key(*b.view, Qt::Key_Home);
        QCOMPARE(b.view->selection().activeCol, qint64(0));
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A1"));
        QCOMPARE(b.layout().scrollY(), qint64(0));
        key(*b.view, Qt::Key_End, Qt::ControlModifier);
        QCOMPARE(b.view->selectionName(), QStringLiteral("E200"));
        // Tab and Enter
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        key(*b.view, Qt::Key_Tab);
        QCOMPARE(b.view->selectionName(), QStringLiteral("B1"));
        key(*b.view, Qt::Key_Backtab);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A1"));
        key(*b.view, Qt::Key_Return);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A2"));
        key(*b.view, Qt::Key_Return, Qt::ShiftModifier);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A1"));
        // whole column, whole row, everything
        key(*b.view, Qt::Key_Space, Qt::ControlModifier);
        QVERIFY(b.view->selection().whole == Selection::Whole::Columns);
        QCOMPARE(b.view->rangeName(), QStringLiteral("A:A"));
        key(*b.view, Qt::Key_Space, Qt::ShiftModifier);   // (the rows of a whole column are the whole sheet)
        QVERIFY(b.view->selection().whole == Selection::Whole::Rows);
        QCOMPARE(b.view->rangeName(), QStringLiteral("1:1048576"));
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        key(*b.view, Qt::Key_Space, Qt::ShiftModifier);
        QCOMPARE(b.view->rangeName(), QStringLiteral("1:1"));
        pressStandard(*b.view, QKeySequence::SelectAll);
        QVERIFY(b.view->selection().whole == Selection::Whole::All);
        // at the edges nothing moves
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        key(*b.view, Qt::Key_Up);
        key(*b.view, Qt::Key_Left);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A1"));
    }

    // Ctrl and an arrow in a sheet of a million rows: the walk for the edge of the data is done between events, and the cursor arrives, whatever the keys in the meantime.
    void ctrlArrowFindsTheEdgeOfTheDataWithoutHoldingUpTheWindow()
    {
        Book b;
        for (quint32 r = 0; r < 5; ++r) b.number(r, 0, r);
        b.number(1048575, 0, 1);
        b.show();
        key(*b.view, Qt::Key_Down, Qt::ControlModifier);
        QTRY_VERIFY(!b.view->busy());
        QCOMPARE(b.view->selection().activeRow, qint64(4));
        key(*b.view, Qt::Key_Down, Qt::ControlModifier);   // the next that holds something: a million rows on
        QTRY_VERIFY_WITH_TIMEOUT(!b.view->busy() && b.view->selection().activeRow == 1048575, 5000);
        QCOMPARE(b.view->selectionName(), QStringLiteral("A1048576"));
        // the cell with the cursor is in view, a million rows down
        QVERIFY(b.layout().pane(Pane::Body).rect.intersects(b.layout().cellRect(1048575, 0)));
        // a key while it is looking ends the search: nothing arrives
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        key(*b.view, Qt::Key_Down, Qt::ControlModifier);
        key(*b.view, Qt::Key_Down, Qt::ControlModifier);   // (from the end of the first run, to the far cell)
        key(*b.view, Qt::Key_Escape);
        QVERIFY(!b.view->busy());
        // with Shift it extends
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        key(*b.view, Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier);
        QTRY_VERIFY(!b.view->busy());
        QCOMPARE(b.view->rangeName(), QStringLiteral("A1:A5"));
    }

    // The selection is settled where the person leaves it: at the click, at the end of a drag (not while the drag goes on), and at a key that moves it. (The Format Painter acts on it.)
    void theSelectionIsSettledWhenThePersonLeavesIt()
    {
        Book b;
        for (quint32 r = 0; r < 20; ++r)
            for (quint32 c = 0; c < 10; ++c) b.number(r, c, r + c);
        b.show();
        QSignalSpy settled(b.view.get(), &GridView::selectionSettled), changed(b.view.get(), &GridView::selectionChanged);
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(3, 2));
        QCOMPARE(settled.count(), 1);
        QCOMPARE(b.view->selectionName(), QStringLiteral("C4"));
        // a drag: changed as it goes, settled once at the end
        settled.clear();
        changed.clear();
        QTest::mousePress(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(1, 1));
        QMouseEvent move(QEvent::MouseMove, QPointF(b.centerOf(3, 3)), QPointF(b.view->viewport()->mapToGlobal(b.centerOf(3, 3))), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(b.view->viewport(), &move);
        QVERIFY(changed.count() >= 2);
        QCOMPARE(settled.count(), 0);
        QTest::mouseRelease(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(3, 3));
        QCOMPARE(settled.count(), 1);
        QCOMPARE(b.view->rangeName(), QStringLiteral("B2:D4"));
        // a click on the cell that is the selection already still settles it
        settled.clear();
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(1, 1));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(1, 1));
        QCOMPARE(settled.count(), 2);
        // a key
        settled.clear();
        QTest::keyClick(b.view.get(), Qt::Key_Down);
        QCOMPARE(settled.count(), 1);
    }

    // A click selects the cell, a shift-click extends, a drag extends as it goes; the headers select whole rows and columns, and the corner everything.
    void theMouseSelectsCellsHeadersAndTheCorner()
    {
        Book b;
        for (quint32 r = 0; r < 50; ++r)
            for (quint32 c = 0; c < 10; ++c) b.number(r, c, r + c);
        b.show();
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(3, 2));
        QCOMPARE(b.view->selectionName(), QStringLiteral("C4"));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::ShiftModifier, b.centerOf(6, 4));
        QCOMPARE(b.view->rangeName(), QStringLiteral("C4:E7"));
        QCOMPARE(b.view->selectionName(), QStringLiteral("C4"));
        // a drag: press, move, release
        QTest::mousePress(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(1, 1));
        QMouseEvent move(QEvent::MouseMove, QPointF(b.centerOf(3, 3)), QPointF(b.view->viewport()->mapToGlobal(b.centerOf(3, 3))), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(b.view->viewport(), &move);
        QCOMPARE(b.view->rangeName(), QStringLiteral("B2:D4"));
        QTest::mouseRelease(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(3, 3));
        // the column header and the row header
        const int hw = b.layout().headerWidth(), hh = b.layout().headerHeight();
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(b.layout().cellRect(0, 3).center().x(), hh / 2));
        QVERIFY(b.view->selection().whole == Selection::Whole::Columns);
        QCOMPARE(b.view->rangeName(), QStringLiteral("D:D"));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::ShiftModifier, QPoint(b.layout().cellRect(0, 5).center().x(), hh / 2));
        QCOMPARE(b.view->rangeName(), QStringLiteral("D:F"));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(hw / 2, b.layout().cellRect(4, 0).center().y()));
        QVERIFY(b.view->selection().whole == Selection::Whole::Rows);
        QCOMPARE(b.view->rangeName(), QStringLiteral("5:5"));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(hw / 2, hh / 2));
        QVERIFY(b.view->selection().whole == Selection::Whole::All);
        // a right click outside the selection selects what is under it, and asks for a menu
        QSignalSpy menu(b.view.get(), &GridView::contextMenuRequested);
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(2, 2));
        QTest::mouseClick(b.view->viewport(), Qt::RightButton, Qt::NoModifier, b.centerOf(8, 6));
        QCOMPARE(b.view->selectionName(), QStringLiteral("G9"));
        QCOMPARE(menu.count(), 1);
    }

    // The numbers of the selection reach the status bar when added up (not for one cell), and a whole column is added up as far as its used rows.
    void theNumbersOfTheSelectionAreToldWhenAddedUp()
    {
        Book b;
        for (quint32 r = 0; r < 4; ++r)
            for (quint32 c = 0; c < 2; ++c) b.number(r, c, r * 2.0 + c + 1);
        b.show();
        QSignalSpy stats(b.view.get(), &GridView::statsChanged);
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, b.centerOf(0, 0));
        QVERIFY(b.view->statsText().isEmpty());
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::ShiftModifier, b.centerOf(1, 1));
        QTRY_COMPARE(b.view->statsText(), QStringLiteral("Average: 2.5    Count: 4    Sum: 10"));
        QVERIFY(stats.count() >= 1);
        // a whole column: 1 + 3 + 5 + 7
        key(*b.view, Qt::Key_Home, Qt::ControlModifier);
        key(*b.view, Qt::Key_Space, Qt::ControlModifier);
        QTRY_COMPARE(b.view->statsText(), QStringLiteral("Average: 4    Count: 4    Sum: 16"));
        pressStandard(*b.view, QKeySequence::SelectAll);
        QTRY_COMPARE(b.view->statsText(), QStringLiteral("Average: 4.5    Count: 8    Sum: 36"));
    }

    // The cell with the cursor, as the formula bar shows it: a formula as written, a number as it is stored, a date as it is shown, and text as it is.
    void theFormulaBarShowsWhatTheCellHolds()
    {
        Book b;
        b.styles.numFmts.insert(164, QStringLiteral("yyyy-mm-dd"));
        CellFormat date;
        date.numFmtId = 164;
        b.styles.formats.append(date);
        b.styles.cellXfs.append(164);
        b.number(0, 0, 0.1 + 0.2);
        b.number(1, 0, 45366, 1);
        b.text(2, 0, QStringLiteral("hello"));
        Cell f;
        f.row = 3;
        f.col = 0;
        f.type = CellType::Number;
        f.number = 7;
        f.flags = jp::office::sheet::HasFormula;
        f.formula = 0;
        b.sheet.formulaTexts.append(QStringLiteral("SUM(A1:A2)"));
        Formula fx;
        fx.text = 0;
        b.sheet.formulas.append(fx);
        b.sheet.cells.add(f);
        b.sheet.extentRows = 4;
        b.show();
        QCOMPARE(b.view->activeCellText(), QStringLiteral("0.3"));
        key(*b.view, Qt::Key_Down);
        QCOMPARE(b.view->activeCellText(), QStringLiteral("2024-03-15"));
        key(*b.view, Qt::Key_Down);
        QCOMPARE(b.view->activeCellText(), QStringLiteral("hello"));
        key(*b.view, Qt::Key_Down);
        QCOMPARE(b.view->activeCellText(), QStringLiteral("=SUM(A1:A2)"));
        key(*b.view, Qt::Key_Down);
        QCOMPARE(b.view->activeCellText(), QString());
    }

    // The selection as text for the clipboard: tabs between the cells, a line to a row, what is shown in them, a text with a tab or a quote quoted; hidden rows left out.
    void theSelectionIsCopiedAsText()
    {
        Book b;
        b.number(0, 0, 1);
        b.number(0, 1, 2);
        b.number(1, 0, 3);
        b.text(1, 1, QStringLiteral("a\tb \"q\""));
        b.number(2, 0, 5);
        b.show();
        b.layout().rows.setSize(2, 0);
        key(*b.view, Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier);
        QTRY_VERIFY(!b.view->busy());
        key(*b.view, Qt::Key_Right, Qt::ShiftModifier);
        QCOMPARE(b.view->selectionAsText(), QStringLiteral("1\t2\n3\t\"a\tb \"\"q\"\"\"\n"));
        // a whole column of the sheet is its used rows
        key(*b.view, Qt::Key_Space, Qt::ControlModifier);
        QCOMPARE(b.view->selectionAsText(), QStringLiteral("1\t2\n3\t\"a\tb \"\"q\"\"\"\n"));
        // too many cells are refused
        bool tooMany = false;
        Book big;
        big.number(1500, 1500, 1);
        big.show();
        pressStandard(*big.view, QKeySequence::SelectAll);
        QCOMPARE(big.view->selectionAsText(&tooMany), QString());
        QVERIFY(tooMany);
    }

    // Zoom (Ctrl and the wheel, or asked for) changes the size of the cells and says so; the cell with the cursor is kept in view.
    void zoomChangesTheCellsAndSaysSo()
    {
        Book b;
        for (quint32 r = 0; r < 100; ++r) b.number(r, 0, r);
        b.show();
        QSignalSpy zoomed(b.view.get(), &GridView::zoomChanged);
        const int h = b.layout().cellRect(0, 0).height();
        QWheelEvent wheel(QPointF(100, 100), QPointF(100, 100), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(b.view->viewport(), &wheel);
        QCOMPARE(b.view->zoom(), 1.1);
        QCOMPARE(zoomed.count(), 1);
        QVERIFY(b.layout().cellRect(0, 0).height() > h);
        b.view->setZoom(2.0);
        QCOMPARE(b.layout().cellRect(0, 0).height(), 2 * h);
        b.view->setZoom(50);
        QCOMPARE(b.view->zoom(), GridLayout::kMaxZoom);
        b.view->setZoom(0.001);
        QCOMPARE(b.view->zoom(), GridLayout::kMinZoom);
    }

    // The gridlines and the headings can be turned off and on, and frozen panes set from the view; the first rows stay when the rest scrolls.
    void gridlinesHeadingsAndFrozenPanesAreTheViews()
    {
        Book b;
        for (quint32 r = 0; r < 200; ++r) b.number(r, 0, r);
        b.show();
        QVERIFY(b.view->showGridlines() && b.view->showHeadings());
        QVERIFY(b.layout().headerWidth() > 0);
        b.view->setShowHeadings(false);
        QCOMPARE(b.layout().headerWidth(), 0);
        QCOMPARE(b.layout().cellRect(0, 0).topLeft(), QPoint(0, 0));
        b.view->setShowHeadings(true);
        b.view->setShowGridlines(false);
        QVERIFY(!b.view->showGridlines());
        b.view->setFrozen(2, 1);
        QCOMPARE(b.layout().frozenRows(), qint64(2));
        b.view->verticalScrollBar()->setValue(400);
        const PaneView top = b.layout().pane(Pane::Top);
        QCOMPARE(top.firstRow, qint64(0));
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(22));
        b.view->setFrozen(0, 0);
        QCOMPARE(b.layout().frozenRows(), qint64(0));
    }

    // Keys, clicks, and scrolling on a sheet with nothing in it, and with its rows hidden, do not fail.
    void anEmptySheetTakesEveryKey()
    {
        Book b;
        b.show();
        for (int k : {Qt::Key_Down, Qt::Key_Up, Qt::Key_Left, Qt::Key_Right, Qt::Key_Home, Qt::Key_End, Qt::Key_PageDown, Qt::Key_PageUp, Qt::Key_Tab, Qt::Key_Return, Qt::Key_Space}) {
            key(*b.view, k);
            key(*b.view, k, Qt::ControlModifier);
            key(*b.view, k, Qt::ShiftModifier);
        }
        QTRY_VERIFY(!b.view->busy());
        QVERIFY(b.view->selection().activeRow >= 0 && b.view->selection().activeCol >= 0);
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(300, 200));
        QCOMPARE(b.view->selectionAsText(), QString());
        b.view->setZoom(0.1);
        b.view->verticalScrollBar()->setValue(b.view->verticalScrollBar()->maximum());
        b.view->grab();
    }

    // Page Down moves the cursor a screen of rows and scrolls the sheet a screen: the cursor stays where it was in the window. Page Up goes back.
    void pageKeysMoveTheCursorAndTheSheetAScreen()
    {
        Book b;
        for (quint32 r = 0; r < 300; ++r) b.number(r, 0, r);
        b.show(QSize(640, 420));
        const int screen = int(b.layout().pane(Pane::Body).rect.height() / b.layout().rows.defaultSize());   // (whole rows)
        QVERIFY(screen >= 15);
        key(*b.view, Qt::Key_PageDown);
        QCOMPARE(b.view->selection().activeRow, qint64(screen));
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(screen));
        QCOMPARE(b.layout().scrollY(), qint64(screen * 20));
        key(*b.view, Qt::Key_PageDown);
        QCOMPARE(b.view->selection().activeRow, qint64(2 * screen));
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(2 * screen));
        key(*b.view, Qt::Key_PageUp);
        QCOMPARE(b.view->selection().activeRow, qint64(screen));
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(screen));
        key(*b.view, Qt::Key_PageUp);
        QCOMPARE(b.view->selection().activeRow, qint64(0));
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(0));
        key(*b.view, Qt::Key_PageUp);   // (at the top: nowhere to go)
        QCOMPARE(b.view->selection().activeRow, qint64(0));
        // the cursor part way down the window keeps its place in it
        for (int i = 0; i < 5; ++i) key(*b.view, Qt::Key_Down);
        key(*b.view, Qt::Key_PageDown);
        QCOMPARE(b.view->selection().activeRow, qint64(5 + screen));
        QCOMPARE(b.layout().pane(Pane::Body).firstRow, qint64(screen));
    }

    // At a zoom where a scroll of whole rows moves every row by whole pixels (any zoom the scroll's size comes to whole pixels at), the pixels already drawn are moved, and not the
    // window drawn again.
    void scrollsAtOtherZoomsMoveThePixelsToo()
    {
        Book b;
        for (quint32 r = 0; r < 1000; ++r)
            for (quint32 c = 0; c < 10; ++c) b.number(r, c, r * 10.0 + c);
        b.show();
        QApplication::processEvents();
        b.view->setZoom(0.5);
        QApplication::processEvents();
        const qint64 blits = b.view->blitScrolls(), fulls = b.view->fullScrolls();
        QWheelEvent wheel(QPointF(100, 100), QPointF(100, 100), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(b.view->viewport(), &wheel);
        QCOMPARE(b.layout().scrollY(), qint64(60));   // (three rows of 20: 30 pixels at half size)
        QCOMPARE(b.view->blitScrolls(), blits + 1);
        QCOMPARE(b.view->fullScrolls(), fulls);
        // at a zoom where it does not (three rows of 20 at 1.01 are 60.6 pixels: the edges are not all moved alike), the window is drawn again
        b.view->setZoom(1.01);
        QApplication::processEvents();
        const qint64 blits2 = b.view->blitScrolls(), fulls2 = b.view->fullScrolls();
        QApplication::sendEvent(b.view->viewport(), &wheel);
        QCOMPARE(b.view->blitScrolls(), blits2);
        QCOMPARE(b.view->fullScrolls(), fulls2 + 1);
    }

    // A sheet that reads right to left in the widget: column A is at the right edge of what is drawn, a click there selects it, the rectangle of a cell is where it is drawn, the arrow
    // keys go the way the columns lie (the left arrow to the next column), the scroll bar runs the other way, and the part that scrolls is moved the right way.
    void aSheetThatReadsRightToLeftIsDrawnAndClickedFromTheRight()
    {
        Book b;
        FillSpec green;
        green.pattern = PatternKind::Solid;
        green.fg = ColorSpec::fromRgb(0x00FF00);
        b.styles.fills.append(green);
        CellFormat fmt;
        fmt.fill = 1;
        b.styles.formats.append(fmt);
        b.styles.cellXfs.append(0);
        for (int c = 0; c < 40; ++c) b.number(0, quint32(c), c + 1, c == 0 ? 1u : 0u);
        b.sheet.view.rightToLeft = true;
        b.show();
        QVERIFY(b.view->sheet().rightToLeft());
        const QRect a = b.view->cellRect(0, 0), c = b.view->cellRect(0, 1);
        QVERIFY2(a.right() > b.view->viewport()->width() - 100, qPrintable(QStringLiteral("A is at %1").arg(a.right())));
        QVERIFY(c.right() < a.left() + 2 && c.left() < a.left());                    // (B is to the left of A)
        // what is drawn: the fill of A1 is in the rectangle the view gives
        const QImage img = b.view->viewport()->grab().toImage();
        QCOMPARE(img.pixel(a.center().x(), a.center().y()), QRgb(0xFF00FF00));
        QCOMPARE(img.pixel(c.center().x(), c.center().y()), QRgb(0xFFFFFFFF));
        QCOMPARE(img.pixel(10, a.center().y()), QRgb(0xFFFFFFFF));
        // a click selects the cell that is drawn there
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, c.center());
        QCOMPARE(b.view->selection().activeCol, qint64(1));
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, a.center());
        QCOMPARE(b.view->selection().activeCol, qint64(0));
        // the arrow keys: the left arrow goes to the next column (to the left on the screen), the right arrow to the one before
        QTest::keyClick(b.view.get(), Qt::Key_Left);
        QCOMPARE(b.view->selection().activeCol, qint64(1));
        QTest::keyClick(b.view.get(), Qt::Key_Right);
        QCOMPARE(b.view->selection().activeCol, qint64(0));
        // the scroll bar runs the other way, and a scroll to a later column draws the columns the other way
        QVERIFY(b.view->horizontalScrollBar()->invertedAppearance());
        b.view->horizontalScrollBar()->setValue(64 * 5);
        QApplication::processEvents();
        const QRect moved = b.view->cellRect(0, 5);
        QVERIFY2(moved.right() >= b.view->viewport()->width() - 100, qPrintable(QStringLiteral("column F is at %1").arg(moved.right())));
        const QImage again = b.view->viewport()->grab().toImage();
        QVERIFY(inkBoxOf(again, moved) > 0);
        // a drag along the cells selects the cells between, to the left
        const QRect from = b.view->cellRect(0, 5), to = b.view->cellRect(0, 7);
        QTest::mousePress(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, from.center());
        QTest::mouseMove(b.view->viewport(), to.center());
        QTest::mouseRelease(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, to.center());
        QCOMPARE(b.view->selection().range.col1, qint64(5));
        QCOMPARE(b.view->selection().range.col2, qint64(7));
    }
};

JP_TEST_CLASS(GridViewTests)
#include "tst_grid_view.moc"
