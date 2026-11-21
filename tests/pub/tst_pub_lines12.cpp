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
#include "text/textprops.h"
#include <QTextBlock>
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
};

JP_TEST_CLASS(PubLines12Tests)
#include "tst_pub_lines12.moc"
