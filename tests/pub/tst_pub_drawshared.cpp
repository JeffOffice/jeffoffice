// Tests of the drawing code the apps share: the points of an outline (Edit Points), where a connector meets an outline, the effects of a shape in DrawingML, and a custom geometry written from
// an outline. Built here from paths and a few lines of XML.

#include "core/pathpoints.h"
#include "core/picturestyles.h"
#include "office/drawingml_effects.h"
#include "office/drawingml_geometry.h"
#include "render/shapes.h"
#include "pubhelpers.h"

#include "office/drawingml.h"
#include "office/xml.h"
#include <QBuffer>
#include <QDomDocument>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QTest>

using namespace jp;

class DrawSharedTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void theVerticesOfARectangleAreItsFourCornersAndTheClosingLineIsNotOne()
    {
        const QPainterPath rect = shapePath(QStringLiteral("rect"), QSizeF(100, 60));
        const QVector<QPointF> points = pts::vertexPoints(rect);
        QCOMPARE(points.size(), 4);
        QVERIFY(points.contains(QPointF(100, 60)));
        QVERIFY(pts::withinCaps(rect));
    }

    void movingTakingAwayAndPuttingInAPointChangesOnlyThatPoint()
    {
        const QPainterPath rect = shapePath(QStringLiteral("rect"), QSizeF(100, 60));
        const QVector<int> at = pts::vertices(rect);
        int corner = -1;
        for (const int i : at)
            if (QLineF(QPointF(rect.elementAt(i).x, rect.elementAt(i).y), QPointF(100, 60)).length() < 0.01) corner = i;
        QVERIFY(corner >= 0);
        const QPainterPath moved = pts::moved(rect, corner, QPointF(20, 10));
        QVERIFY(pts::vertexPoints(moved).contains(QPointF(120, 70)));
        QCOMPARE(pts::vertexPoints(moved).size(), 4);
        const QPainterPath fewer = pts::removed(rect, at[1]);
        QCOMPARE(pts::vertexPoints(fewer).size(), 3);
        // (A subpath keeps at least three points.)
        QCOMPARE(pts::vertexPoints(pts::removed(fewer, pts::vertices(fewer)[1])).size(), 3);
        int made = -2;
        const QPainterPath more = pts::inserted(rect, QPointF(50, 2), 5, &made);
        QCOMPARE(pts::vertexPoints(more).size(), 5);
        QVERIFY(made >= 0);
        pts::inserted(rect, QPointF(50, 30), 5, &made);
        QCOMPARE(made, -1);
    }

    void aPathOfTooManyElementsIsPastTheCap()
    {
        QPainterPath big(QPointF(0, 0));
        for (int i = 0; i < pts::kMostElements + 10; ++i) big.lineTo(i % 100, i / 100);
        QVERIFY(!pts::withinCaps(big));
    }

    void connectorsMeetTheOutlineAndNotTheBoxRoundIt()
    {
        const QSizeF size(100, 100);
        const QVector<QPointF> rect = pts::sideSites(shapePath(QStringLiteral("rect"), size), size);
        QCOMPARE(rect.size(), 4);
        QVERIFY(QLineF(rect[0], QPointF(50, 0)).length() < 0.01 && QLineF(rect[1], QPointF(0, 50)).length() < 0.01);
        QVERIFY(QLineF(rect[2], QPointF(50, 100)).length() < 0.01 && QLineF(rect[3], QPointF(100, 50)).length() < 0.01);
        const QVector<QPointF> star = pts::sideSites(shapePath(QStringLiteral("star5"), size), size);
        QVERIFY(star[3].x() < 99 && star[3].x() > 60);
        QVERIFY(star[1].x() > 1);
    }

    void aShadowIsPutInAndTakenOutWhileTheOtherEffectsStayInTheSchemasOrder()
    {
        using namespace office;
        QByteArray list = "<a:effectLst><a:innerShdw blurRad=\"1\"/><a:glow rad=\"5\"/></a:effectLst>";
        list = effectListWith(list, ShapeEffectKind::Shadow, true);
        QVERIFY(list.contains("<a:innerShdw blurRad=\"1\"/>") && list.contains("<a:glow rad=\"5\"/>") && list.contains("<a:outerShdw"));
        QVERIFY(list.indexOf("<a:glow") < list.indexOf("<a:innerShdw") && list.indexOf("<a:innerShdw") < list.indexOf("<a:outerShdw"));
        QVERIFY(hasShapeEffect(list, ShapeEffectKind::Shadow) && hasShapeEffect(list, ShapeEffectKind::Glow) && !hasShapeEffect(list, ShapeEffectKind::Reflection));
        // Put in again, it is one shadow; taken out, the others stay.
        QCOMPARE(effectListWith(list, ShapeEffectKind::Shadow, true), list);
        const QByteArray without = effectListWith(list, ShapeEffectKind::Shadow, false);
        QVERIFY(!without.contains("outerShdw") && without.contains("<a:glow rad=\"5\"/>") && without.contains("innerShdw"));
        QCOMPARE(effectListWith(effectListWith(QByteArray(), ShapeEffectKind::Glow, true), ShapeEffectKind::Glow, false), QByteArray());
        QCOMPARE(shapeEffectKinds().size(), 5);
    }

    void aBevelKeepsTheShapesOtherThreeDSettings()
    {
        using namespace office;
        Solid3d have;
        have.scene = "<a:scene3d><a:camera prst=\"perspectiveFront\"/><a:lightRig rig=\"soft\" dir=\"t\"/></a:scene3d>";
        have.solid = "<a:sp3d extrusionH=\"12700\"><a:extrusionClr><a:srgbClr val=\"FF0000\"/></a:extrusionClr></a:sp3d>";
        const Solid3d with = bevelWith(have, true);
        QVERIFY(with.solid.contains("extrusionH=\"12700\"") && with.solid.contains("<a:extrusionClr>") && with.solid.contains("<a:bevelT"));
        QVERIFY(with.solid.indexOf("<a:bevelT") < with.solid.indexOf("<a:extrusionClr>"));
        QCOMPARE(with.scene, have.scene);
        QVERIFY(hasShapeEffect(with.solid, ShapeEffectKind::Bevel));
        const Solid3d back = bevelWith(with, false);
        QCOMPARE(back.solid, have.solid);
        QCOMPARE(back.scene, have.scene);
        // A shape with only the bevel has no 3-D settings left; one with none gets the usual camera and light.
        const Solid3d fresh = bevelWith(Solid3d(), true);
        QVERIFY(fresh.scene.contains("orthographicFront") && fresh.solid.contains("<a:bevelT"));
        QVERIFY(bevelWith(fresh, false).solid.isEmpty() && bevelWith(fresh, false).scene.isEmpty());
    }

    void aCustomGeometryIsWrittenFromAnOutlineAndReadBackAsTheSameOutline()
    {
        const QSizeF size(100, 60);
        const QPainterPath outline = shapePath(QStringLiteral("ellipse"), size);
        const QRectF text = shapeTextRect(QStringLiteral("ellipse"), size);
        const QByteArray xml = office::customGeometryXml(outline, size, text, pts::sideSites(outline, size));
        QVERIFY(xml.contains("<a:cxn ") && xml.contains("<a:cubicBezTo>") && xml.contains("<a:close/>"));
        QDomDocument dom;
        QVERIFY(dom.setContent("<x xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">" + xml + "</x>", true));
        const office::Geometry g = office::readCustomGeometry(dom.documentElement().firstChildElement(), size);
        QCOMPARE(g.paths.size(), 1);
        const QPainterPath read = g.paths[0].path;
        QCOMPARE(pts::vertexPoints(read).size(), pts::vertexPoints(outline).size());
        for (int i = 0; i < pts::vertexPoints(read).size(); ++i) QVERIFY(QLineF(pts::vertexPoints(read)[i], pts::vertexPoints(outline)[i]).length() < 0.01);
        // The text rectangle is the ellipse's own and not the whole frame.
        QVERIFY(xml.contains(QByteArray("l=\"") + QByteArray::number(qint64(std::llround(text.left() * 12700))) + "\""));
        QVERIFY(text.left() > 1);
    }
    // ---- picture effects in the renderer

    static std::shared_ptr<PictureItem> effectPicture(Document &doc, const QRectF &rect)
    {
        QImage img(120, 80, QImage::Format_RGB32);
        QPainter ip(&img);
        QLinearGradient lg(0, 0, 120, 80);
        lg.setColorAt(0, QColor(40, 120, 220));
        lg.setColorAt(1, QColor(240, 200, 40));
        ip.fillRect(img.rect(), lg);
        ip.end();
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc.addImage(png, QStringLiteral("png"));
        pic->rect = rect;
        pic->imgRect = QRectF(QPointF(0, 0), rect.size());
        return pic;
    }

    static QImage paintOf(Document &doc, const PictureItem &pic, double zoom = 1, QSize size = QSize(320, 240))
    {
        QImage out(size, QImage::Format_ARGB32_Premultiplied);
        out.fill(Qt::white);
        QPainter p(&out);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(zoom, zoom);
        PaintContext ctx;
        ctx.doc = &doc;
        Renderer::paintItem(&p, ctx, pic);
        p.end();
        return out;
    }

    static Effects effectsRead(const QByteArray &list)
    {
        QDomDocument dom;
        const QByteArray xml = "<a:spPr xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">" + list + "</a:spPr>";
        if (!office::parseXml(xml, &dom)) return Effects();
        office::DrawingContext ctx;
        return office::readEffects(dom.documentElement(), QDomElement(), ctx);
    }

    // A file may ask for a blur of hundreds of points and a shadow a thousand points away (blurRad 2540000 and dist 20116800 are 200 and 1584 points): at 400 percent the buffers the effects
    // are made in stay under the cap (4 million pixels each), and the picture is still drawn.
    void hostileEffectSizesKeepTheEffectBuffersUnderTheCap()
    {
        Document doc;
        auto pic = effectPicture(doc, QRectF(20, 20, 120, 80));
        pic->fx = effectsRead("<a:effectLst><a:glow rad=\"2540000\"><a:srgbClr val=\"FF0000\"/></a:glow><a:outerShdw blurRad=\"2540000\" dist=\"20116800\" dir=\"2700000\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                              "<a:reflection blurRad=\"2540000\" endPos=\"100000\"/><a:softEdge rad=\"2540000\"/></a:effectLst>");
        QVERIFY(pic->fx.shadow.on && pic->fx.glow.on && pic->fx.reflection.on && pic->fx.softEdge > 0);
        QVERIFY(pic->fx.shadow.blur >= 100 && pic->fx.shadow.distance >= 1000);
        const QImage plain = paintOf(doc, *effectPicture(doc, QRectF(20, 20, 120, 80)), 4, QSize(1400, 1000));
        const QImage shown = paintOf(doc, *pic, 4, QSize(1400, 1000));
        QVERIFY(shown != plain);
        QVERIFY2(Renderer::largestEffectBufferForTest() <= 4000000, qPrintable(QString::number(Renderer::largestEffectBufferForTest())));
        // (Values held in a file of our own are held too.)
        QJsonObject o{{"bevel", QJsonObject{{"type", 99}, {"w", 1e9}, {"h", -5}}}, {"rot3d", QJsonObject{{"x", 1e9}, {"y", -1e9}, {"p", 1e6}}}};
        const Effects fx = Effects::fromJson(o);
        QCOMPARE(fx.bevel.type, 12);
        QCOMPARE(fx.bevel.width, 100.0);
        QCOMPARE(fx.bevel.height, 0.0);
        QCOMPARE(fx.rot3d.x, 180.0);
        QCOMPARE(fx.rot3d.y, -180.0);
        QCOMPARE(fx.rot3d.perspective, 100.0);
    }

    // The bevel is drawn with soft edges and with a turn in space too, not only on a picture with neither.
    void aBevelIsDrawnWithSoftEdgesAndATurnInSpace()
    {
        Document doc;
        for (const bool turned : {false, true}) {
            auto pic = effectPicture(doc, QRectF(60, 50, 120, 80));
            if (turned) pic->fx.rot3d.y = 25;
            else pic->fx.softEdge = 6;
            pic->fx.rot3d.perspective = 30;
            const QImage without = paintOf(doc, *pic);
            pic->fx.bevel.type = 1;
            pic->fx.bevel.width = 8;
            const QImage with = paintOf(doc, *pic);
            QVERIFY2(with != without, turned ? "turn" : "soft edges");
        }
    }

    // Reset Picture on a picture whose size is not known still takes the crop off (the picture fills its frame as the frame is).
    void resetPictureTakesTheCropOffAPictureOfNoKnownSize()
    {
        PictureItem pic;
        pic.imageId = QStringLiteral("x");
        pic.rect = QRectF(10, 10, 100, 50);
        pic.imgRect = QRectF(-20, -10, 160, 80);
        pic.fx.shadow.on = true;
        QVERIFY(resetPictureLook(&pic, QSize(), false));
        QCOMPARE(pic.imgRect, QRectF(0, 0, 100, 50));
        QVERIFY(!pic.fx.any());
    }

    // A glow with no radius draws nothing: it is not read as a glow, and one set with a size of 0 draws as none.
    void aGlowWithNoRadiusDrawsNothing()
    {
        QVERIFY(!effectsRead("<a:effectLst><a:glow><a:srgbClr val=\"FF0000\"/></a:glow></a:effectLst>").glow.on);
        QVERIFY(effectsRead("<a:effectLst><a:glow rad=\"127000\"><a:srgbClr val=\"FF0000\"/></a:glow></a:effectLst>").glow.on);
        Document doc;
        auto pic = effectPicture(doc, QRectF(60, 50, 120, 80));
        const QImage plain = paintOf(doc, *pic);
        pic->fx.glow.on = true;
        pic->fx.glow.size = 0;
        QCOMPARE(paintOf(doc, *pic), plain);
    }

    // A picture's effect images are made once and kept: ten shadowed pictures painted twenty times (a page scrolled) are blurred ten times, not two hundred. The time for the twenty passes is told.
    void thePictureEffectsOfAPageAreMadeOnceForManyPaints()
    {
        Document doc;
        QVector<std::shared_ptr<PictureItem>> pics;
        for (int i = 0; i < 10; ++i) {
            auto pic = effectPicture(doc, QRectF(10 + i * 20, 10 + i * 12, 120, 80));
            pic->fx.shadow.on = true;
            pic->fx.shadow.blur = 12 + i;
            pic->fx.shadow.distance = 6;
            pics << pic;
        }
        auto paints = [&] {
            for (int pass = 0; pass < 20; ++pass) {
                QImage out(400, 300, QImage::Format_ARGB32_Premultiplied);
                out.fill(Qt::white);
                QPainter p(&out);
                p.translate(0, -pass);
                PaintContext ctx;
                ctx.doc = &doc;
                for (const auto &pic : pics) Renderer::paintItem(&p, ctx, *pic);
                p.end();
            }
        };
        Renderer::keepEffectLayersForTest(false);
        QElapsedTimer t;
        t.start();
        paints();
        qInfo("10 shadowed pictures, 20 paints, images made at every paint: %lld ms", qint64(t.elapsed()));
        Renderer::keepEffectLayersForTest(true);
        const qint64 before = Renderer::effectLayersMadeForTest();
        t.start();
        paints();
        qInfo("10 shadowed pictures, 20 paints, images kept: %lld ms", qint64(t.elapsed()));
        QCOMPARE(Renderer::effectLayersMadeForTest() - before, qint64(10));
        // (Changing a picture's look makes its images again.)
        const qint64 made = Renderer::effectLayersMadeForTest();
        pics[0]->fx.shadow.blur = 30;
        paintOf(doc, *pics[0]);
        QVERIFY(Renderer::effectLayersMadeForTest() > made);
        QVERIFY(Renderer::effectLayersMadeForTest() - before <= 12);
    }


    // A text box that is a shape with words in it saves its shape only when it has one: a box saved before the fields existed loads as the plain box it was and paints the same, and
    // one with a shape comes back with its name, adjustments, and points, and is painted in its outline.
    void aTextBoxWithAShapeSavesItAndAPlainOneIsPaintedAsBefore()
    {
        auto paintedBox = [](const TextItem &t) {
            Document doc;
            QImage out(120, 80, QImage::Format_ARGB32_Premultiplied);
            out.fill(Qt::white);
            QPainter p(&out);
            PaintContext ctx;
            ctx.doc = &doc;
            LayoutCache cache;
            ctx.cache = &cache;
            Renderer::paintItem(&p, ctx, t);
            p.end();
            return out;
        };
        TextItem plain;
        plain.rect = QRectF(10, 10, 100, 60);
        plain.fill = Fill::solid(ColorRef::rgb(QColor(200, 0, 0)));
        const QJsonObject json = plain.toJson();
        QVERIFY(!json.contains("shape") && !json.contains("adj") && !json.contains("path"));
        TextItem again;
        again.fromJson(json);
        QVERIFY(again.shape.isEmpty() && again.adj.isEmpty() && again.customPath.isEmpty());
        const QImage before = paintedBox(plain);
        QCOMPARE(paintedBox(again), before);
        QCOMPARE(before.pixelColor(12, 12), QColor(200, 0, 0));
        TextItem oval;
        oval.rect = plain.rect;
        oval.fill = plain.fill;
        oval.shape = QStringLiteral("ellipse");
        TextItem ovalAgain;
        ovalAgain.fromJson(oval.toJson());
        QCOMPARE(ovalAgain.shape, QStringLiteral("ellipse"));
        const QImage shaped = paintedBox(ovalAgain);
        QCOMPARE(shaped.pixelColor(60, 40), QColor(200, 0, 0));
        QCOMPARE(shaped.pixelColor(12, 12), QColor(Qt::white));   // (the corner of the frame is outside the ellipse)
        TextItem freeform;
        freeform.rect = plain.rect;
        freeform.customPath.moveTo(0, 0);
        freeform.customPath.lineTo(100, 0);
        freeform.customPath.lineTo(0, 60);
        freeform.customPath.closeSubpath();
        TextItem freeformAgain;
        freeformAgain.fromJson(freeform.toJson());
        QCOMPARE(freeformAgain.customPath.elementCount(), freeform.customPath.elementCount());
    }

};

JP_TEST_CLASS(DrawSharedTests)
#include "tst_pub_drawshared.moc"
