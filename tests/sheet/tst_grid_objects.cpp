// Tests of the objects drawn over the grid (libs/shell/grid/objectlayer.h and overlay.h): where an anchor puts an object on the rows and columns the sheet has, drawing the object's
// picture in each part of the grid (scrolled, zoomed, frozen), the pictures made only when they are needed and counted, the bounds on them, picking an object and deleting it, and the
// grid view giving the layer its parts and its clicks. The objects' pictures are solid colors, so no font or file is needed.
#include "sheethelpers.h"

#include "core/fonts.h"
#include "grid/gridview.h"
#include "grid/objectlayer.h"

#include <QApplication>
#include <QPainter>
#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace jp::grid;
using jp::office::sheet::AnchorKind;
using jp::office::sheet::DrawingAnchor;
using jp::office::sheet::kEmuPerPixel;

namespace {

DrawingAnchor twoCell(quint32 col1, quint32 row1, quint32 col2, quint32 row2, qint64 colOff1 = 0, qint64 rowOff1 = 0, qint64 colOff2 = 0, qint64 rowOff2 = 0)
{
    DrawingAnchor a;
    a.kind = AnchorKind::TwoCell;
    a.from = {col1, row1, colOff1, rowOff1};
    a.to = {col2, row2, colOff2, rowOff2};
    return a;
}

DrawingAnchor oneCell(quint32 col, quint32 row, qint64 cxPixels, qint64 cyPixels)
{
    DrawingAnchor a;
    a.kind = AnchorKind::OneCell;
    a.editAs = AnchorKind::OneCell;
    a.from = {col, row, 0, 0};
    a.cx = cxPixels * kEmuPerPixel;
    a.cy = cyPixels * kEmuPerPixel;
    return a;
}

DrawingAnchor absolute(qint64 x, qint64 y, qint64 w, qint64 h)
{
    DrawingAnchor a;
    a.kind = AnchorKind::Absolute;
    a.editAs = AnchorKind::Absolute;
    a.x = x * kEmuPerPixel;
    a.y = y * kEmuPerPixel;
    a.cx = w * kEmuPerPixel;
    a.cy = h * kEmuPerPixel;
    return a;
}

constexpr qint64 px(qint64 n) { return n * kEmuPerPixel; }

// An object whose picture is a solid color; the sizes it was asked for are kept.
struct Painted {
    QVector<QSize> asked;
    QVector<double> zooms;
    QVector<QSizeF> units;
    QVector<QRectF> inner;
};

LayerObject colored(int id, const DrawingAnchor &anchor, QRgb color, Painted *log = nullptr)
{
    LayerObject o;
    o.id = id;
    o.anchor = anchor;
    o.render = [color, log](const RenderRequest &q) {
        if (log) {
            log->asked.append(q.pixels);
            log->zooms.append(q.zoom);
            log->units.append(q.units);
            log->inner.append(q.inner);
        }
        QImage img(q.pixels, QImage::Format_ARGB32_Premultiplied);
        img.fill(color);
        return img;
    };
    return o;
}

const QRgb kRed = 0xFFFF0000, kGreen = 0xFF00A000, kBlue = 0xFF0000FF, kWhite = 0xFFFFFFFF;

// A layout of 400 by 300 pixels with no headers: rows 20 high, columns 64 wide.
struct Scene {
    GridLayout layout;
    ObjectLayer objects;
    QImage canvas{400, 300, QImage::Format_ARGB32_Premultiplied};
    Scene()
    {
        layout.setHeaders(false, 0, 0);
        layout.setViewport(QSize(400, 300));
        objects.attach(&layout);
    }
    // Draws every part of the grid, as the view does, on a white canvas.
    void draw()
    {
        canvas.fill(kWhite);
        QPainter p(&canvas);
        for (const Pane pane : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
            const QRect area = layout.pane(pane).rect;
            if (!area.isEmpty()) objects.paint(&p, layout, pane, area);
        }
    }
    QRgb at(int x, int y) const { return canvas.pixel(x, y); }
};

} // namespace

class GridObjectTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The place of an anchor on the rows and columns: between two places, at a place with a size, or at a place of the sheet; no further into a cell than it is wide; nothing where the
    // rows or columns are hidden.
    void anchorsPutObjectsOnTheRowsAndColumns()
    {
        Axis rows(1000, 20), cols(100, 64);
        ObjectBox b = anchorBox(twoCell(1, 2, 4, 9), rows, cols);
        QCOMPARE(b.x, 64.0);
        QCOMPARE(b.y, 40.0);
        QCOMPARE(b.w, 192.0);
        QCOMPARE(b.h, 140.0);
        // a place in a cell
        b = anchorBox(twoCell(1, 2, 4, 9, px(10), px(5), px(30), px(15)), rows, cols);
        QCOMPARE(b.x, 74.0);
        QCOMPARE(b.y, 45.0);
        QCOMPARE(b.w, 64.0 * 3 + 20.0);
        QCOMPARE(b.h, 140.0 + 10.0);
        // not further into a cell than the cell goes: each end is at the far end of its cell
        b = anchorBox(twoCell(1, 2, 2, 3, px(5000), px(5000), px(5000), px(5000)), rows, cols);
        QCOMPARE(b.x, 128.0);
        QCOMPARE(b.y, 60.0);
        QCOMPARE(b.w, 64.0);
        QCOMPARE(b.h, 20.0);
        // sizes of their own
        rows.setSize(3, 50);
        cols.setSize(2, 100);
        b = anchorBox(twoCell(1, 2, 4, 9), rows, cols);
        QCOMPARE(b.x, 64.0);
        QCOMPARE(b.y, 40.0);
        QCOMPARE(b.w, 64.0 + 100 + 64);
        QCOMPARE(b.h, 140.0 + 30);
        // a hidden row between: the object is shorter by it; all of its rows hidden: nothing
        rows.setSize(5, 0);
        rows.setSize(6, 0);
        b = anchorBox(twoCell(1, 2, 4, 9), rows, cols);
        QCOMPARE(b.h, 140.0 + 30 - 40);
        QVERIFY(anchorBox(twoCell(0, 5, 1, 7), rows, cols).isEmpty());
        QVERIFY(!anchorBox(twoCell(0, 5, 1, 8), rows, cols).isEmpty());
        cols.setSize(3, 0);
        QCOMPARE(anchorBox(twoCell(1, 2, 4, 9), rows, cols).w, 64.0 + 100);
        // one cell: a size of its own; absolute: where it says whatever the rows are
        b = anchorBox(oneCell(1, 2, 120, 80), rows, cols);
        QCOMPARE(b.x, 64.0);
        QCOMPARE(b.y, 40.0);
        QCOMPARE(b.w, 120.0);
        QCOMPARE(b.h, 80.0);
        b = anchorBox(absolute(30, 700, 50, 60), rows, cols);
        QCOMPARE(b.x, 30.0);
        QCOMPARE(b.y, 700.0);
        QCOMPARE(b.w, 50.0);
        QCOMPARE(b.h, 60.0);
        // far outside the sheet: a place far outside, with nothing wrong
        b = anchorBox(twoCell(16383, 1048575, 16383, 1048575, px(900), px(900), px(900), px(900)), Axis(), Axis(16384, 64));
        QVERIFY(std::isfinite(b.x) && std::isfinite(b.y) && b.w >= 0 && b.h >= 0);
        DrawingAnchor huge = absolute(0, 0, 0, 0);
        huge.x = huge.y = huge.cx = huge.cy = jp::office::sheet::kMaxDrawingEmu;
        b = anchorBox(huge, rows, cols);
        QVERIFY(std::isfinite(b.x) && b.w > 0);
    }

    // The picture is drawn where the anchor puts it, and moves with the scroll; what is outside the window is not asked for.
    void objectsMoveWithTheCells()
    {
        Scene s;
        Painted log;
        s.objects.setObjects({colored(1, twoCell(1, 2, 3, 5), kRed, &log), colored(2, twoCell(5, 12, 7, 14), kBlue, &log)});
        s.draw();
        QCOMPARE(s.at(64, 40), kRed);
        QCOMPARE(s.at(64 + 127, 40 + 59), kRed);
        QCOMPARE(s.at(63, 40), kWhite);
        QCOMPARE(s.at(64, 39), kWhite);
        QCOMPARE(s.at(64 + 128, 40), kWhite);
        QCOMPARE(s.at(64, 40 + 60), kWhite);
        QCOMPARE(s.at(5 * 64 + 10, 12 * 20 + 10), kBlue);    // (the second: row 12 is at 240, inside the 300 pixels the window shows)
        QCOMPARE(log.asked.value(0), QSize(128, 60));
        // scrolled by 25 pixels: the object is 25 up, and the pictures made are the ones before
        s.layout.setScroll(0, 25);
        const qint64 renders = s.objects.stats().renders;
        s.draw();
        QCOMPARE(s.at(64, 40 - 25), kRed);
        QCOMPARE(s.at(64, 40 - 26), kWhite);
        QCOMPARE(s.objects.stats().renders, renders);
        // scrolled to the right by two columns
        s.layout.setScroll(128, 0);
        s.draw();
        QCOMPARE(s.at(0, 40), kRed);
        QCOMPARE(s.at(63, 40), kRed);
        QCOMPARE(s.at(64, 40), kWhite);
        QCOMPARE(s.objects.stats().renders, renders);
    }

    // Zoom makes a picture of the size it is shown at, once; sizes of rows changed move the objects after layoutChanged.
    void zoomAndSizesChangeThePictures()
    {
        Scene s;
        Painted log;
        s.objects.setObjects({colored(1, twoCell(1, 2, 3, 5), kRed, &log)});
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(1));
        s.layout.setZoom(2.0);
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(2));
        QCOMPARE(log.asked.last(), QSize(256, 120));
        QCOMPARE(log.zooms.last(), 2.0);
        QCOMPARE(log.units.last(), QSizeF(128, 60));   // (the size at 100%, whatever the zoom)
        QCOMPARE(s.at(128, 80), kRed);
        QCOMPARE(s.at(127, 80), kWhite);
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(2));
        // rows 0 and 1 made taller: the object is lower, and its picture is the same size
        s.layout.setZoom(1.0);
        s.layout.rows.setSize(0, 40);
        s.layout.rows.setSize(1, 40);
        s.objects.layoutChanged();
        s.draw();
        QCOMPARE(s.at(64, 80), kRed);
        QCOMPARE(s.at(64, 79), kWhite);
        QCOMPARE(s.objects.stats().renders, qint64(3));   // (zoom 1.0 again: its picture was made for 2.0)
        // sizes changed with the totals and the number of rows that differ the same: the layer is told, and it works the places out again
        s.objects.setObjects({colored(2, twoCell(1, 1, 3, 3), kBlue)});
        s.layout.rows.setSize(0, 30);
        s.layout.rows.setSize(1, 30);
        s.objects.layoutChanged();
        s.draw();
        QCOMPARE(s.at(64, 30), kBlue);
        QCOMPARE(s.at(64, 29), kWhite);
        s.layout.rows.setSize(0, 25);
        s.layout.rows.setSize(1, 35);
        s.objects.layoutChanged();
        s.draw();
        QCOMPARE(s.at(64, 25), kBlue);
        QCOMPARE(s.at(64, 24), kWhite);
    }

    // Frozen rows and columns: an object in the frozen part stays when the rest scrolls, one in the part that scrolls goes under it, and one across the edge is drawn by each part as its
    // own coordinates put it.
    void frozenPartsDrawTheirObjects()
    {
        Scene s;
        s.layout.setFrozen(2, 1);   // rows 0 and 1 (40 pixels) and column 0 (64 pixels)
        s.objects.setObjects({colored(1, twoCell(0, 1, 1, 2), kRed), colored(2, twoCell(2, 3, 4, 5), kGreen), colored(3, twoCell(0, 0, 4, 1), kBlue)});
        s.layout.setScroll(64, 20);   // (one column and a row: the part that scrolls begins at x 128 and y 60 of the sheet)
        s.draw();
        QCOMPARE(s.at(10, 30), kRed);          // (in the frozen corner: it stays)
        QCOMPARE(s.at(10, 10), kBlue);         // (across the edge: the corner shows its first column)
        QCOMPARE(s.at(64 + 10, 10), kBlue);    // (and the top shows the rest of it, from the third column on: the second has scrolled away)
        QCOMPARE(s.at(64 + 127, 10), kBlue);
        QCOMPARE(s.at(64 + 128, 10), kWhite);
        // object 2 is in the part that scrolls: columns 2 to 4 and rows 3 to 5 (x 128 to 256, y 60 to 100), whose left and top edges are at the frozen edges
        QCOMPARE(s.at(64, 40), kGreen);
        QCOMPARE(s.at(64 + 127, 40 + 39), kGreen);
        QCOMPARE(s.at(64 + 128, 40), kWhite);
        QCOMPARE(s.at(64, 40 + 40), kWhite);
        // (and it does not show in the frozen column or rows)
        QCOMPARE(s.at(10, 60), kWhite);
        // scrolled back to the start: it is further from the frozen edges by what was scrolled
        s.layout.setScroll(0, 0);
        s.draw();
        QCOMPARE(s.at(128, 60), kGreen);
        QCOMPARE(s.at(127, 60), kWhite);
        QCOMPARE(s.at(128, 59), kWhite);
    }

    // The pictures are made when they are needed: at most kMaxRendersPerPaint in a paint, the rest by prepareAll(), and a changed object once.
    void picturesAreMadeWhenNeededAndCounted()
    {
        Scene s;
        std::vector<LayerObject> objects;
        for (int i = 0; i < 20; ++i) objects.push_back(colored(i + 1, twoCell(i % 6, i % 14, i % 6 + 1, i % 14 + 1), kRed));
        s.objects.setObjects(objects);
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(ObjectLayer::kMaxRendersPerPaint));
        QCOMPARE(s.objects.stats().placeholders, qint64(20 - ObjectLayer::kMaxRendersPerPaint));
        QCOMPARE(s.objects.pendingRenders(), 20 - ObjectLayer::kMaxRendersPerPaint);
        s.objects.prepareAll();
        QCOMPARE(s.objects.stats().renders, qint64(20));
        QCOMPARE(s.objects.pendingRenders(), 0);
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(20));
        QCOMPARE(s.objects.stats().placeholders, qint64(20 - ObjectLayer::kMaxRendersPerPaint));
        // one object's cells change: its old picture is drawn, and one is made again
        s.objects.invalidate(7);
        QCOMPARE(s.objects.pendingRenders(), 1);
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(20));
        s.objects.prepareAll();
        QCOMPARE(s.objects.stats().renders, qint64(21));
        s.objects.invalidateAll();
        QCOMPARE(s.objects.pendingRenders(), 20);
        s.objects.prepareAll();
        QCOMPARE(s.objects.stats().renders, qint64(41));
        // an object that is not on the screen is not made
        s.objects.setObjects({colored(1, twoCell(0, 500, 1, 502), kRed)});
        s.draw();
        s.objects.prepareAll();
        QCOMPARE(s.objects.stats().renders, qint64(41));
        QCOMPARE(s.objects.pendingRenders(), 0);
    }

    // The time between events makes the pictures a paint left: the layer's own timer, a step at a time.
    void theTimerMakesWhatAPaintLeft()
    {
        Scene s;
        std::vector<LayerObject> objects;
        for (int i = 0; i < 30; ++i) objects.push_back(colored(i + 1, twoCell(i % 6, i % 14, i % 6 + 1, i % 14 + 1), kRed));
        s.objects.setObjects(objects);
        s.draw();
        QSignalSpy repaint(&s.objects, &ObjectLayer::selectionChanged);   // (nothing to wait for in a signal: the repaint hook is the layer's way)
        int repaints = 0;
        s.objects.setRepaintHook([&repaints](const QRect &) { ++repaints; });
        QTRY_COMPARE(s.objects.pendingRenders(), 0);
        QVERIFY(repaints > 0);
        QCOMPARE(s.objects.stats().renders, qint64(30));
    }

    // Bounds: a picture is at most kMaxImagePixels however large the object, and the pictures held stay within the budget as the window moves over many large ones.
    void picturesAreBounded()
    {
        Scene s;
        Painted log;
        s.objects.setObjects({colored(1, absolute(10, 10, 40000, 40000), kGreen, &log)});
        s.draw();
        QCOMPARE(log.asked.size(), 1);
        QVERIFY(qint64(log.asked[0].width()) * log.asked[0].height() <= ObjectLayer::kMaxImagePixels);
        QCOMPARE(s.at(100, 100), kGreen);
        QCOMPARE(s.at(399, 299), kGreen);
        QCOMPARE(s.at(5, 5), kWhite);
        // scrolled far into it: the part of the picture under the window, from a picture a fraction of the size
        s.layout.setScroll(20000, 20000);
        s.layout.rows.setSize(0, 20);
        s.draw();
        QCOMPARE(s.at(200, 150), kGreen);
        QCOMPARE(log.asked.size(), 1);
        // many large objects, shown one after another
        std::vector<LayerObject> many;
        for (int i = 0; i < 70; ++i) many.push_back(colored(i + 1, absolute(0, i * 1000, 1000, 1000), kRed));
        s.layout.setScroll(0, 0);
        s.objects.setObjects(many);
        for (int i = 0; i < 70; ++i) {
            s.layout.setScroll(0, i * 1000);
            s.draw();
            s.objects.prepareAll();
        }
        QVERIFY2(s.objects.cacheBytes() <= ObjectLayer::kCacheBytes + 8 * 1000 * 1000 * 4, qPrintable(QString::number(s.objects.cacheBytes())));
        QVERIFY(s.objects.stats().evicted > 0);
        // (the last one is still there to draw)
        s.layout.setScroll(0, 69 * 1000);
        const qint64 renders = s.objects.stats().renders;
        s.draw();
        QCOMPARE(s.objects.stats().renders, renders);
        QCOMPARE(s.at(10, 10), kRed);
    }

    // A line has no width or no height and is still drawn; an object empty both ways is not. An object's margin is drawn around its box, and the draw function is told where the box is in it.
    void thinObjectsAndMargins()
    {
        Scene s;
        Painted log;
        LayerObject line = colored(1, twoCell(1, 2, 4, 2), kRed, &log);   // (one row, no height)
        line.thin = true;
        line.margin = 3;
        LayerObject flat = colored(2, twoCell(1, 5, 4, 5), kBlue);          // (the same, not a line: nothing)
        LayerObject margined = colored(3, twoCell(1, 8, 2, 9), kGreen, &log);
        margined.margin = 4;
        s.objects.setObjects({line, flat, margined});
        s.draw();
        // the line: 192 wide and no high, 3 around it
        QCOMPARE(log.asked.value(0), QSize(192 + 6, 1 + 6));
        QCOMPARE(log.inner.value(0), QRectF(3, 3, 192, 1));
        QCOMPARE(s.at(64 + 50, 40), kRed);
        QCOMPARE(s.at(64 + 50, 40 - 3), kRed);
        QCOMPARE(s.at(64 + 50, 40 - 4), kWhite);
        QCOMPARE(s.at(64 - 3, 40), kRed);
        QCOMPARE(s.at(64 - 4, 40), kWhite);
        QCOMPARE(s.at(64 + 50, 100), kWhite);   // (the other, not thin, is not drawn)
        // the margin: the box is where the anchor puts it, and the picture reaches 4 further
        QCOMPARE(log.asked.value(1), QSize(64 + 8, 20 + 8));
        QCOMPARE(log.inner.value(1), QRectF(4, 4, 64, 20));
        QCOMPARE(s.at(64 - 4, 160), kGreen);
        QCOMPARE(s.at(64 - 5, 160), kWhite);
        // the margin is not the object's: a click on it misses
        QCOMPARE(s.objects.objectAt(QPoint(64 - 2, 165)), -1);
        QCOMPARE(s.objects.objectAt(QPoint(64 + 2, 165)), 3);
    }

    // An object far outside the window, one that is not there, and a picture that cannot be made, cost nothing and draw nothing wrong.
    void objectsOutsideAndFailures()
    {
        Scene s;
        LayerObject failing = colored(3, twoCell(0, 0, 1, 1), kRed);
        failing.render = [](const RenderRequest &) { return QImage(); };
        s.objects.setObjects({colored(1, twoCell(16000, 1000000, 16001, 1000001), kRed), colored(2, twoCell(0, 0, 0, 0), kRed), failing});
        s.draw();
        s.objects.prepareAll();
        QCOMPARE(s.objects.stats().renders, qint64(1));   // (the failing one only)
        QCOMPARE(s.objects.pendingRenders(), 0);
        s.draw();
        QCOMPARE(s.objects.stats().renders, qint64(1));   // (and not again)
        QVERIFY(s.objects.stats().placeholders >= 2);
        bool outlined = false;
        for (int x = 0; x < 12; ++x) outlined = outlined || s.at(x, 0) != kWhite;   // (the dashed outline of the one that failed)
        QVERIFY(outlined);
        s.objects.invalidate(3);
        s.objects.prepareAll();
        QCOMPARE(s.objects.stats().renders, qint64(2));   // (asked again after its cells changed)
        s.objects.setObjects({});
        s.draw();
        QCOMPARE(s.at(1, 1), kWhite);
        // more objects than the limit are left out
        std::vector<LayerObject> flood;
        for (int i = 0; i < ObjectLayer::kMaxObjects + 100; ++i) flood.push_back(colored(i + 1, twoCell(0, 0, 1, 1), kRed));
        s.objects.setObjects(flood);
        QCOMPARE(s.objects.count(), ObjectLayer::kMaxObjects);
    }

    // Picking: the topmost object under the click, nothing for a click elsewhere (which goes to the cells); Delete is handed to the hook; Esc and other keys let the object go.
    void anObjectIsPickedAndDeleted()
    {
        Scene s;
        s.objects.setObjects({colored(1, twoCell(1, 2, 4, 6), kRed), colored(2, twoCell(2, 3, 5, 8), kBlue)});
        s.draw();
        QCOMPARE(s.objects.objectAt(QPoint(70, 45)), 1);
        QCOMPARE(s.objects.objectAt(QPoint(150, 80)), 2);   // (both are there: the later is in front)
        QCOMPARE(s.objects.objectAt(QPoint(10, 10)), -1);
        QSignalSpy picked(&s.objects, &ObjectLayer::selectionChanged);
        QVERIFY(s.objects.press(QPoint(70, 45), s.layout, Qt::NoModifier));
        QCOMPARE(s.objects.selectedId(), 1);
        QCOMPARE(picked.size(), 1);
        s.draw();
        QVERIFY(s.at(64, 40) != kRed);   // (the picked object has its frame)
        int deleted = 0, which = 0;
        s.objects.setDeleteHook([&](int id) {
            ++deleted;
            which = id;
        });
        QKeyEvent shift(QEvent::KeyPress, Qt::Key_Shift, Qt::ShiftModifier);
        QVERIFY(!s.objects.key(&shift));
        QCOMPARE(s.objects.selectedId(), 1);   // (a modifier alone changes nothing)
        QKeyEvent del(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QVERIFY(s.objects.key(&del));
        QCOMPARE(deleted, 1);
        QCOMPARE(which, 1);
        // the owner takes it away; it is not drawn or hit
        s.objects.setDeleted(1, true);
        QVERIFY(s.objects.isDeleted(1));
        QCOMPARE(s.objects.selectedId(), -1);
        s.draw();
        QCOMPARE(s.at(70, 45), kWhite);
        QCOMPARE(s.objects.objectAt(QPoint(70, 45)), -1);
        s.objects.setDeleted(1, false);
        s.draw();
        QCOMPARE(s.at(70, 45), kRed);
        // Esc lets go; another key lets go and goes to the cells; a click elsewhere lets go and goes to the cells
        s.objects.select(2);
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QVERIFY(s.objects.key(&esc));
        QCOMPARE(s.objects.selectedId(), -1);
        s.objects.select(2);
        QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QVERIFY(!s.objects.key(&down));
        QCOMPARE(s.objects.selectedId(), -1);
        s.objects.select(2);
        QVERIFY(!s.objects.press(QPoint(5, 5), s.layout, Qt::NoModifier));
        QCOMPARE(s.objects.selectedId(), -1);
        // with no hook, Delete is not taken
        s.objects.setDeleteHook(nullptr);
        s.objects.select(2);
        QVERIFY(!s.objects.key(&del));
    }

    // A chart sheet: the first object fills the part that scrolls, at its size.
    void aChartSheetFillsTheView()
    {
        Scene s;
        Painted log;
        s.objects.setObjects({colored(1, absolute(0, 0, 640, 480), kRed, &log), colored(2, absolute(0, 0, 10, 10), kBlue)});
        s.objects.setFillsView(true);
        s.draw();
        QCOMPARE(log.asked.value(0), QSize(400, 300));
        QCOMPARE(log.units.value(0), QSizeF(400, 300));
        QCOMPARE(s.at(0, 0), kRed);
        QCOMPARE(s.at(399, 299), kRed);
        QCOMPARE(s.objects.stats().renders, qint64(1));
        s.objects.setDeleted(1, true);
        s.draw();
        QCOMPARE(s.at(0, 0), kBlue);   // (the next that is not deleted)
    }
};

JP_TEST_CLASS(GridObjectTests)

// The view gives the layer its parts, its clicks, and its keys.
class GridViewObjectTests : public QObject {
    Q_OBJECT

    struct Book {
        Sheet sheet;
        Styles styles;
        SharedStrings strings;
        std::unique_ptr<GridView> view;
        ObjectLayer layer;
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
        void show()
        {
            sheet.cells.finish();
            view = std::make_unique<GridView>();
            view->resize(QSize(640, 420));
            GridData d;
            d.styles = &styles;
            d.strings = &strings;
            d.sheet = &sheet;
            view->setData(d);
            view->show();
            QVERIFY(QTest::qWaitForWindowExposed(view.get()));
            layer.attach(&view->sheet().layout());
            view->setOverlay(&layer);
        }
        ~Book()
        {
            if (view) view->setOverlay(nullptr);
        }
    };

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The picture is over the cells, in the window's pixels, and follows the scroll.
    void theViewDrawsTheLayerOverTheCells()
    {
        Book b;
        b.layer.setObjects({colored(1, twoCell(2, 3, 4, 6), kRed)});
        b.show();
        b.layer.prepareAll();
        const QRect r = b.view->sheet().layout().rangeRect(3, 2, 5, 3);
        QImage shot = b.view->viewport()->grab().toImage();
        QCOMPARE(shot.pixel(r.center()), kRed);
        QVERIFY(shot.pixel(r.left() - 3, r.center().y()) != kRed);
        // scrolled by a row: the object is a row up
        b.view->sheet().layout().setScroll(0, 20);
        b.view->viewport()->update();
        shot = b.view->viewport()->grab().toImage();
        QCOMPARE(shot.pixel(r.center().x(), r.top() - 20 + 5), kRed);
        QVERIFY(shot.pixel(r.center().x(), r.top() - 20 - 5) != kRed);
        // the layer takes away: the cells show
        b.view->setOverlay(nullptr);
        shot = b.view->viewport()->grab().toImage();
        QVERIFY(shot.pixel(r.center().x(), r.top() - 20 + 5) != kRed);
    }

    // A click on an object picks it and the cell cursor stays where it was; a click elsewhere goes to the cells; Delete goes to the layer's hook and not to the cells.
    void clicksAndKeysGoToTheLayerFirst()
    {
        Book b;
        b.layer.setObjects({colored(1, twoCell(2, 3, 4, 6), kRed)});
        b.show();
        b.layer.prepareAll();
        b.view->selectCell(0, 0);
        QSignalSpy cleared(b.view.get(), &GridView::clearRequested);
        const QRect r = b.view->sheet().layout().rangeRect(3, 2, 5, 3);
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
        QCOMPARE(b.layer.selectedId(), 1);
        QCOMPARE(b.view->selection().range.row1, qint64(0));   // (the cursor did not move)
        int deleted = 0;
        b.layer.setDeleteHook([&](int) { ++deleted; });
        QTest::keyClick(b.view.get(), Qt::Key_Delete);
        QCOMPARE(deleted, 1);
        QCOMPARE(cleared.size(), 0);
        // a click on a cell: the object is let go, and the cursor goes there
        const QRect cell = b.view->sheet().layout().cellRect(10, 5);
        QTest::mouseClick(b.view->viewport(), Qt::LeftButton, Qt::NoModifier, cell.center());
        QCOMPARE(b.layer.selectedId(), -1);
        QCOMPARE(b.view->selection().range.row1, qint64(10));
        // Delete with nothing picked clears the cells, as before
        QTest::keyClick(b.view.get(), Qt::Key_Delete);
        QCOMPARE(cleared.size(), 1);
    }

    // A change of the layout (a row made taller) moves the object with the cells.
    void sizesMoveTheObjects()
    {
        Book b;
        b.layer.setObjects({colored(1, twoCell(2, 3, 4, 6), kRed)});
        b.show();
        b.layer.prepareAll();
        b.view->sheet().layout().rows.setSize(0, 60);
        b.view->setZoom(1.0);   // (a view change that makes the layout be worked out again)
        b.view->setZoom(1.5);
        b.view->setZoom(1.0);
        const QRect r = b.view->sheet().layout().rangeRect(3, 2, 5, 3);
        b.layer.prepareAll();
        const QImage shot = b.view->viewport()->grab().toImage();
        QCOMPARE(shot.pixel(r.center()), kRed);
        QVERIFY(r.top() >= 3 * 20 + 40);
    }
};

JP_TEST_CLASS(GridViewObjectTests)
#include "tst_grid_objects.moc"
