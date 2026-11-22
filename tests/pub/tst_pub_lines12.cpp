// JeffPub's tests of the commands that closed lines of the feature list in slice 12: Merge Shapes, Convert to Curve, Polygon, and Contour, and Break Apart.
#include "pubhelpers.h"
#include "../withdialog.h"
#include "core/pathops.h"
#include "text/textengine.h"
#include "app/pagespane.h"
#include "office/connectionsites.h"
#include "render/renderer.h"
#include "render/shapes.h"
#include "app/formatpresets.h"
#include "core/texteffects.h"
#include "core/workthread.h"
#include "text/textprops.h"
#include <QDialogButtonBox>
#include <QTextBlock>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <cmath>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTest>

class PubLines12Tests : public QObject {
    Q_OBJECT

    static QString addShape(jp::Editor *ed, const QRectF &r, const QString &kind = QStringLiteral("rect"), const QColor &color = Qt::red)
    {
        auto s = std::make_shared<jp::ShapeItem>();
        s->rect = r;
        s->shape = kind;
        s->fill = jp::Fill::solid(jp::ColorRef::rgb(color));
        s->stroke = jp::Stroke::none();
        ed->doc()->pages[0]->items.push_back(s);
        return s->id;
    }
    static QColor pixel(jp::Editor *ed, int x, int y)
    {
        jp::PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        return jp::Renderer::renderToImage(ctx, 0, 1.0).pixelColor(x, y);
    }
    static bool filled(jp::Editor *ed, int x, int y) { return pixel(ed, x, y).green() < 128; }
    static int shapeCount(jp::Editor *ed) { return int(ed->doc()->pages[0]->items.size()); }
    static jp::ShapeItem *shapeAt(jp::Editor *ed, int i) { return dynamic_cast<jp::ShapeItem *>(ed->doc()->pages[0]->items[i].get()); }
    // Two overlapping squares, A (50..150) and B (100..200) on both axes, selected with A first.
    static void twoSquares(jp::MainWindow &w)
    {
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 100)), b = addShape(ed, QRectF(100, 100, 100, 100), QStringLiteral("rect"), Qt::blue);
        ed->select(QStringList{a, b});
    }
    // Triggers a command once the window has caught up with the selection and enabled it (a disabled command does nothing).
    static void fire(jp::MainWindow &w, const char *id)
    {
        QAction *a = w.act(QString::fromLatin1(id));
        QTRY_VERIFY(a->isEnabled());
        a->trigger();
    }
    // The window saves the publication and a second window opens it: the shapes come back as the first one had them.
    static jp::MainWindow *reopened(jp::MainWindow &w, const QTemporaryDir &dir, std::unique_ptr<jp::MainWindow> &holder)
    {
        const QString path = dir.filePath(QStringLiteral("merged.jpub"));
        if (!w.saveTo(path)) return nullptr;
        holder = std::make_unique<jp::MainWindow>();
        return holder->openFile(path) ? holder.get() : nullptr;
    }

    // ---- review fixes (Nov 21): holes, counted work, text, connectors
    static QPainterPath squarePath(double x0, double y0, double x1, double y1, bool reversed = false)
    {
        QPainterPath p;
        if (!reversed) {
            p.moveTo(x0, y0);
            p.lineTo(x1, y0);
            p.lineTo(x1, y1);
            p.lineTo(x0, y1);
        } else {
            p.moveTo(x0, y0);
            p.lineTo(x0, y1);
            p.lineTo(x1, y1);
            p.lineTo(x1, y0);
        }
        p.closeSubpath();
        return p;
    }
    static bool covers(const QVector<QPainterPath> &parts, const QPointF &at)
    {
        for (const QPainterPath &p : parts)
            if (p.contains(at)) return true;
        return false;
    }
    // A ring: the outer square and the inner one wound the same way under the odd-even rule (what a shape that is filled with holes has), or opposite ways under winding.
    static QPainterPath donutPath(bool oddEven)
    {
        QPainterPath p = squarePath(50, 50, 250, 250);
        p.addPath(squarePath(100, 100, 200, 200, !oddEven));
        p.setFillRule(oddEven ? Qt::OddEvenFill : Qt::WindingFill);
        return p;
    }
    // A five-pointed star drawn with one stroke that crosses itself (its middle is a pentagon the odd-even rule leaves empty and winding fills).
    static QPainterPath pentagram(bool oddEven)
    {
        QPainterPath p;
        for (int i = 0; i < 5; ++i) {
            const double a = -M_PI / 2 + 2 * M_PI * ((i * 2) % 5) / 5;
            const QPointF q(150 + 100 * std::cos(a), 150 + 100 * std::sin(a));
            if (i) p.lineTo(q);
            else p.moveTo(q);
        }
        p.closeSubpath();
        p.setFillRule(oddEven ? Qt::OddEvenFill : Qt::WindingFill);
        return p;
    }
    // A scribble of n corners in a 100 point square: it crosses itself a great deal. (Made from a fixed sequence: the same on every machine.)
    static QPainterPath scribble(int n, quint32 seed)
    {
        quint32 s = seed * 2654435761u + 12345u;
        QPainterPath p;
        for (int i = 0; i < n; ++i) {
            s = s * 1664525u + 1013904223u;
            const double x = (s >> 8) % 1000 / 10.0;
            s = s * 1664525u + 1013904223u;
            const double y = (s >> 8) % 1000 / 10.0;
            if (i) p.lineTo(x, y);
            else p.moveTo(x, y);
        }
        p.closeSubpath();
        return p;
    }
    static QPainterPath starPath(int n, int step, double radius, double cx, double cy)
    {
        QPainterPath p;
        for (int i = 0; i < n; ++i) {
            const double a = 2 * M_PI * (qint64(i) * step % n) / n;
            const QPointF q(cx + radius * std::cos(a), cy + radius * std::sin(a));
            if (i) p.lineTo(q);
            else p.moveTo(q);
        }
        p.closeSubpath();
        return p;
    }

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Shape Format > Insert Shapes > Merge Shapes: Union, Combine, Subtract, Intersect, and Fragment make shapes of the outlines, with the look of the shape selected first;
    // each is one step of Undo and Redo, and the shapes are kept by a save and open of the app.
    void mergeShapesMakesOneShapeOrTheirPieces()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        twoSquares(w);
        struct Case { const char *id; int count; bool onlyA, overlap, onlyB; QString undo; } cases[] = {
            {"shape.union", 1, true, true, true, "Union"}, {"shape.combine", 1, true, false, true, "Combine"},
            {"shape.subtract", 1, true, false, false, "Subtract"}, {"shape.intersect", 1, false, true, false, "Intersect"},
            {"shape.fragment", 3, true, true, true, "Fragment"}};
        for (const Case &c : cases) {
            twoSquares(w);
            QTRY_VERIFY(w.act(QString::fromLatin1(c.id))->isEnabled());
            w.act(QString::fromLatin1(c.id))->trigger();
            QCOMPARE(shapeCount(ed), c.count);
            QCOMPARE(filled(ed, 60, 60), c.onlyA);
            QCOMPARE(filled(ed, 125, 125), c.overlap);
            QCOMPARE(filled(ed, 190, 190), c.onlyB);
            QVERIFY(!filled(ed, 60, 190));   // (a corner neither covers)
            QCOMPARE(ed->undoStack()->undoText(), c.undo);
            QCOMPARE(ed->selectedItems().size(), c.count);
            // (the pieces take the look of the shape selected first: red, not B's blue)
            QCOMPARE(pixel(ed, 60, 60).red() > 200 || !c.onlyA, true);
            if (c.count == 1) {
                std::unique_ptr<jp::MainWindow> holder;
                jp::MainWindow *again = reopened(w, dir, holder);
                QVERIFY(again);
                QCOMPARE(shapeCount(again->editor()), 1);
                QCOMPARE(filled(again->editor(), 60, 60), c.onlyA);
                QCOMPARE(filled(again->editor(), 125, 125), c.overlap);
                QCOMPARE(filled(again->editor(), 190, 190), c.onlyB);
            }
            ed->undo();
            QCOMPARE(shapeCount(ed), 2);
            QVERIFY(filled(ed, 125, 125));
            QCOMPARE(pixel(ed, 125, 125).blue() > 200, true);   // (B is in front)
            ed->redo();
            QCOMPARE(shapeCount(ed), c.count);
        }
        // Fragment's three pieces are three separate shapes, each one island.
        twoSquares(w);
        fire(w, "shape.fragment");
        for (int i = 0; i < 3; ++i) QCOMPARE(int(jp::pathops::subpaths(shapeAt(ed, i)->customPath).size()), 1);
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("3")));
    }

    // Subtract of a shape inside another leaves a ring: its hole stays empty, and Break Apart makes the ring and the hole two shapes.
    void breakApartTakesAShapeToItsParts()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString outer = addShape(ed, QRectF(100, 100, 200, 200), QStringLiteral("ellipse")), inner = addShape(ed, QRectF(150, 150, 100, 100), QStringLiteral("ellipse"));
        ed->select(QStringList{outer, inner});
        fire(w, "shape.subtract");
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(!filled(ed, 200, 200));   // the hole
        QVERIFY(filled(ed, 120, 200));    // the ring
        QCOMPARE(int(jp::pathops::subpaths(shapeAt(ed, 0)->customPath).size()), 2);
        QTRY_VERIFY(w.act(QStringLiteral("shape.breakApart"))->isEnabled());
        fire(w, "shape.breakApart");
        QCOMPARE(shapeCount(ed), 2);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Break Apart"));
        QVERIFY(filled(ed, 200, 200));    // the hole is a filled shape of its own now
        QVERIFY(filled(ed, 120, 200));
        for (int i = 0; i < 2; ++i) QCOMPARE(int(jp::pathops::subpaths(shapeAt(ed, i)->customPath).size()), 1);
        std::unique_ptr<jp::MainWindow> holder;
        jp::MainWindow *again = reopened(w, dir, holder);
        QVERIFY(again);
        QCOMPARE(shapeCount(again->editor()), 2);
        QVERIFY(filled(again->editor(), 200, 200));
        ed->undo();
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(!filled(ed, 200, 200));
        ed->redo();
        QCOMPARE(shapeCount(ed), 2);
        // One piece has nothing to break apart: refused, no step.
        ed->select(ed->doc()->pages[0]->items[0]->id);
        const int steps = ed->undoStack()->count();
        fire(w, "shape.breakApart");
        QCOMPARE(ed->undoStack()->count(), steps);
        QVERIFY(!w.statusBar()->currentMessage().isEmpty());
    }

    // Convert to Curve, Polygon, and Contour: each makes the selected shape's outline its own; one step of Undo each, kept by a save and open.
    void convertMakesTheOutlineTheShapesOwn()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString id = addShape(ed, QRectF(100, 100, 200, 120), QStringLiteral("ellipse"));
        ed->select(id);
        QVERIFY(shapeAt(ed, 0)->customPath.isEmpty());
        fire(w, "shape.toCurve");
        QVERIFY(!shapeAt(ed, 0)->customPath.isEmpty());
        bool curves = false;
        for (int i = 0; i < shapeAt(ed, 0)->customPath.elementCount(); ++i) curves = curves || shapeAt(ed, 0)->customPath.elementAt(i).type == QPainterPath::CurveToElement;
        QVERIFY(curves);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Convert to Curve"));
        QVERIFY(filled(ed, 200, 160));
        ed->undo();
        QVERIFY(shapeAt(ed, 0)->customPath.isEmpty());
        ed->redo();
        const int steps = ed->undoStack()->count();
        fire(w, "shape.toCurve");   // (already a curve: refused, no step)
        QCOMPARE(ed->undoStack()->count(), steps);
        fire(w, "shape.toPolygon");
        const QPainterPath poly = shapeAt(ed, 0)->customPath;
        QVERIFY(poly.elementCount() > 8);
        for (int i = 0; i < poly.elementCount(); ++i) QVERIFY(poly.elementAt(i).type != QPainterPath::CurveToElement && poly.elementAt(i).type != QPainterPath::CurveToDataElement);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Convert to Polygon"));
        QVERIFY(filled(ed, 200, 160));
        std::unique_ptr<jp::MainWindow> holder;
        jp::MainWindow *again = reopened(w, dir, holder);
        QVERIFY(again);
        QCOMPARE(int(dynamic_cast<jp::ShapeItem *>(again->editor()->doc()->pages[0]->items[0].get())->customPath.elementCount()), int(poly.elementCount()));
        // Contour: a shape with a 10 point blue line becomes the line's outline, filled blue, with the middle empty.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString sq = addShape(ed, QRectF(100, 100, 200, 200));
        auto *s = shapeAt(ed, 0);
        s->stroke = jp::Stroke::line(jp::ColorRef::rgb(Qt::blue), 10);
        s->fill = jp::Fill();
        ed->select(sq);
        fire(w, "shape.toContour");
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(shapeAt(ed, 0)->stroke.isNone());
        QCOMPARE(pixel(ed, 100, 200).blue() > 200 && pixel(ed, 100, 200).red() < 80, true);
        QVERIFY(pixel(ed, 200, 200).red() > 200);   // (white: the middle is empty)
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Convert to Contour"));
    }

    // The limits: 64 shapes at a time, 256 pieces from a break apart; locked shapes, shapes in a group, and artwork are refused with a reason and no step.
    void limitsAndRefusals()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QStringList ids;
        for (int i = 0; i < 65; ++i) ids << addShape(ed, QRectF(20 + i * 4, 50, 30, 30));
        ed->select(ids);
        QString why;
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Union, &why));
        QCOMPARE(shapeCount(ed), 65);
        QVERIFY(why.contains(QStringLiteral("64")));
        ed->select(ids.mid(0, 64));
        QVERIFY(ed->mergeShapes(jp::pathops::Op::Union, &why));
        QCOMPARE(shapeCount(ed), 2);   // (64 made one; the 65th stays)
        // The subpaths read from one outline stop at 256, and a break apart that would make more is refused.
        QPainterPath many;
        for (int i = 0; i < 300; ++i) many.addRect(QRectF(i * 2, 0, 1, 1));
        QCOMPARE(int(jp::pathops::subpaths(many).size()), jp::pathops::kMostPieces + 1);   // (one past the limit tells a caller there are too many)
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString big = addShape(ed, QRectF(10, 10, 600, 10));
        shapeAt(ed, 0)->customPath = many;
        ed->select(big);
        QVERIFY(!ed->breakApart(&why));
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(why.contains(QStringLiteral("256")));
        // A shape with more elements than the limit is not merged at all.
        QPainterPath huge;
        for (int i = 0; i < jp::pathops::kMostElements / 4 + 10; ++i) huge.addRect(QRectF(i, 0, 0.5, 0.5));
        int refused = -1;
        QVERIFY(jp::pathops::merge(jp::pathops::Op::Union, {huge, many}, &refused).isEmpty());
        QCOMPARE(refused, 1);
        // Locked, grouped, and artwork.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 100)), b = addShape(ed, QRectF(100, 100, 100, 100));
        ed->doc()->item(b)->locked = true;
        ed->select(QStringList{a, b});
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Union, &why));
        QVERIFY(!why.isEmpty());
        ed->doc()->item(b)->locked = false;
        static_cast<jp::ShapeItem *>(ed->doc()->item(b))->shape = QStringLiteral("art");
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Union, &why));
        static_cast<jp::ShapeItem *>(ed->doc()->item(b))->shape = QStringLiteral("rect");
        ed->select(a);
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Union, &why));   // (one shape has nothing to merge with)
        QCOMPARE(shapeCount(ed), 2);
        ed->select(QStringList{a, b});
        QVERIFY(ed->mergeShapes(jp::pathops::Op::Intersect, &why));
        QCOMPARE(shapeCount(ed), 1);
        // Shapes that share no area leave nothing to intersect: refused, and the shapes stay.
        const QString c = addShape(ed, QRectF(400, 400, 20, 20)), d = addShape(ed, QRectF(500, 500, 20, 20));
        ed->select(QStringList{c, d});
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Intersect, &why));
        QCOMPARE(shapeCount(ed), 3);
    }
    // Home > Effects > Text Styles: 12 ready-made looks (fill, outline, and effects together) for the selected letters, each replacing what the letters had of the three; one step of
    // Undo, kept in a saved file that the app opens again, and drawn (the pixels change).
    // The effects of a run are parsed once for a string, whichever thread asks, and what a thread holds stays whole while other threads fill the cache past its size.
    void textEffectsAreKeptOnceForAllThreads()
    {
        const QString json = QStringLiteral("{\"o\":{\"on\":true,\"d\":7}}");
        const auto first = jp::TextEffects::cached(json);
        const int parsed = jp::TextEffects::parsedCount();
        jp::WorkThread other([&] {
            QCOMPARE(jp::TextEffects::cached(json).get(), first.get());
            for (int i = 0; i < 600; ++i) jp::TextEffects::cached(QStringLiteral("{\"o\":{\"on\":true,\"d\":%1}}").arg(1000 + i));
        });
        other.join();
        QCOMPARE(jp::TextEffects::parsedCount(), parsed + 600);
        QCOMPARE(first->outer.distance, jp::TextEffects::fromJson(json).outer.distance);   // (still whole, though the cache has let it go)
        QCOMPARE(jp::TextEffects::cached(json)->outer.distance, first->outer.distance);
    }

    void textStylesSetFillOutlineAndEffectsTogether()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::initCore();
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200), QStringLiteral("Styled")));
        ed->addItem(box);
        ed->select(box->id);
        auto format = [&](jp::Editor *e, const jp::TextItem *t) { return e->doc()->storyDoc(t->storyId)->firstBlock().begin().fragment().charFormat(); };
        auto effects = [&](const QTextCharFormat &cf) {
            const QString list = cf.stringProperty(jp::tp::TextFx);
            return list.isEmpty() ? jp::TextEffects() : jp::TextEffects::fromJson(list);
        };
        QCOMPARE(jp::formatpresets::textStyleCount(), 12);
        for (int i = 0; i < 12; ++i) {
            QVERIFY(w.act(QStringLiteral("textstyle.%1").arg(i)));
            QCOMPARE(w.act(QStringLiteral("textstyle.%1").arg(i))->text(), jp::formatpresets::textStyleName(i));
        }
        const QImage plain = [&] {
            jp::PaintContext ctx;
            ctx.doc = ed->doc();
            ctx.cache = &ed->cache();
            return jp::Renderer::renderToImage(ctx, 0, 1.0);
        }();
        QVERIFY(format(ed, box.get()).stringProperty(jp::tp::OutlineRef).isEmpty());
        w.act(QStringLiteral("textstyle.2"))->trigger();   // Bold Outline: clear letters with a 2 point outline
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Text Style"));
        QVERIFY(!format(ed, box.get()).stringProperty(jp::tp::OutlineRef).isEmpty());
        QCOMPARE(format(ed, box.get()).doubleProperty(jp::tp::OutlineWidth), 2.0);
        w.act(QStringLiteral("textstyle.4"))->trigger();   // Shadow: the outline of the style before goes, a shadow comes
        QVERIFY(format(ed, box.get()).stringProperty(jp::tp::OutlineRef).isEmpty());
        QVERIFY(effects(format(ed, box.get())).outer.on);
        w.act(QStringLiteral("textstyle.9"))->trigger();   // Reflection: the shadow goes
        QVERIFY(effects(format(ed, box.get())).reflection.on);
        QVERIFY(!effects(format(ed, box.get())).outer.on);
        w.act(QStringLiteral("textstyle.7"))->trigger();   // Glow
        QVERIFY(effects(format(ed, box.get())).glow.on);
        QVERIFY(!effects(format(ed, box.get())).reflection.on);
        w.act(QStringLiteral("textstyle.10"))->trigger();  // Emboss
        QVERIFY(format(ed, box.get()).boolProperty(jp::tp::Emboss));
        QVERIFY(!effects(format(ed, box.get())).glow.on);
        w.act(QStringLiteral("textstyle.11"))->trigger();  // Engrave: emboss goes
        QVERIFY(format(ed, box.get()).boolProperty(jp::tp::Engrave));
        QVERIFY(!format(ed, box.get()).boolProperty(jp::tp::Emboss));
        w.act(QStringLiteral("textstyle.4"))->trigger();
        jp::PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        const QImage styled = jp::Renderer::renderToImage(ctx, 0, 1.0);
        int different = 0;
        for (int y = 0; y < plain.height(); ++y)
            for (int x = 0; x < plain.width(); ++x) different += plain.pixel(x, y) != styled.pixel(x, y);
        QVERIFY2(different > 20, qPrintable(QString::number(different)));
        const QString path = dir.filePath(QStringLiteral("styled.jpub"));
        QVERIFY(w.saveTo(path));
        {
            jp::MainWindow again;
            QVERIFY(again.openFile(path));
            jp::TextItem *kept = nullptr;
            for (const jp::ItemPtr &it : again.editor()->surfaceItems())
                if (it->type() == jp::ItemType::Text) kept = static_cast<jp::TextItem *>(it.get());
            QVERIFY(kept);
            QVERIFY(effects(format(again.editor(), kept)).outer.on);
            QVERIFY(!format(again.editor(), kept).boolProperty(jp::tp::Engrave));
        }
        ed->undo();
        QVERIFY(format(ed, box.get()).boolProperty(jp::tp::Engrave));
        ed->redo();
        QVERIFY(effects(format(ed, box.get())).outer.on);
    }
    // Reroute Connectors: a connector attached to a selected shape takes, at each attached end, the connection site that makes its path the shortest; one step of Undo and Redo, and
    // the sites are kept in a saved file that the app opens again.
    void rerouteConnectorsTakesTheShortestPath()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 100, 80, 60)), b = addShape(ed, QRectF(300, 300, 80, 60));
        auto line = std::make_shared<jp::LineItem>();
        line->start = jp::LineItem::Glue{a, 0};   // the top of the first shape, and the bottom of the second: the long way round
        line->end = jp::LineItem::Glue{b, 2};
        line->route = jp::LineItem::Elbow;
        ed->doc()->pages[0]->items.push_back(line);
        ed->doc()->routeConnectors();
        const QString id = line->id;
        auto at = [&](jp::Editor *e) { return static_cast<jp::LineItem *>(e->doc()->item(id)); };
        ed->select(a);
        QTRY_VERIFY(w.act(QStringLiteral("shape.reroute"))->isEnabled());
        const double longWay = QLineF(at(ed)->p1, at(ed)->p2).length();
        fire(w, "shape.reroute");
        QCOMPARE(at(ed)->start.site, 3);   // (the right of the first shape and the left of the second lie nearest each other)
        QCOMPARE(at(ed)->end.site, 1);
        QVERIFY(QLineF(at(ed)->p1, at(ed)->p2).length() < longWay);
        QCOMPARE(at(ed)->p1, jp::connectionSite(*ed->doc()->item(a), 3));   // (the ends follow the sites)
        QCOMPARE(at(ed)->p2, jp::connectionSite(*ed->doc()->item(b), 1));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Reroute Connectors"));
        const int steps = ed->undoStack()->count();
        fire(w, "shape.reroute");   // (on the shortest path already: refused, no step)
        QCOMPARE(ed->undoStack()->count(), steps);
        QVERIFY(!w.statusBar()->currentMessage().isEmpty());
        std::unique_ptr<jp::MainWindow> holder;
        jp::MainWindow *again = reopened(w, dir, holder);
        QVERIFY(again);
        QCOMPARE(at(again->editor())->start.site, 3);
        QCOMPARE(at(again->editor())->end.site, 1);
        QCOMPARE(at(again->editor())->start.id, a);
        ed->undo();
        QCOMPARE(at(ed)->start.site, 0);
        QCOMPARE(at(ed)->end.site, 2);
        ed->redo();
        QCOMPARE(at(ed)->start.site, 3);
        // A shape no connector is attached to has nothing to reroute.
        const QString c = addShape(ed, QRectF(450, 50, 40, 40));
        ed->select(c);
        fire(w, "shape.reroute");
        QCOMPARE(ed->undoStack()->count(), steps);
    }
    // Insert > Pages > Section: a page starts a section whose page numbers begin again (or go on) in a format of its own; the page number fields, the Pages list, and a saved and reopened
    // file follow it; Merge with Previous Section takes it away; each is one step of Undo. A file's numbers are held to the limits (1 to 1,000, five formats, none on the first page).
    void sectionsCountThePageNumbersAgain()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->insertPages(0, 4, false, false);
        QCOMPARE(int(ed->doc()->pages.size()), 5);
        auto shown = [](jp::Editor *e, int page) {
            jp::FieldContext ctx;
            ctx.doc = e->doc();
            ctx.pageNumber = page;
            return ctx.resolve(QStringLiteral("page"));
        };
        for (int p = 1; p <= 5; ++p) QCOMPARE(shown(ed, p), QString::number(p));
        // Page 3 starts a section numbered again from 1 in roman; page 5 starts one that goes on, in capital letters.
        QVERIFY(w.act(QStringLiteral("page.sectionStart")));
        QVERIFY(w.act(QStringLiteral("page.sectionMerge")));
        QString why;
        QVERIFY(!ed->startSection(0, 1, 0, &why));   // (the first page has none)
        QVERIFY(!why.isEmpty());
        QVERIFY(ed->startSection(2, 1, 3));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Insert Section"));
        QVERIFY(ed->startSection(4, 0, 2));
        const QStringList want{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("i"), QStringLiteral("ii"), QStringLiteral("C")};
        for (int p = 1; p <= 5; ++p) QCOMPARE(shown(ed, p), want[p - 1]);
        QVERIFY(!ed->startSection(4, 0, 2));   // (the same again: nothing to change, no step)
        QCOMPARE(ed->doc()->sectionCount(), 3);
        // The Pages list shows a page that counts differently with its number after its place.
        jp::PagesPane *pane = w.findChild<jp::PagesPane *>();
        QVERIFY(pane);
        pane->refresh(false);
        QCOMPARE(pane->item(1)->text(), QStringLiteral("2"));
        QCOMPARE(pane->item(2)->text(), QStringLiteral("3 (i)"));
        QCOMPARE(pane->item(4)->text(), QStringLiteral("5 (C)"));
        // The dialog of the window sets the same.
        ed->setCurrentPage(3);
        withDialog([&] { fire(w, "page.sectionStart"); }, QStringLiteral("sectionDialog"), [&](QDialog *d) {
            auto *restart = d->findChild<QCheckBox *>(QStringLiteral("restart"));
            auto *start = d->findChild<QSpinBox *>(QStringLiteral("start"));
            auto *format = d->findChild<QComboBox *>(QStringLiteral("format"));
            QVERIFY(restart && start && format);
            QVERIFY(!restart->isChecked());
            QVERIFY(!start->isEnabled());
            restart->setChecked(true);
            QVERIFY(start->isEnabled());
            start->setValue(10);
            format->setCurrentIndex(1);   // 1, 2, 3
        });
        QCOMPARE(shown(ed, 4), QStringLiteral("10"));
        QCOMPARE(shown(ed, 5), QStringLiteral("K"));   // (page 5 goes on from 10 to 11, in its own capital letters)
        QVERIFY(ed->mergeSection(3));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Merge with Previous Section"));
        QCOMPARE(shown(ed, 4), QStringLiteral("ii"));
        ed->undo();
        QCOMPARE(shown(ed, 4), QStringLiteral("10"));
        ed->redo();
        QVERIFY(!ed->mergeSection(3));
        // Saved through the window and opened again.
        const QString path = dir.filePath(QStringLiteral("sections.jpub"));
        QVERIFY(w.saveTo(path));
        jp::MainWindow again;
        QVERIFY(again.openFile(path));
        for (int p = 1; p <= 5; ++p) QCOMPARE(shown(again.editor(), p), want[p - 1]);
        QCOMPARE(again.editor()->doc()->sectionCount(), 3);
        // A file's numbers are held to the limits: a start past 1,000, a format that is not one of the five, and a section on the first page.
        QJsonObject json = ed->doc()->toJson();
        QJsonArray pages = json["pages"].toArray();
        QJsonObject first = pages[0].toObject(), third = pages[2].toObject();
        first["section"] = true;
        first["sectionStart"] = 7;
        third["sectionStart"] = 99999;
        third["sectionFormat"] = 42;
        pages[0] = first;
        pages[2] = third;
        json["pages"] = pages;
        jp::Document hostile;
        hostile.fromJson(json);
        QVERIFY(!hostile.pages[0]->startsSection);
        QCOMPARE(hostile.pages[2]->sectionStart, 1000);
        QCOMPARE(hostile.pages[2]->sectionFormat, 4);
        QCOMPARE(hostile.pageNumberShown(3), 1000);
        QCOMPARE(hostile.pageNumberFormatOf(3), QStringLiteral("ROMAN"));
    }
    // Glue points: with Edit Points on, Shift+click on a shape puts a connection site of its own there (Shift+click on it again takes it away); a connector attached to it follows the
    // shape when it moves or is sized (the point keeps its share of the frame); one step of Undo each; kept by a save and open of the app; held to 16 a shape.
    void glueGivesAShapeConnectionSitesOfItsOwn()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(100, 100, 100, 60)), b = addShape(ed, QRectF(300, 300, 80, 60));
        auto *shape = static_cast<jp::ShapeItem *>(ed->doc()->item(a));
        QCOMPARE(jp::connectionSiteCount(*shape), 4);
        ed->select(a);
        QTRY_VERIFY(w.act(QStringLiteral("shape.editPoints"))->isEnabled());
        fire(w, "shape.editPoints");
        QCOMPARE(ed->pointsItem, a);
        const QPoint click = w.canvas()->pageToView(QPointF(125, 130)).toPoint();
        QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::ShiftModifier, click);
        shape = static_cast<jp::ShapeItem *>(ed->doc()->item(a));
        QCOMPARE(int(shape->gluePoints.size()), 1);
        QVERIFY(qAbs(shape->gluePoints[0].x() - 0.25) < 0.03 && qAbs(shape->gluePoints[0].y() - 0.5) < 0.03);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Add Glue Point"));
        QCOMPARE(jp::connectionSiteCount(*shape), 5);   // (the shape was turned into its own points by Edit Points: 4 sides, then the glue point)
        const QPointF site = jp::connectionSite(*shape, 4);
        QVERIFY(QLineF(site, QPointF(125, 130)).length() < 3);
        // A connector from the glue point to the second shape follows the first when it is moved and sized.
        auto line = std::make_shared<jp::LineItem>();
        line->start = jp::LineItem::Glue{a, 4};
        line->end = jp::LineItem::Glue{b, 1};
        ed->doc()->pages[0]->items.push_back(line);
        ed->doc()->routeConnectors();
        const QString id = line->id;
        auto start = [&](jp::Editor *e) { return static_cast<jp::LineItem *>(e->doc()->item(id))->p1; };
        QCOMPARE(start(ed), jp::connectionSite(*shape, 4));
        const QPointF from = start(ed);
        ed->change(QStringLiteral("Move"), [&] { ed->doc()->item(a)->moveBy(50, 20); });
        QVERIFY(QLineF(start(ed), from + QPointF(50, 20)).length() < 0.01);
        ed->change(QStringLiteral("Size"), [&] { ed->doc()->item(a)->scaleInto(ed->doc()->item(a)->rect, QRectF(150, 120, 200, 60)); });
        QVERIFY2(QLineF(start(ed), QPointF(150 + 200 * shape->gluePoints[0].x(), 120 + 60 * shape->gluePoints[0].y())).length() < 0.01, qPrintable(QString::number(start(ed).x())));
        // Saved through the window and opened again: the point and the connector stay.
        std::unique_ptr<jp::MainWindow> holder;
        jp::MainWindow *again = reopened(w, dir, holder);
        QVERIFY(again);
        auto *kept = static_cast<jp::ShapeItem *>(again->editor()->doc()->item(a));
        QVERIFY(kept);
        QCOMPARE(int(kept->gluePoints.size()), 1);
        QCOMPARE(static_cast<jp::LineItem *>(again->editor()->doc()->item(id))->start.site, 4);
        QVERIFY(QLineF(static_cast<jp::LineItem *>(again->editor()->doc()->item(id))->p1, start(ed)).length() < 0.01);
        // Shift+click on the point takes it away, and lets go of the connector attached to it.
        const QPoint onPoint = w.canvas()->pageToView(jp::connectionSite(*static_cast<jp::ShapeItem *>(ed->doc()->item(a)), 4)).toPoint();
        QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::ShiftModifier, onPoint);
        QCOMPARE(int(static_cast<jp::ShapeItem *>(ed->doc()->item(a))->gluePoints.size()), 0);
        QVERIFY(static_cast<jp::LineItem *>(ed->doc()->item(id))->start.id.isEmpty());
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Remove Glue Point"));
        ed->undo();
        QCOMPARE(static_cast<jp::LineItem *>(ed->doc()->item(id))->start.site, 4);
        QCOMPARE(int(static_cast<jp::ShapeItem *>(ed->doc()->item(a))->gluePoints.size()), 1);
        // A click off the shape adds nothing; the most a shape holds is 16.
        QString why;
        QVERIFY(!ed->addGluePoint(a, QPointF(500, 500), &why));
        QVERIFY(!why.isEmpty());
        for (int i = 0; i < 30; ++i) ed->addGluePoint(a, QPointF(160 + i, 130), &why);
        QCOMPARE(int(static_cast<jp::ShapeItem *>(ed->doc()->item(a))->gluePoints.size()), jp::kMostGluePoints);
        QVERIFY(why.contains(QStringLiteral("16")));
        // A file with more points than that, or points off the frame, is held to the limits.
        QJsonObject json = static_cast<jp::ShapeItem *>(ed->doc()->item(a))->toJson();
        QJsonArray many;
        for (int i = 0; i < 40; ++i) many.append(QJsonArray{i * 10.0 - 50, -3.0});
        json["glue"] = many;
        jp::ShapeItem hostile;
        hostile.fromJson(json);
        QCOMPARE(int(hostile.gluePoints.size()), jp::kMostGluePoints);
        for (const QPointF &q : hostile.gluePoints) QVERIFY(q.x() >= 0 && q.x() <= 1 && q.y() == 0);
    }
    // Three shapes inside one another, in every operation: what each covers is what it should (the holes between them stay empty).
    void nestedShapesKeepTheirHolesInEveryOperation()
    {
        using namespace jp::pathops;
        const QVector<QPainterPath> nest{squarePath(50, 50, 250, 250), squarePath(100, 100, 200, 200), squarePath(140, 140, 160, 160)};
        struct Want { Op op; bool ring, between, middle; int count; } wants[] = {
            {Op::Union, true, true, true, 1}, {Op::Combine, true, false, true, 1}, {Op::Subtract, true, false, false, 1}, {Op::Intersect, false, false, true, 1}, {Op::Fragment, true, true, true, 3}};
        for (const Want &w : wants) {
            int refused = -1;
            const QVector<QPainterPath> out = merge(w.op, nest, &refused);
            QVERIFY2(!out.isEmpty(), qPrintable(QString::number(int(w.op)) + " refused " + QString::number(refused)));
            QCOMPARE(int(out.size()), w.count);
            QCOMPARE(covers(out, QPointF(70, 70)), w.ring);
            QCOMPARE(covers(out, QPointF(120, 120)), w.between);
            QCOMPARE(covers(out, QPointF(150, 150)), w.middle);
            QVERIFY(!covers(out, QPointF(30, 30)));
            for (const QPainterPath &p : out) QCOMPARE(p.fillRule(), Qt::OddEvenFill);
        }
    }

    // A ring (under either fill rule) with another shape in each operation, and the result of a step used in the next: the hole stays a hole.
    void aRingKeepsItsHoleWhateverIsDoneWithIt()
    {
        using namespace jp::pathops;
        const QPainterPath far = squarePath(400, 400, 450, 450), big = squarePath(0, 0, 300, 300), part = squarePath(40, 40, 150, 150);
        for (bool oddEven : {true, false}) {
            const QPainterPath ring = donutPath(oddEven);
            const QString rule = oddEven ? QStringLiteral("odd-even") : QStringLiteral("winding");
            for (Op op : {Op::Union, Op::Combine, Op::Subtract, Op::Intersect, Op::Fragment}) {
                const QVector<QPainterPath> out = merge(op, {ring, op == Op::Intersect ? big : far});
                QVERIFY2(!out.isEmpty(), qPrintable(rule + " " + QString::number(int(op))));
                QVERIFY2(!covers(out, QPointF(150, 150)), qPrintable(rule + " hole, op " + QString::number(int(op))));
                QVERIFY2(covers(out, QPointF(70, 70)), qPrintable(rule + " ring, op " + QString::number(int(op))));
                QCOMPARE(covers(out, QPointF(420, 420)), op == Op::Union || op == Op::Combine || op == Op::Fragment);
            }
            // The shape that covers half the hole fills that half and no more.
            const QVector<QPainterPath> half = merge(Op::Union, {ring, part});
            QVERIFY(covers(half, QPointF(120, 120)));
            QVERIFY(!covers(half, QPointF(180, 180)));
            // Steps in a row: a square minus a square, then the result with a far shape (the result of the first step is a ring that has the winding of its own).
            const QVector<QPainterPath> first = merge(Op::Subtract, {squarePath(50, 50, 250, 250), squarePath(100, 100, 200, 200)});
            QCOMPARE(int(first.size()), 1);
            const QVector<QPainterPath> second = merge(Op::Union, {first[0], far});
            QVERIFY(!covers(second, QPointF(150, 150)));
            QVERIFY(covers(second, QPointF(70, 70)) && covers(second, QPointF(420, 420)));
            const QVector<QPainterPath> third = merge(Op::Union, {ring, far, squarePath(500, 500, 520, 520)});
            QVERIFY(!covers(third, QPointF(150, 150)));
            QVERIFY(covers(third, QPointF(510, 510)));
        }
    }

    // A path that crosses itself keeps the fill rule it has: its middle is empty under odd-even and filled under winding, in every operation.
    void aStarThatCrossesItselfKeepsItsFillRule()
    {
        using namespace jp::pathops;
        const QPainterPath far = squarePath(400, 400, 450, 450), big = squarePath(0, 0, 400, 400);
        for (bool oddEven : {true, false}) {
            for (Op op : {Op::Union, Op::Combine, Op::Subtract, Op::Intersect, Op::Fragment}) {
                const QVector<QPainterPath> out = merge(op, {pentagram(oddEven), op == Op::Intersect ? big : far});
                QVERIFY(!out.isEmpty());
                QCOMPARE(covers(out, QPointF(150, 150)), !oddEven);
                QVERIFY(covers(out, QPointF(150, 150 - 85)));   // (a point of the upper tip)
                QVERIFY(!covers(out, QPointF(5, 5)));
            }
        }
    }

    // The same through the window: a ring made by Subtract, merged again with a far shape, saved and opened again: the hole is empty at each stage, and the outline keeps its rule.
    void aRingMergedAgainKeepsItsHoleThroughSaveAndOpen()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString outer = addShape(ed, QRectF(50, 50, 200, 200)), inner = addShape(ed, QRectF(100, 100, 100, 100));
        ed->select(QStringList{outer, inner});
        fire(w, "shape.subtract");
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(!filled(ed, 150, 150) && filled(ed, 70, 70));
        const QString ring = ed->doc()->pages[0]->items[0]->id, far = addShape(ed, QRectF(400, 400, 50, 50));
        ed->select(QStringList{ring, far});
        fire(w, "shape.union");
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(!filled(ed, 150, 150));   // (the hole comes back filled when the outline is merged under the winding rule)
        QVERIFY(filled(ed, 70, 70) && filled(ed, 420, 420));
        QCOMPARE(shapeAt(ed, 0)->customPath.fillRule(), Qt::OddEvenFill);
        std::unique_ptr<jp::MainWindow> holder;
        jp::MainWindow *again = reopened(w, dir, holder);
        QVERIFY(again);
        QCOMPARE(shapeCount(again->editor()), 1);
        QVERIFY(!filled(again->editor(), 150, 150));
        QVERIFY(filled(again->editor(), 70, 70) && filled(again->editor(), 420, 420));
        // A shape with holes of its own under the odd-even rule (two squares wound the same way) merged with another.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString donut = addShape(ed, QRectF(50, 50, 200, 200));
        shapeAt(ed, 0)->customPath = QTransform::fromTranslate(-50, -50).map(donutPath(true));
        QVERIFY(!filled(ed, 150, 150));
        const QString other = addShape(ed, QRectF(400, 400, 50, 50), QStringLiteral("rect"), Qt::blue);
        ed->select(QStringList{donut, other});
        fire(w, "shape.combine");
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(!filled(ed, 150, 150));
        QVERIFY(filled(ed, 70, 70) && filled(ed, 420, 420));
        QVERIFY(reopened(w, dir, holder));
        QVERIFY(!filled(holder->editor(), 150, 150));
    }

    // The work is counted before it is done, and a command whose work passes the budget is refused (nothing is run). The four inputs that froze the window for minutes
    // (all within the element limits) are refused for every operation; the count is the test, not a time.
    void workIsCountedAndRefusedBeforeItIsDone()
    {
        using namespace jp::pathops;
        QVector<QVector<QPainterPath>> cases(4);
        for (int i = 0; i < 4; ++i) cases[0] << starPath(5999, 2999, 100, i * 3, i * 2);
        for (int i = 0; i < 64; ++i) cases[1] << starPath(370, 171, 100, i * 0.5, 0);
        for (int i = 0; i < 64; ++i) cases[2] << scribble(370, i + 1);
        for (int i = 0; i < 4; ++i) cases[3] << scribble(5990, i + 1);
        for (int c = 0; c < cases.size(); ++c) {
            qint64 total = 0;
            for (const QPainterPath &p : cases[c]) {
                QVERIFY(p.elementCount() <= kMostElements);   // (within the stated limits: the elements do not bound the work)
                total += p.elementCount();
            }
            QVERIFY(total <= kMostTotalElements);
            for (Op op : {Op::Union, Op::Combine, Op::Subtract, Op::Intersect, Op::Fragment}) {
                const QString what = QStringLiteral("case %1, op %2").arg(c).arg(int(op));
                QVERIFY2(estimateWork(op, cases[c]) > kMostWork, qPrintable(what));
                int refused = -1;
                qint64 done = -1;
                QVERIFY2(merge(op, cases[c], &refused, {}, &done).isEmpty(), qPrintable(what));
                QCOMPARE(refused, 3);
                QCOMPARE(done, qint64(0));   // (refused before any step)
            }
        }
        // What is only above the quick work is done, and counted: 64 ovals that overlap are cheap.
        QVector<QPainterPath> ovals;
        for (int i = 0; i < 64; ++i) {
            QPainterPath o;
            o.addEllipse(QPointF(i * 5, i * 3), 50, 40);
            ovals << o;
        }
        qint64 done = -1;
        QVERIFY(estimateWork(Op::Union, ovals) < kMostWork);
        QCOMPARE(int(merge(Op::Union, ovals, nullptr, {}, &done).size()), 1);
        QVERIFY(done > 0 && done <= kMostSteps);
        // A scribble pair passes the first count and is refused by the count of a step, for Subtract and Intersect as for the others (the same growth checks on what the steps make).
        const QVector<QPainterPath> pair{scribble(250, 7), scribble(250, 8)};
        for (Op op : {Op::Union, Op::Subtract, Op::Intersect}) {
            QVERIFY(estimateWork(op, pair) <= kMostWork);
            int refused = -1;
            qint64 spent = 0;
            QVERIFY(merge(op, pair, &refused, {}, &spent).isEmpty());
            QCOMPARE(refused, 3);
            QVERIFY(spent > kMostSteps);
        }
        // The steps are counted: a progress function that says stop ends the merge at that step, refused as cancelled.
        int steps = 0, refused = -1;
        QVERIFY(merge(Op::Union, {squarePath(0, 0, 10, 10), squarePath(5, 5, 20, 20), squarePath(8, 8, 30, 30)}, &refused, [&](int step, int) { steps = step + 1; return step < 2; }).isEmpty());
        QCOMPARE(refused, 4);
        QCOMPARE(steps, 3);
    }

    // A merge above the quick work runs on a thread of its own with a box that cancels it; the result is one step of Undo, and a cancel leaves the shapes as they were.
    void aLongMergeRunsOnAWorkerAndCanBeCancelled()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        auto addScribbles = [&] {
            ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
            QStringList ids;
            QVector<QPainterPath> outlines;
            for (int i = 0; i < 3; ++i) {
                ids << addShape(ed, QRectF(10, 10, 100, 100));
                shapeAt(ed, i)->customPath = scribble(150, 7 + i);
                outlines << shapeAt(ed, i)->customPath.translated(10, 10);
            }
            ed->select(ids);
            return outlines;
        };
        const QVector<QPainterPath> outlines = addScribbles();
        QVERIFY(jp::pathops::estimateWork(jp::pathops::Op::Union, outlines) > jp::pathops::kQuickWork);
        QVERIFY(jp::pathops::estimateWork(jp::pathops::Op::Union, outlines) <= jp::pathops::kMostWork);
        QThread *main = QThread::currentThread();
        std::atomic<int> steps{0};
        std::atomic<bool> offMain{true};
        ed->mergeStepHook = [&](int, std::atomic<bool> &) {
            ++steps;
            if (QThread::currentThread() == main) offMain = false;
        };
        bool boxShown = false;
        int started = 0, ended = 0;
        QMetaObject::Connection c1 = connect(ed, &jp::Editor::busyStarted, ed, [&](const QString &, std::atomic<bool> *) {
            ++started;
            auto *box = w.findChild<QDialog *>(QStringLiteral("busyBox"));
            boxShown = box && box->isVisible() && box->findChild<QDialogButtonBox *>();
        });
        QMetaObject::Connection c2 = connect(ed, &jp::Editor::busyEnded, ed, [&] { ++ended; });
        QString why;
        int made = 0;
        const int undoBefore = ed->undoStack()->count();
        QVERIFY2(ed->mergeShapes(jp::pathops::Op::Union, &why, &made), qPrintable(why));
        QCOMPARE(made, 1);
        QCOMPARE(shapeCount(ed), 1);
        QCOMPARE(ed->undoStack()->count(), undoBefore + 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Union"));
        QVERIFY(boxShown);
        QCOMPARE(started, 1);
        QCOMPARE(ended, 1);
        QVERIFY(steps > 0);
        QVERIFY(offMain.load());
        QTRY_VERIFY(!w.findChild<QDialog *>(QStringLiteral("busyBox")));
        // Cancel in the box (pressed before the worker starts): the shapes stay, no step, and the message says so.
        addScribbles();
        steps = 0;
        QObject::disconnect(c1);
        c1 = connect(ed, &jp::Editor::busyStarted, ed, [&](const QString &, std::atomic<bool> *) {
            auto *box = w.findChild<QDialog *>(QStringLiteral("busyBox"));
            QVERIFY(box);
            auto *buttons = box->findChild<QDialogButtonBox *>();
            QVERIFY(buttons);
            buttons->button(QDialogButtonBox::Cancel)->click();
        });
        const int undoAfter = ed->undoStack()->count();
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Union, &why, &made));
        QVERIFY(why.contains(QStringLiteral("Cancelled")));
        QCOMPARE(shapeCount(ed), 3);
        QCOMPARE(ed->undoStack()->count(), undoAfter);
        QCOMPARE(steps.load(), 1);   // (stopped at the first step)
        QTRY_VERIFY(!w.findChild<QDialog *>(QStringLiteral("busyBox")));
        // Cancel from the worker's own step: the work stops at the next step (counted, not timed), and the shapes stay.
        addScribbles();
        QObject::disconnect(c1);
        steps = 0;
        ed->mergeStepHook = [&](int step, std::atomic<bool> &cancel) {
            ++steps;
            if (step == 2) cancel = true;
        };
        QVERIFY(!ed->mergeShapes(jp::pathops::Op::Union, &why, &made));
        QCOMPARE(steps.load(), 3);
        QCOMPARE(shapeCount(ed), 3);
        QCOMPARE(ed->undoStack()->count(), undoAfter);
        QObject::disconnect(c2);
        ed->mergeStepHook = nullptr;
    }

    // The busy box going leaves what others hung on its destroyed signal (the style sheet holds one): only its own connections are cut.
    void whatOthersHungOnTheBusyBoxSurvivesItsEnd()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QStringList ids;
        for (int i = 0; i < 3; ++i) {
            ids << addShape(ed, QRectF(10, 10, 100, 100));
            shapeAt(ed, i)->customPath = scribble(150, 7 + i);
        }
        ed->select(ids);
        QObject sink;
        int boxGone = 0;
        bool hung = false;
        connect(ed, &jp::Editor::busyStarted, &sink, [&](const QString &, std::atomic<bool> *) {
            auto *box = w.findChild<QDialog *>(QStringLiteral("busyBox"));
            if (!box) return;
            hung = true;
            connect(box, &QObject::destroyed, &sink, [&boxGone] { ++boxGone; });
        });
        QString why;
        int made = 0;
        QVERIFY2(ed->mergeShapes(jp::pathops::Op::Union, &why, &made), qPrintable(why));
        QVERIFY(hung);
        QTRY_COMPARE(boxGone, 1);
    }

    // Every outline a merge makes is closed (it ends where it began), so that the stroke of one joins at its start: a fragment of ovals and a rectangle had open ones.
    void mergeResultsAreClosedOutlines()
    {
        using namespace jp::pathops;
        QPainterPath e1, e2, r;
        e1.addEllipse(0, 0, 100, 80);
        e2.addEllipse(50, 20, 100, 80);
        r.addRect(10, 10, 200, 30);
        for (Op op : {Op::Union, Op::Combine, Op::Subtract, Op::Intersect, Op::Fragment}) {
            const QVector<QPainterPath> out = merge(op, {e1, e2, r});
            QVERIFY(!out.isEmpty());
            for (const QPainterPath &p : out) {
                QPointF start;
                int open = 0;
                for (int i = 0; i <= p.elementCount(); ++i) {
                    const bool end = i == p.elementCount() || p.elementAt(i).type == QPainterPath::MoveToElement;
                    if (end && i > 0 && QPointF(p.elementAt(i - 1)) != start) ++open;
                    if (i < p.elementCount() && p.elementAt(i).type == QPainterPath::MoveToElement) start = p.elementAt(i);
                }
                QVERIFY2(open == 0, qPrintable(QStringLiteral("op %1: %2 outlines left open").arg(int(op)).arg(open)));
            }
        }
        // The polygon of a curve is closed too, and a stroke of it joins at the start (the corner of a square is filled out to the point).
        const QPainterPath square = polygon(squarePath(0, 0, 100, 100));
        QVERIFY(contour(square, 10).contains(QPointF(-3, -3)));
    }

    // A merge that is far past the budget is refused with a message, and nothing opens a box.
    void aMergeTooBigIsRefusedWithAMessage()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QStringList ids;
        for (int i = 0; i < 4; ++i) {
            ids << addShape(ed, QRectF(10, 10, 100, 100));
            shapeAt(ed, i)->customPath = starPath(5999, 2999, 50, 50 + i * 3, 50 + i * 2);
        }
        ed->select(ids);
        int started = 0;
        connect(ed, &jp::Editor::busyStarted, ed, [&] { ++started; });
        QString why;
        fire(w, "shape.union");
        QCOMPARE(shapeCount(ed), 4);
        QCOMPARE(started, 0);
        QVERIFY(ed->mergeShapes(jp::pathops::Op::Union, &why) == false);
        QVERIFY(why.contains(QStringLiteral("too complex")));
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("too complex")));
    }

    // Text in shapes: Break Apart and Convert keep it, Merge removes it and says so (and asks once when more than one of the shapes has some).
    void textInShapesIsKeptOrRemovedAsTheCommandSays()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        auto textOf = [&](int i) {
            const QString id = shapeAt(ed, i)->storyId;
            const QTextDocument *d = id.isEmpty() ? nullptr : ed->doc()->storyDoc(id);
            return d ? d->toPlainText() : QString();
        };
        auto withText = [&](const QRectF &r, const QString &text, const QColor &color = Qt::red) {
            const QString id = addShape(ed, r, QStringLiteral("rect"), color);
            auto *s = static_cast<jp::ShapeItem *>(ed->doc()->item(id));
            s->storyId = ed->doc()->createStory(text);
            return id;
        };
        // Break Apart: the first piece has the text.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString two = withText(QRectF(10, 10, 200, 50), QStringLiteral("Hello"));
        QPainterPath halves = squarePath(0, 0, 50, 50);
        halves.addPath(squarePath(150, 0, 200, 50));
        shapeAt(ed, 0)->customPath = halves;
        ed->select(two);
        fire(w, "shape.breakApart");
        QCOMPARE(shapeCount(ed), 2);
        QCOMPARE(textOf(0), QStringLiteral("Hello"));
        QVERIFY(shapeAt(ed, 1)->storyId.isEmpty());
        ed->undo();
        QCOMPARE(shapeCount(ed), 1);
        QCOMPARE(textOf(0), QStringLiteral("Hello"));
        // Convert (each of the three) keeps it, a contour too.
        for (const char *id : {"shape.toCurve", "shape.toPolygon", "shape.toContour"}) {
            ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
            const QString one = withText(QRectF(10, 10, 100, 50), QStringLiteral("Keep me"), Qt::red);
            shapeAt(ed, 0)->stroke = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 4);
            ed->select(one);
            fire(w, id);
            QCOMPARE(shapeCount(ed), 1);
            QCOMPARE(textOf(0), QStringLiteral("Keep me"));
        }
        // Merge with one shape that has text: removed, and the status bar says so; no question.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = withText(QRectF(50, 50, 100, 100), QStringLiteral("Alpha")), b = addShape(ed, QRectF(100, 100, 100, 100), QStringLiteral("rect"), Qt::blue);
        ed->select(QStringList{a, b});
        QCOMPARE(ed->mergeTextCount(), 1);
        bool asked = false;
        QTimer watch;
        QObject::connect(&watch, &QTimer::timeout, [&] {
            if (auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
                asked = true;
                d->reject();
            }
        });
        watch.start(20);
        fire(w, "shape.union");
        QVERIFY(!asked);
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(shapeAt(ed, 0)->storyId.isEmpty());
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("removed")));
        // More than one with text: one question. Cancel leaves the shapes and their text; Merge removes it.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString c = withText(QRectF(50, 50, 100, 100), QStringLiteral("One")), d2 = withText(QRectF(100, 100, 100, 100), QStringLiteral("Two"), Qt::blue),
                      e = withText(QRectF(300, 300, 50, 50), QStringLiteral("Three"));
        ed->select(QStringList{c, d2, e});
        QCOMPARE(ed->mergeTextCount(), 3);
        watch.stop();
        const int steps = ed->undoStack()->count();
        withDialog([&] { fire(w, "shape.fragment"); }, QStringLiteral("mergeTextBox"), [&](QDialog *) {}, false);
        QCOMPARE(shapeCount(ed), 3);
        QCOMPARE(ed->undoStack()->count(), steps);
        QCOMPARE(textOf(0), QStringLiteral("One"));
        withDialog([&] { fire(w, "shape.union"); }, QStringLiteral("mergeTextBox"), [&](QDialog *box) {
            QPushButton *go = nullptr;
            for (QPushButton *p : box->findChildren<QPushButton *>())
                if (p->text() == QStringLiteral("Merge")) go = p;
            QVERIFY(go);
            go->click();
        });
        QCOMPARE(shapeCount(ed), 1);
        QVERIFY(shapeAt(ed, 0)->storyId.isEmpty());
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("removed")));
    }

    // A connector attached to a shape that is merged or broken apart follows the shape's new outline when the site it was at is a site of a new shape, and is let go where it was otherwise.
    void connectorsFollowAMergedShapeWhenTheirSiteStays()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString a = addShape(ed, QRectF(50, 50, 100, 100)), b = addShape(ed, QRectF(100, 50, 100, 100)), far = addShape(ed, QRectF(300, 400, 40, 40), QStringLiteral("rect"), Qt::blue);
        auto connect = [&](const QString &from, int site, const QString &to, int toSite) {
            auto line = std::make_shared<jp::LineItem>();
            line->start = jp::LineItem::Glue{from, site};
            line->end = jp::LineItem::Glue{to, toSite};
            ed->doc()->pages[0]->items.push_back(line);
            return line->id;
        };
        const jp::ShapeItem *sa = static_cast<jp::ShapeItem *>(ed->doc()->item(a));
        const int left = jp::connectionSiteOnSide(*sa, 1), right = jp::connectionSiteOnSide(*sa, 3);
        const QString onLeft = connect(a, left, far, 0), onRight = connect(a, right, far, 0), onB = connect(b, jp::connectionSiteOnSide(*sa, 3), far, 0);
        ed->doc()->routeConnectors();
        auto line = [&](const QString &id) { return static_cast<jp::LineItem *>(ed->doc()->item(id)); };
        const QPointF rightAt = line(onRight)->p1;
        QCOMPARE(rightAt, QPointF(150, 100));
        ed->select(QStringList{a, b});
        fire(w, "shape.union");
        QCOMPARE(shapeCount(ed), 5);   // (the union, the far shape, and the three connectors)
        const QString merged = ed->selectedItems().first()->id;
        // The left side of A is the left side of the union (its middle is at 50, 100): attached to the union. B's right side is the union's right side.
        QCOMPARE(line(onLeft)->start.id, merged);
        QCOMPARE(jp::connectionSite(*ed->doc()->item(merged), line(onLeft)->start.site), QPointF(50, 100));
        QCOMPARE(line(onB)->start.id, merged);
        QCOMPARE(jp::connectionSite(*ed->doc()->item(merged), line(onB)->start.site), QPointF(200, 100));
        // A's right side is inside the union: not a site of it, so the connector is let go and stays where it was.
        QVERIFY(line(onRight)->start.id.isEmpty());
        QCOMPARE(line(onRight)->p1, rightAt);
        QCOMPARE(line(onLeft)->end.id, far);   // (the other end is on a shape that stayed)
        // Break Apart: the sides of the whole that are the sides of a piece stay attached, the top (the middle of the gap) is let go.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        const QString whole = addShape(ed, QRectF(10, 10, 150, 50)), other = addShape(ed, QRectF(300, 400, 40, 40), QStringLiteral("rect"), Qt::blue);
        QPainterPath halves = squarePath(0, 0, 50, 50);
        halves.addPath(squarePath(100, 0, 150, 50));
        shapeAt(ed, 0)->customPath = halves;
        const jp::ShapeItem *sw = static_cast<jp::ShapeItem *>(ed->doc()->item(whole));
        const QString l1 = connect(whole, jp::connectionSiteOnSide(*sw, 1), other, 0), l2 = connect(whole, jp::connectionSiteOnSide(*sw, 3), other, 0), l3 = connect(whole, jp::connectionSiteOnSide(*sw, 0), other, 0);
        ed->doc()->routeConnectors();
        const QPointF top = line(l3)->p1;
        ed->select(whole);
        fire(w, "shape.breakApart");
        QCOMPARE(shapeCount(ed), 6);   // (the two pieces, the other shape, and the three connectors)
        QCOMPARE(line(l1)->start.id, shapeAt(ed, 0)->id);
        QCOMPARE(line(l2)->start.id, shapeAt(ed, 1)->id);
        QVERIFY(line(l3)->start.id.isEmpty());
        QCOMPARE(line(l3)->p1, top);
    }

    // A section starts with its page. Moved to the front, the page after it starts the section (the first page cannot); deleted, the page after it starts it; both in the step that moves or deletes.
    void aSectionStartMovesWithItsPageAndIsNotLost()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        auto fresh = [&] {
            ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
            ed->insertPages(0, 4, false, false);
            QVERIFY(ed->startSection(2, 1, 3));   // (page 3 starts a section of its own: again from 1, in lowercase roman)
        };
        auto shown = [&](jp::Editor *e, int page) {
            jp::FieldContext ctx;
            ctx.doc = e->doc();
            ctx.pageNumber = page;
            return ctx.resolve(QStringLiteral("page"));
        };
        auto starts = [&](int i) { return ed->doc()->pages[i]->startsSection; };
        fresh();
        QCOMPARE(int(ed->doc()->pages.size()), 5);
        // To the front: the old first page is page 2, and it starts the section.
        int steps = ed->undoStack()->index();
        ed->movePage(2, 0);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QVERIFY(!starts(0) && starts(1) && !starts(2));
        QCOMPARE(ed->doc()->pages[1]->sectionStart, 1);
        QCOMPARE(ed->doc()->pages[1]->sectionFormat, 3);
        QCOMPARE(ed->doc()->sectionCount(), 2);
        const QStringList want{QStringLiteral("1"), QStringLiteral("i"), QStringLiteral("ii"), QStringLiteral("iii"), QStringLiteral("iv")};
        for (int p = 1; p <= 5; ++p) QCOMPARE(shown(ed, p), want[p - 1]);
        std::unique_ptr<jp::MainWindow> holder;
        jp::MainWindow *again = reopened(w, dir, holder);   // (a section start on the first page is dropped on open: this one is not on it)
        QVERIFY(again);
        QCOMPARE(again->editor()->doc()->sectionCount(), 2);
        for (int p = 1; p <= 5; ++p) QCOMPARE(shown(again->editor(), p), want[p - 1]);
        ed->undo();
        QVERIFY(starts(2) && !starts(0) && !starts(1));
        // Elsewhere: the start goes with the page.
        ed->movePage(2, 3);
        QVERIFY(starts(3) && !starts(2));
        QCOMPARE(ed->doc()->sectionCount(), 2);
        ed->undo();
        // A page that is not the section's first moves out of it and the section stays on its page.
        ed->movePage(3, 0);
        QVERIFY(starts(3) && !starts(0));
        ed->undo();
        // Deleted: the page after it starts the section, in the one step, and undo brings everything back.
        steps = ed->undoStack()->index();
        ed->deletePage(2);
        QCOMPARE(ed->undoStack()->index(), steps + 1);
        QCOMPARE(int(ed->doc()->pages.size()), 4);
        QVERIFY(starts(2));
        QCOMPARE(ed->doc()->pages[2]->sectionStart, 1);
        QCOMPARE(ed->doc()->pages[2]->sectionFormat, 3);
        QCOMPARE(ed->doc()->sectionCount(), 2);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 5);
        QVERIFY(starts(2) && !starts(3));
        // The last page of the publication starts the section and is deleted: the section goes with it.
        ed->movePage(2, 4);
        ed->deletePage(4);
        QCOMPARE(ed->doc()->sectionCount(), 1);
        // A section's page that becomes the first page (the page before it is deleted, or moved off) cannot start a section: what it numbered is the publication's own now.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->insertPages(0, 3, false, false);
        QVERIFY(ed->startSection(1, 7, 4));
        ed->deletePage(0);
        QVERIFY(!starts(0));
        QCOMPARE(ed->doc()->setup.firstPageNumber, 7);
        QCOMPARE(ed->doc()->setup.pageNumberFormat, QStringLiteral("ROMAN"));
        QCOMPARE(shown(ed, 1), QStringLiteral("VII"));
        QCOMPARE(ed->doc()->sectionCount(), 1);
        ed->undo();
        QVERIFY(starts(1));
        QCOMPARE(ed->doc()->setup.firstPageNumber, 1);
        ed->movePage(0, 2);
        QVERIFY(!starts(0));
        QCOMPARE(ed->doc()->setup.firstPageNumber, 7);
    }

    // The numbers and formats a page shows are looked up in a table made once after a change: they are the ones counted the long way, whatever the sections; and the key a page number field
    // is laid out under carries the number and format the page shows, so a section put in before the page lays it out again.
    // The page-number field, a date field, a formatted merge field, and a field of the shared engine on the pages of two sections (the second restarts at 1, in capital roman numerals),
    // all worked out in one layout pass: each shows its own text, no page's text leaks to another through the pass's cache, and the pass's budget counts a field once.
    void pageDateMergeAndEngineFieldsTogetherInTwoSections()
    {
        jp::initCore();
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->insertPages(0, 3, false, false);
        ed->doc()->merge.fields = {QStringLiteral("Price")};
        ed->doc()->merge.rows = {{QStringLiteral("1234.5")}};
        QVERIFY(ed->startSection(2, 1, int(jp::Document::pageNumberFormats().indexOf(QStringLiteral("ROMAN")))));
        const jp::Document &doc = *ed->doc();
        QCOMPARE(int(doc.pages.size()), 4);
        const QDateTime when(QDate(2026, 3, 4), QTime(15, 30));
        const QStringList pageShows = {QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("I"), QStringLiteral("II")};
        const QString numpages = QStringLiteral("fld:NUMPAGES");
        auto at = [&](const std::shared_ptr<jp::FieldWork> &pass, int page) {
            jp::FieldContext c;
            c.doc = &doc;
            c.work = pass;
            c.pageNumber = page;
            c.pageCount = 4;
            c.mergeRecord = 0;
            c.now = when;
            return c;
        };
        auto pass = std::make_shared<jp::FieldWork>();
        // Forward, then backward and again: what a page shows never depends on which page was asked first.
        for (int round = 0; round < 3; ++round)
            for (int k = 0; k < 4; ++k) {
                const int page = round == 1 ? 4 - k : k + 1;
                const jp::FieldContext c = at(pass, page);
                QCOMPARE(c.resolve(QStringLiteral("page")), pageShows[page - 1]);
                QCOMPARE(c.resolve(QStringLiteral("date:yyyy-MM-dd")), QStringLiteral("2026-03-04"));
                QCOMPARE(c.resolve(QStringLiteral("merge:Price|currency")), QStringLiteral("$1,234.50"));
                QCOMPARE(c.resolve(numpages), QStringLiteral("4"));
            }
        // The engine's field is worked out once for each place it shows (here a page of its own), however often the pass asks.
        auto cost = [&](int page, int times) {
            auto fresh = std::make_shared<jp::FieldWork>();
            for (int i = 0; i < times; ++i)
                if (at(fresh, page).resolve(numpages) != QStringLiteral("4")) return qint64(-1);
            return fresh->spent.load();
        };
        const qint64 once = cost(1, 1);
        QVERIFY(once > 0);
        QCOMPARE(cost(1, 5), once);
        QCOMPARE(cost(3, 5), once);
        // Four places, so four times the work and no more, however many times each is asked.
        QCOMPARE(pass->spent.load(), 4 * once);
    }

    void sectionTableMatchesTheLongCountAndTheLayoutKeyFollowsIt()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->insertPages(0, 11, false, false);
        auto longCount = [&](int page, QString *format) {
            const jp::Document &d = *ed->doc();
            int n = d.setup.firstPageNumber;
            QString f = d.setup.pageNumberFormat;
            const int upTo = std::min<int>(std::max(page, 1), int(d.pages.size()));
            for (int i = 1; i < upTo; ++i) {
                const jp::Page &p = *d.pages[i];
                n = p.startsSection && p.sectionStart > 0 ? p.sectionStart : n + 1;
                if (p.startsSection && p.sectionFormat >= 0) f = jp::Document::pageNumberFormats()[p.sectionFormat];
            }
            *format = f;
            return n + (page > upTo ? page - upTo : 0);
        };
        auto check = [&] {
            for (int page = -1; page <= int(ed->doc()->pages.size()) + 3; ++page) {
                QString format;
                const int n = longCount(page, &format);
                QCOMPARE(ed->doc()->pageNumberShown(page), n);
                QCOMPARE(ed->doc()->pageNumberFormatOf(page), format);
            }
        };
        check();
        QVERIFY(ed->startSection(3, 10, 3));
        check();
        QVERIFY(ed->startSection(5, 0, 2));
        check();
        QVERIFY(ed->startSection(8, 2, -1));
        QVERIFY(ed->startSection(9, 0, 4));
        check();
        QVERIFY(ed->mergeSection(5));
        check();
        ed->movePage(3, 7);
        check();
        ed->deletePage(8);
        check();
        ed->undo();
        check();
        ed->doc()->setup.firstPageNumber = 4;   // (the first section's own numbering is read as it is, not kept in the table)
        check();
        // The layout key of a page number field on the last page.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->insertPages(0, 4, false, false);
        jp::FieldContext ctx;
        ctx.doc = ed->doc();
        ctx.pageNumber = 5;
        const QString plain = ctx.key(false);
        QVERIFY(ed->startSection(2, 1, 0));   // (a section before the page: it shows 3, not 5)
        const QString renumbered = ctx.key(false);
        QVERIFY(renumbered != plain);
        QVERIFY(ed->startSection(2, 1, 3));   // (the same number, in another format)
        const QString reformatted = ctx.key(false);
        QVERIFY(reformatted != renumbered);
        QVERIFY(ed->mergeSection(2));
        QCOMPARE(ctx.key(false), plain);
    }

    // The .pub file keeps every section's numbering: the start number, the style, and where it begins; a save and open of the app brings back what each page shows. A glue point is named as a loss.
    void everySectionsNumberingIsKeptByAPubFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        jp::Editor *ed = w.editor();
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        ed->insertPages(0, 5, false, false);
        ed->doc()->setup.firstPageNumber = 3;
        QVERIFY(ed->startSection(2, 1, 3));    // lowercase roman from 1
        QVERIFY(ed->startSection(4, 0, 2));    // goes on, in capital letters
        QVERIFY(ed->startSection(5, 20, -1));  // from 20, in the same style
        const QString path = dir.filePath(QStringLiteral("sections.pub"));
        QVERIFY(w.saveTo(path));
        auto holder = std::make_unique<jp::MainWindow>();
        QVERIFY(holder->openFile(path));
        const jp::Document &back = *holder->editor()->doc();
        QCOMPARE(int(back.pages.size()), 6);
        QCOMPARE(back.sectionCount(), 4);
        for (int p = 1; p <= 6; ++p) {
            QCOMPARE(back.pageNumberShown(p), ed->doc()->pageNumberShown(p));
            QCOMPARE(back.pageNumberFormatOf(p), ed->doc()->pageNumberFormatOf(p));
        }
        QCOMPARE(back.pageNumberShown(6), 20);
        QCOMPARE(back.pageNumberFormatOf(6), QStringLiteral("ALPHA"));
        // What a copy to .pub loses is listed: nothing about the sections, and the glue points when a shape has some.
        const QStringList plain = jp::publisherLosses(*ed->doc());
        for (const QString &line : plain) QVERIFY(!line.contains(QStringLiteral("lue point")) && !line.contains(QStringLiteral("ection")));
        const QString shape = addShape(ed, QRectF(100, 100, 80, 40));
        QVERIFY(ed->addGluePoint(shape, QPointF(120, 120)));
        bool named = false;
        for (const QString &line : jp::publisherLosses(*ed->doc())) named = named || line.contains(QStringLiteral("lue points can"));
        QVERIFY(named);
    }
};

JP_TEST_CLASS(PubLines12Tests)
#include "tst_pub_lines12.moc"
