// Tests of the drawing code the apps share: the points of an outline (Edit Points), where a connector meets an outline, the effects of a shape in DrawingML, and a custom geometry written from
// an outline. Built here from paths and a few lines of XML.

#include "core/pathpoints.h"
#include "office/drawingml_effects.h"
#include "office/drawingml_geometry.h"
#include "render/shapes.h"
#include "pubhelpers.h"

#include <QDomDocument>
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
};

JP_TEST_CLASS(DrawSharedTests)
#include "tst_pub_drawshared.moc"
