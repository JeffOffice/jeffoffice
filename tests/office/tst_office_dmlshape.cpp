// Tests of libs/office: reading DrawingML shapes, groups, pictures, and connectors.
#include "officehelpers.h"

class OfficeDmlShapeTests : public QObject {
    Q_OBJECT


    static QString dmlCxn(const QString &spPr, const QString &cNvCxnSpPr = QString(), const QString &style = QString(), const QString &id = QStringLiteral("9"))
    {
        return QStringLiteral("<p:cxnSp><p:nvCxnSpPr><p:cNvPr id=\"%4\" name=\"Connector %4\"/><p:cNvCxnSpPr>%2</p:cNvCxnSpPr><p:nvPr/></p:nvCxnSpPr>"
                              "<p:spPr>%1</p:spPr>%3</p:cxnSp>").arg(spPr, cNvCxnSpPr, style, id);
    }

private Q_SLOTS:
    void dmlShapeReadsPositionSizeAndTurn()
    {
        DmlEnv env;
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(10, 20, 100, 50, QStringLiteral("rot=\"5400000\" flipH=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s);
        QVERIFY2(dmlRectIs(s->rect, 10, 20, 100, 50), qPrintable(dmlRectText(s->rect)));
        QCOMPARE(s->rotation, 90.0);
        QVERIFY(s->flipH && !s->flipV);
        QCOMPARE(s->shape, QStringLiteral("rect"));
        QCOMPARE(s->name, QStringLiteral("Shape 2"));
        QCOMPARE(s->altText, QStringLiteral("alt 2"));
        QVERIFY(s->storyId.isEmpty());
        QVERIFY(s->wrap.mode == jp::Wrap::None);
        // A turn is read as 0 up to a full one, whichever way it's written; both flips together.
        for (const auto &row : QList<QPair<QString, double>>{{"0", 0}, {"-5400000", 270}, {"21600000", 0}, {"27000000", 90}, {"10800000", 180}, {"1800000", 30}, {"junk", 0}}) {
            it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 10, 10, QStringLiteral("rot=\"%1\" flipH=\"true\" flipV=\"1\"").arg(row.first)) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
            QVERIFY2(near(it->rotation, row.second), qPrintable(row.first));
            QVERIFY(it->flipH && it->flipV);
        }
        // Negative positions are fine, and a shape that says nothing of its place is a point at the origin.
        it = dmlOne(env, dmlSp(dmlXfrm(-30, -40, 5, 6) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QVERIFY(dmlRectIs(it->rect, -30, -40, 5, 6));
        it = dmlOne(env, dmlSp(QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QVERIFY2(it->rect.x() == 0 && it->rect.y() == 0 && it->rect.width() == 0 && it->rect.height() == 0, qPrintable(dmlRectText(it->rect)));
        // Strict files write lengths with units.
        it = dmlOne(env, dmlSp(QStringLiteral("<a:xfrm><a:off x=\"1in\" y=\"2cm\"/><a:ext cx=\"36pt\" cy=\"10mm\"/></a:xfrm><a:prstGeom prst=\"rect\"/>")));
        QVERIFY2(dmlRectIs(it->rect, 72, 72 * 2 / 2.54, 36, 72 * 10 / 25.4, 1e-4), qPrintable(dmlRectText(it->rect)));
    }

    void dmlShapeMapsPresetsAndTheirAdjustments()
    {
        DmlEnv env;
        const auto shape = [&](const QString &geometry, const QString &style = QString()) {
            return dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + geometry + dmlSolid(QStringLiteral("FF0000")), QString(), style));
        };
        auto rr = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst/></a:prstGeom>")));
        QVERIFY(rr);
        QCOMPARE(rr->shape, QStringLiteral("roundRect"));
        QCOMPARE(rr->adj.size(), 1);
        QVERIFY(near(rr->adj[0], 0.16667, 1e-4));
        QVERIFY(rr->customPath.isEmpty());
        auto half = shape(QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 50000\"/></a:avLst></a:prstGeom>"));
        QVERIFY(near(dmlAs<ShapeItem>(half)->adj[0], 0.5));
        auto arrow = shape(QStringLiteral("<a:prstGeom prst=\"rightArrow\"><a:avLst><a:gd name=\"adj1\" fmla=\"val 30000\"/><a:gd name=\"adj2\" fmla=\"val 60000\"/></a:avLst></a:prstGeom>"));
        QCOMPARE(dmlAs<ShapeItem>(arrow)->shape, QStringLiteral("rightArrow"));
        QVERIFY(near(dmlAs<ShapeItem>(arrow)->adj[0], 0.3) && near(dmlAs<ShapeItem>(arrow)->adj[1], 0.6));
        auto flow = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"flowChartDecision\"/>")));
        QCOMPARE(flow->shape, QStringLiteral("fcDecision"));
        QVERIFY(flow->adj.isEmpty());
        // The standard's defaults apply when it gives none, though the library's differ.
        auto star = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"star24\"/>")));
        QCOMPARE(star->shape, QStringLiteral("star24"));
        QVERIFY(near(star->adj[0], 0.75));
        // Geometry the library draws wrongly or not at all comes as the path the standard defines.
        auto pie = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"pie\"/>")));
        QCOMPARE(pie->shape, QStringLiteral("pie"));
        QVERIFY(!pie->customPath.isEmpty());
        QVERIFY(pie->customPath.contains(QPointF(25, 12.5)));
        QVERIFY(!pie->customPath.contains(QPointF(75, 12.5)));
        auto wedge = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"wedgeRectCallout\"/>")));
        QVERIFY(!wedge->customPath.isEmpty());
        QVERIFY(wedge->customPath.boundingRect().bottom() > 50);
        auto trap = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"nonIsoscelesTrapezoid\"/>")));
        QVERIFY(!trap->customPath.isEmpty());
        // An arc is its curve, and with a fill its sector too, drawn as shapes whose names don't say "no fill".
        auto arcItem = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"arc\"/>") + dmlSolid(QStringLiteral("FF0000")) +
                                         QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        auto arcGroup = dmlAs<GroupItem>(arcItem);
        QVERIFY(arcGroup && arcGroup->children.size() == 2);
        QVERIFY(dmlAs<ShapeItem>(arcGroup->children[0])->shape != QLatin1String("arc") && !dmlAs<ShapeItem>(arcGroup->children[0])->fill.isNone());
        QVERIFY(dmlAs<ShapeItem>(arcGroup->children[1])->fill.isNone() && !dmlAs<ShapeItem>(arcGroup->children[1])->stroke.isNone());
        auto arcOnly = dmlAs<ShapeItem>(dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"arc\"/><a:noFill/><a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000"))))));
        QVERIFY(arcOnly && arcOnly->fill.isNone() && !arcOnly->customPath.isEmpty());
        // A preset nobody draws is a rectangle, and so is one that isn't one.
        auto gear = dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"gear6\"/>")));
        QCOMPARE(gear->shape, QStringLiteral("rect"));
        QVERIFY(gear->customPath.isEmpty());
        QCOMPARE(dmlAs<ShapeItem>(shape(QStringLiteral("<a:prstGeom prst=\"noSuchThing\"/>")))->shape, QStringLiteral("rect"));
        QCOMPARE(dmlAs<ShapeItem>(shape(QString()))->shape, QStringLiteral("rect"));
        // Fill and outline.
        QCOMPARE(dmlHex(rr->fill.color), QStringLiteral("FF0000"));
        QVERIFY(rr->stroke.isNone());
    }

    void dmlShapeReadsCustomGeometry()
    {
        DmlEnv env;
        const QString triangle = QStringLiteral("<a:custGeom><a:avLst/><a:gdLst/><a:ahLst/><a:cxnLst/><a:rect l=\"0\" t=\"0\" r=\"r\" b=\"b\"/><a:pathLst>"
                                                "<a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"100\"/></a:moveTo><a:lnTo><a:pt x=\"50\" y=\"0\"/></a:lnTo>"
                                                "<a:lnTo><a:pt x=\"100\" y=\"100\"/></a:lnTo><a:close/></a:path></a:pathLst></a:custGeom>");
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 200, 100) + triangle + dmlSolid(QStringLiteral("00FF00")) + QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s);
        QVERIFY2(dmlRectIs(s->customPath.boundingRect(), 0, 0, 200, 100), qPrintable(dmlRectText(s->customPath.boundingRect())));
        QVERIFY(s->customPath.contains(QPointF(100, 90)));
        QVERIFY(!s->customPath.contains(QPointF(5, 5)));
        QCOMPARE(dmlHex(s->fill.color), QStringLiteral("00FF00"));
        QVERIFY(!s->stroke.isNone());
        // Paths drawn without fill, or without outline, are their own objects so that each is drawn as it says.
        const QString two = QStringLiteral("<a:custGeom><a:pathLst><a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:lnTo><a:pt x=\"10\" y=\"0\"/></a:lnTo>"
                                           "<a:lnTo><a:pt x=\"10\" y=\"10\"/></a:lnTo><a:close/></a:path>"
                                           "<a:path w=\"10\" h=\"10\" fill=\"none\"><a:moveTo><a:pt x=\"0\" y=\"10\"/></a:moveTo><a:lnTo><a:pt x=\"10\" y=\"10\"/></a:lnTo></a:path></a:pathLst></a:custGeom>");
        const QString looks = dmlSolid(QStringLiteral("00FF00")) + QStringLiteral("<a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")));
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 100, QStringLiteral("rot=\"900000\"")) + two + looks, dmlBody()));
        auto g = dmlAs<GroupItem>(it);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 2);
        auto first = dmlAs<ShapeItem>(g->children[0]);
        auto second = dmlAs<ShapeItem>(g->children[1]);
        QVERIFY(first && second);
        QVERIFY(!first->fill.isNone() && !first->stroke.isNone());
        QVERIFY(second->fill.isNone() && !second->stroke.isNone());
        QVERIFY(dmlRectIs(first->rect, 10, 10, 100, 100) && dmlRectIs(second->rect, 10, 10, 100, 100));
        QCOMPARE(first->rotation, 15.0);
        QCOMPARE(second->rotation, 15.0);
        QVERIFY(!first->storyId.isEmpty() && second->storyId.isEmpty());   // the text is in the first
        QCOMPARE(g->name, QStringLiteral("Shape 2"));
        // With no fill on the shape, the path that is only for fill has nothing to do.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 100) + two + QStringLiteral("<a:noFill/><a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        QVERIFY(dmlAs<ShapeItem>(it));
        // A callout is its box and its leader line.
        it = dmlOne(env, dmlSp(dmlXfrm(100, 100, 200, 100) + QStringLiteral("<a:prstGeom prst=\"borderCallout1\"/>") + looks));
        g = dmlAs<GroupItem>(it);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 2);
        // A line callout with no outline draws only the fill.
        it = dmlOne(env, dmlSp(dmlXfrm(100, 100, 200, 100) + QStringLiteral("<a:prstGeom prst=\"callout1\"/>") + dmlSolid(QStringLiteral("00FF00")) + QStringLiteral("<a:ln><a:noFill/></a:ln>")));
        QVERIFY(dmlAs<ShapeItem>(it));
        QVERIFY(dmlAs<ShapeItem>(it)->stroke.isNone());
    }

    void dmlShapeBecomesATextBoxOrAShapeWithText()
    {
        DmlEnv env;
        const QString rect = dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        // A text box.
        ItemPtr it = dmlOne(env, dmlSp(rect, dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        auto t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QVERIFY(!t->storyId.isEmpty());
        QVERIFY(env.stories[0]->toPlainText().contains(QStringLiteral("Hello")));
        QVERIFY(t->fill.isNone() && t->stroke.isNone());
        QVERIFY(!t->hyphenate);
        QVERIFY(t->wrap.mode == jp::Wrap::None);
        // An empty one is a text box all the same, with a story to type into.
        it = dmlOne(env, dmlSp(rect, QStringLiteral("<p:txBody><a:bodyPr/><a:p/></p:txBody>"), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<TextItem>(it) && !dmlAs<TextItem>(it)->storyId.isEmpty());
        it = dmlOne(env, dmlSp(rect, QString(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<TextItem>(it));
        // A placeholder, with its fill and line.
        it = dmlOne(env, dmlSp(rect + dmlSolid(QStringLiteral("123456")) + QStringLiteral("<a:ln w=\"25400\">%1</a:ln>").arg(dmlSolid(QStringLiteral("654321"))), dmlBody(),
                               QString(), QString(), QStringLiteral("<p:ph type=\"title\"/>")));
        t = dmlAs<TextItem>(it);
        QVERIFY(t);
        QCOMPARE(dmlHex(t->fill.color), QStringLiteral("123456"));
        QVERIFY(near(t->stroke.width, 2));
        // A placeholder with nothing in it still is one (and has no geometry of its own to say).
        it = dmlOne(env, dmlSp(QString(), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>")));
        QVERIFY(dmlAs<TextItem>(it));
        // A rectangle with text, and one without.
        it = dmlOne(env, dmlSp(rect + dmlSolid(QStringLiteral("FF0000")), dmlBody()));
        QVERIFY(dmlAs<TextItem>(it));
        QCOMPARE(dmlHex(it->fill.color), QStringLiteral("FF0000"));
        it = dmlOne(env, dmlSp(rect, QStringLiteral("<p:txBody><a:bodyPr/><a:p/></p:txBody>")));
        QVERIFY(dmlAs<ShapeItem>(it) && dmlAs<ShapeItem>(it)->storyId.isEmpty());
        // Text in another shape stays in the shape.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"roundRect\"/>"), dmlBody(QStringLiteral("Round"))));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s && !s->storyId.isEmpty());
        QVERIFY(env.stories.last()->toPlainText().contains(QStringLiteral("Round")));
        QCOMPARE(s->shape, QStringLiteral("roundRect"));
        // A text box that isn't a rectangle is a shape with text, and one in a shape that is not a text box is not.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->storyId.isEmpty());
        // An empty picture placeholder is a picture with no picture.
        it = dmlOne(env, dmlSp(QString(), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"pic\" idx=\"13\"/>")));
        auto p = dmlAs<PictureItem>(it);
        QVERIFY(p && p->imageId.isEmpty());
        // Without a way to make stories, a text box has none and is no worse for it.
        DmlEnv bare;
        bare.ctx.newStory = nullptr;
        it = dmlOne(bare, dmlSp(rect, dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(dmlAs<TextItem>(it) && dmlAs<TextItem>(it)->storyId.isEmpty());
    }

    void dmlShapeReadsTextBoxProperties()
    {
        DmlEnv env;
        const QString rect = dmlXfrm(10, 10, 100, 40) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        const auto box = [&](const QString &bodyPr, const QString &geometry = QString(), const QVector<QString> &inherits = {}) {
            const QString g = geometry.isEmpty() ? rect : dmlXfrm(10, 10, 100, 40) + geometry;
            return dmlOne(env, dmlSp(g, dmlBody(QStringLiteral("x"), bodyPr), QString(), QStringLiteral("txBox=\"1\""), QString()), inherits);
        };
        // The standard's insets and top anchor, which a shape or a text box has with nothing said.
        auto t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr/>")));
        QVERIFY(t);
        QCOMPARE(t->insets.left(), 7.2);
        QCOMPARE(t->insets.top(), 3.6);
        QCOMPARE(t->insets.right(), 7.2);
        QCOMPARE(t->insets.bottom(), 3.6);
        QVERIFY(t->valign == VAlign::Top);
        QVERIFY(t->autofit == TextItem::NoAutofit);
        QCOMPARE(t->columns, 1);
        QVERIFY(!t->vertical);
        auto s = dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")));
        QVERIFY(s);
        QCOMPARE(s->insets.left(), 7.2);
        QCOMPARE(s->insets.top(), 3.6);
        QVERIFY(s->valign == VAlign::Top);
        // Each said inset, including none.
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr lIns=\"0\" tIns=\"12700\" rIns=\"25400\" bIns=\"38100\"/>")));
        QCOMPARE(t->insets.left(), 0.0);
        QCOMPARE(t->insets.top(), 1.0);
        QCOMPARE(t->insets.right(), 2.0);
        QCOMPARE(t->insets.bottom(), 3.0);
        s = dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr lIns=\"12700\" bIns=\"0\"/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")));
        QCOMPARE(s->insets.left(), 1.0);
        QCOMPARE(s->insets.bottom(), 0.0);
        QCOMPARE(s->insets.top(), 3.6);
        // Anchors.
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"ctr\"/>")))->valign == VAlign::Middle);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"b\"/>")))->valign == VAlign::Bottom);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"t\"/>")))->valign == VAlign::Top);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"just\"/>")))->valign == VAlign::Top);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"nonsense\"/>")))->valign == VAlign::Top);
        QVERIFY(dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr anchor=\"ctr\"/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")))->valign == VAlign::Middle);
        QVERIFY(dmlAs<ShapeItem>(box(QStringLiteral("<a:bodyPr anchor=\"b\"/>"), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>")))->valign == VAlign::Bottom);
        // Fitting the text to the box.
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"62500\" lnSpcReduction=\"20000\"/></a:bodyPr>")));
        QVERIFY(t->autofit == TextItem::ShrinkOnOverflow);
        QVERIFY(near(t->fitScale, 0.625));
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit/></a:bodyPr>")));
        QVERIFY(t->autofit == TextItem::ShrinkOnOverflow);
        QCOMPARE(t->fitScale, 1.0);
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"5\"/></a:bodyPr>")));
        QVERIFY(t->fitScale > 0 && t->fitScale <= 1);
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"500000\"/></a:bodyPr>")));
        QVERIFY(t->fitScale > 0 && t->fitScale <= 1);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:spAutoFit/></a:bodyPr>")))->autofit == TextItem::GrowBox);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr><a:noAutofit/></a:bodyPr>")))->autofit == TextItem::NoAutofit);
        // Columns, and the way the text runs.
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"3\" spcCol=\"228600\"/>")));
        QCOMPARE(t->columns, 3);
        QCOMPARE(t->columnGap, 18.0);
        QCOMPARE(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"99999\"/>")))->columns <= 16, true);
        QCOMPARE(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"0\"/>")))->columns, 1);
        QCOMPARE(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr numCol=\"-4\"/>")))->columns, 1);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr vert=\"vert\"/>")))->vertical);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr vert=\"vert270\"/>")))->vertical);
        QVERIFY(!dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr vert=\"horz\"/>")))->vertical);
        QVERIFY(dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr wrap=\"none\"/>"))));
        // What a placeholder's own body leaves out comes from the layout's, and then from the master's.
        const QString layout = dmlSp(dmlXfrm(1, 2, 3, 4), QStringLiteral("<p:txBody><a:bodyPr lIns=\"0\" anchor=\"ctr\"><a:normAutofit/></a:bodyPr><a:p/></p:txBody>"));
        const QString master = dmlSp(dmlXfrm(1, 2, 3, 4), QStringLiteral("<p:txBody><a:bodyPr tIns=\"25400\" anchor=\"b\" rIns=\"12700\"><a:spAutoFit/></a:bodyPr><a:p/></p:txBody>"));
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr rIns=\"38100\"/>"), QString(), {layout, master}));
        QCOMPARE(t->insets.left(), 0.0);        // the layout's
        QCOMPARE(t->insets.top(), 2.0);         // the master's, which the layout lacks
        QCOMPARE(t->insets.right(), 3.0);       // its own
        QCOMPARE(t->insets.bottom(), 3.6);      // nobody's
        QVERIFY(t->valign == VAlign::Middle);   // the layout's, over the master's
        QVERIFY(t->autofit == TextItem::ShrinkOnOverflow);
        t = dmlAs<TextItem>(box(QStringLiteral("<a:bodyPr anchor=\"t\"><a:noAutofit/></a:bodyPr>"), QString(), {layout, master}));
        QVERIFY(t->valign == VAlign::Top);
        QVERIFY(t->autofit == TextItem::NoAutofit);
    }

    void dmlShapeTakesWhatItInheritsFrom()
    {
        DmlEnv env;
        // A placeholder that says nothing is placed, shaped, filled, and lined as the layout's, else the master's.
        const QString layout = dmlSp(dmlXfrm(50, 60, 300, 100, QStringLiteral("rot=\"600000\"")) + dmlSolid(QStringLiteral("00FF00")),
                                     QString(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>"));
        const QString master = dmlSp(dmlXfrm(1, 2, 3, 4) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + dmlSolid(QStringLiteral("0000FF")) +
                                     QStringLiteral("<a:ln w=\"25400\">%1</a:ln>").arg(dmlSolid(QStringLiteral("FF00FF"))), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>"));
        ItemPtr it = dmlOne(env, dmlSp(QString(), dmlBody(), QString(), QString(), QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>")), {layout, master});
        auto t = dmlAs<TextItem>(it);
        QVERIFY(t);   // a placeholder is a text frame
        QVERIFY2(dmlRectIs(t->rect, 50, 60, 300, 100), qPrintable(dmlRectText(t->rect)));
        QVERIFY(near(t->rotation, 10));
        QCOMPARE(dmlHex(t->fill.color), QStringLiteral("00FF00"));       // the layout's fill
        QVERIFY(near(t->stroke.width, 2));                                // and the master's line
        QCOMPARE(dmlHex(t->stroke.color), QStringLiteral("FF00FF"));
        // Said itself, it's its own.
        it = dmlOne(env, dmlSp(dmlXfrm(5, 6, 7, 8) + dmlSolid(QStringLiteral("123456")) + QStringLiteral("<a:ln><a:noFill/></a:ln>"), dmlBody(), QString(), QString(),
                               QStringLiteral("<p:ph type=\"body\" idx=\"1\"/>")), {layout, master});
        QVERIFY(dmlRectIs(it->rect, 5, 6, 7, 8));
        QCOMPARE(dmlHex(it->fill.color), QStringLiteral("123456"));
        QVERIFY(it->stroke.isNone());
        // A shape inheriting a preset takes its shape, and its adjustments.
        const QString shaped = dmlSp(dmlXfrm(50, 60, 300, 100) + QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 40000\"/></a:avLst></a:prstGeom>"));
        it = dmlOne(env, dmlSp(QString(), dmlBody(QStringLiteral("x")), QString(), QString(), QStringLiteral("<p:ph type=\"title\"/>")), {shaped});
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->storyId.isEmpty());   // a placeholder that isn't a rectangle holds its text as a shape does
        it = dmlOne(env, dmlSp(QString(), QString(), QString(), QString(), QString()), {shaped});
        QCOMPARE(dmlAs<ShapeItem>(it)->shape, QStringLiteral("roundRect"));
        QVERIFY(near(dmlAs<ShapeItem>(it)->adj[0], 0.4));
        // A picture placeholder takes the position of the layout's too.
        const QString pic = dmlSp(dmlXfrm(11, 12, 130, 140), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"pic\" idx=\"13\"/>"));
        it = dmlOne(env, QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"4\" name=\"P\"/><p:cNvPicPr/><p:nvPr><p:ph type=\"pic\" idx=\"13\"/></p:nvPr></p:nvPicPr>"
                                        "<p:blipFill><a:blip/></p:blipFill><p:spPr/></p:pic>"), {pic});
        QVERIFY(dmlAs<PictureItem>(it));
        QVERIFY(dmlRectIs(it->rect, 11, 12, 130, 140));
    }

    void dmlShapeTakesItsStyle()
    {
        DmlEnv env;
        env.useFormats();
        const QString style = dmlStyle(QStringLiteral("2"), QStringLiteral("1"), QStringLiteral("3"));
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), QString(), style));
        auto s = dmlAs<ShapeItem>(it);
        QCOMPARE(dmlHex(s->fill.color), QStringLiteral("4472C4"));
        QCOMPARE(dmlHex(s->stroke.color), QStringLiteral("4472C4"));
        QVERIFY(near(s->stroke.width, 1));
        QVERIFY(s->stroke.dash == Stroke::DashLine);
        // A shadow from effect style 3 (a 4.5 point blur, 1.5 points down, 63% opaque black).
        QVERIFY(s->fx.shadow.on);
        QVERIFY(near(s->fx.shadow.blur, 4.5, 1e-6) && near(s->fx.shadow.distance, 1.5, 1e-6) && near(s->fx.shadow.angle, 90));
        QVERIFY(near(s->fx.shadow.transparency, 0.37, 2e-2));
        // The shape's own say overrides it all; an empty effect list takes the shadow away.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:noFill/><a:ln><a:noFill/></a:ln><a:effectLst/>"), QString(), style));
        QVERIFY(it->fill.isNone() && it->stroke.isNone() && !it->fx.shadow.on);
        // A text box with no style has no fill or line, as it says.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), dmlBody(), QString(), QStringLiteral("txBox=\"1\"")));
        QVERIFY(it->fill.isNone() && it->stroke.isNone());
    }

    // A shape that says its text shrinks to fit (a:normAutofit) keeps the scale and the line spacing taken off, as a text box does; one that says no
    // fit, or none, doesn't shrink.
    void dmlShapeReadsNormAutofit()
    {
        DmlEnv env;
        auto shape = [&](const QString &prst, const QString &bodyPr) {
            return dmlAs<ShapeItem>(dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 40) + QStringLiteral("<a:prstGeom prst=\"%1\"/>").arg(prst), dmlBody(QStringLiteral("Hello"), bodyPr))));
        };
        auto round = shape(QStringLiteral("roundRect"), QStringLiteral("<a:bodyPr><a:normAutofit fontScale=\"62500\" lnSpcReduction=\"20000\"/></a:bodyPr>"));
        QVERIFY(round);
        QVERIFY(round->shrinks);
        QVERIFY(near(round->fitScale, 0.625, 1e-6) && near(round->fitLineCut, 20, 1e-6));
        auto oval = shape(QStringLiteral("ellipse"), QStringLiteral("<a:bodyPr><a:normAutofit/></a:bodyPr>"));
        QVERIFY(oval && oval->shrinks && near(oval->fitScale, 1.0, 1e-6) && near(oval->fitLineCut, 0, 1e-6));
        auto none = shape(QStringLiteral("roundRect"), QStringLiteral("<a:bodyPr><a:noAutofit/></a:bodyPr>"));
        QVERIFY(none && !none->shrinks);
        auto plain = shape(QStringLiteral("roundRect"), QStringLiteral("<a:bodyPr/>"));
        QVERIFY(plain && !plain->shrinks && near(plain->fitScale, 1.0, 1e-6));
    }

    // The words kept with a Macintosh picture's PNG are no more than the reader takes: a picture of as many words as the writer allows, each as long as
    // it allows, has its words cut to what fits in the reader's limit (a megabyte) instead of being dropped whole, and they come back through the PNG.
    void drawnWordsFitTheLimitOfTheReader()
    {
        QVector<jp::DrawnWord> many;
        for (int i = 0; i < jp::kMaxDrawnWords; ++i)
            many.push_back({QString(jp::kMaxDrawnWordChars, QChar(0x00E9)) + QString::number(i), QStringLiteral("Arimo"), QPointF(0.1, 0.2), 0.05, i % 2 == 0, false});
        const QString text = jp::drawnWordsToText(many);
        QVERIFY(!text.isEmpty());
        QVERIFY2(text.toUtf8().size() <= (1 << 20), qPrintable(QString::number(text.toUtf8().size())));
        const QVector<jp::DrawnWord> back = jp::drawnWordsFromText(text);
        QVERIFY2(back.size() > 100, qPrintable(QString::number(back.size())));
        QCOMPARE(back.first().text, many.first().text.left(jp::kMaxDrawnWordChars));
        // Through a PNG's own text.
        QImage img(8, 8, QImage::Format_ARGB32);
        img.fill(Qt::white);
        img.setText(QString::fromLatin1(jp::kDrawnWordsKey), text);
        QByteArray png;
        QBuffer buf(&png);
        QVERIFY(buf.open(QIODevice::WriteOnly) && img.save(&buf, "PNG"));
        QCOMPARE(QImage::fromData(png).text(QString::fromLatin1(jp::kDrawnWordsKey)), text);
        QCOMPARE(int(jp::drawnWordsFromText(QImage::fromData(png).text(QString::fromLatin1(jp::kDrawnWordsKey))).size()), int(back.size()));
    }

    void dmlTextTakesTheStylesFontReference()
    {
        DmlEnv env;
        const QString a(ns::A);
        const QDomElement base = env.element(QStringLiteral("<a:lstStyle><a:lvl1pPr><a:defRPr sz=\"1800\"/></a:lvl1pPr></a:lstStyle>"));
        const QDomElement other = env.element(QStringLiteral("<a:lstStyle><a:lvl1pPr algn=\"ctr\"/></a:lstStyle>"));
        const QDomElement style = env.element(dmlStyle(QStringLiteral("1"), QStringLiteral("1")));
        QDomDocument keep;
        // As one more list style, above the others: the font reference's color (light 1 here) at every level; and the theme's body font as one just above
        // the default text style (the first), under every other, so that what a placeholder inherits is not overridden.
        auto bodyFont = [&](const QDomElement &list) { return attr(child(child(child(list, a, QStringLiteral("lvl1pPr")), a, QStringLiteral("defRPr")), a, QStringLiteral("latin")), QStringLiteral("typeface")); };
        TextStyleStack stack = withFontReference({base}, style, QDomElement(), &keep);
        QCOMPARE(int(stack.size()), 3);
        QVERIFY(stack[0] == base);
        QCOMPARE(bodyFont(stack[1]), QStringLiteral("+mn-lt"));
        for (int level = 1; level <= 9; ++level) {
            const QDomElement run = child(child(stack[2], a, QStringLiteral("lvl%1pPr").arg(level)), a, QStringLiteral("defRPr"));
            QVERIFY(!run.isNull());
            QCOMPARE(attr(child(child(run, a, QStringLiteral("solidFill")), a, QStringLiteral("schemeClr")), QStringLiteral("val")), QStringLiteral("lt1"));
            QVERIFY(child(run, a, QStringLiteral("latin")).isNull());
        }
        // The color below the shape's own list, when the stack ends in it.
        stack = withFontReference({other, base}, style, base, &keep);
        QCOMPARE(int(stack.size()), 4);
        QVERIFY(stack[0] == other && stack[3] == base);
        QCOMPARE(bodyFont(stack[1]), QStringLiteral("+mn-lt"));
        QVERIFY(!child(stack[2], a, QStringLiteral("lvl3pPr")).isNull());
        stack = withFontReference({base, other}, style, base, &keep);
        QVERIFY(stack.last() != other && stack.size() == 4);
        // The heading font, with or without a color; and the body font the same, over the default text style's own font; no reference, nothing to say,
        // no change.
        const QDomElement major = env.element(QStringLiteral("<p:style><a:fontRef idx=\"major\"/></p:style>"));
        stack = withFontReference({base}, major, QDomElement(), &keep);
        QCOMPARE(int(stack.size()), 2);
        const QDomElement run = child(child(stack[1], a, QStringLiteral("lvl1pPr")), a, QStringLiteral("defRPr"));
        QCOMPARE(attr(child(run, a, QStringLiteral("latin")), QStringLiteral("typeface")), QStringLiteral("+mj-lt"));
        QVERIFY(child(run, a, QStringLiteral("solidFill")).isNull());
        const QDomElement minorOnly = env.element(QStringLiteral("<p:style><a:fontRef idx=\"minor\"/></p:style>"));
        stack = withFontReference({base}, minorOnly, QDomElement(), &keep);
        QCOMPARE(int(stack.size()), 2);
        QCOMPARE(bodyFont(stack[1]), QStringLiteral("+mn-lt"));
        QVERIFY(child(child(child(stack[1], a, QStringLiteral("lvl1pPr")), a, QStringLiteral("defRPr")), a, QStringLiteral("solidFill")).isNull());
        // Under what a placeholder inherits: the stack of a default style, a master's style, and a layout's, and its own, has the body font second.
        stack = withFontReference({base, other, base}, minorOnly, other, &keep);
        QCOMPARE(int(stack.size()), 4);
        QVERIFY(stack[0] == base && stack[2] == other);
        QCOMPARE(bodyFont(stack[1]), QStringLiteral("+mn-lt"));

        QCOMPARE(int(withFontReference({base}, env.element(QStringLiteral("<p:style/>")), QDomElement(), &keep).size()), 1);
        QCOMPARE(int(withFontReference({base}, QDomElement(), QDomElement(), &keep).size()), 1);
        QCOMPARE(int(withFontReference({}, style, QDomElement(), &keep).size()), 2);   // (the body font, and the color)
        // A shape that has the style reads its text with it.
        const QString rect = dmlXfrm(0, 0, 100, 40) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        env.useFormats();
        const ItemPtr it = dmlOne(env, dmlSp(rect, dmlBody(), dmlStyle(QStringLiteral("1"), QStringLiteral("1"))));
        QVERIFY(dmlAs<TextItem>(it));
        QVERIFY(env.stories[0]->toPlainText().contains(QStringLiteral("Hello")));
    }

    void dmlShapeReadsEffects()
    {
        DmlEnv env;
        const QString shadow = QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"50800\" dist=\"38100\" dir=\"2700000\" algn=\"tl\" rotWithShape=\"0\"><a:srgbClr val=\"FF0000\"><a:alpha val=\"40000\"/></a:srgbClr></a:outerShdw>"
                                              "<a:glow rad=\"101600\"><a:srgbClr val=\"00FF00\"><a:alpha val=\"60000\"/></a:srgbClr></a:glow><a:softEdge rad=\"63500\"/><a:reflection/></a:effectLst>");
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + shadow));
        QVERIFY(it->fx.shadow.on && !it->fx.shadow.inner);
        QVERIFY(near(it->fx.shadow.blur, 4) && near(it->fx.shadow.distance, 3) && near(it->fx.shadow.angle, 45));
        QCOMPARE(dmlHex(it->fx.shadow.color), QStringLiteral("FF0000"));
        QVERIFY(near(it->fx.shadow.transparency, 0.6, 2e-2));
        QVERIFY(it->fx.glow.on && near(it->fx.glow.size, 8) && near(it->fx.glow.transparency, 0.4, 2e-2));
        QCOMPARE(dmlHex(it->fx.glow.color), QStringLiteral("00FF00"));
        QVERIFY(near(it->fx.softEdge, 5));
        QVERIFY(!it->fx.reflection.on);
        // An inner shadow, when it's the only one.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:effectLst><a:innerShdw blurRad=\"12700\" dist=\"12700\" dir=\"0\"><a:srgbClr val=\"000000\"/></a:innerShdw></a:effectLst>")));
        QVERIFY(it->fx.shadow.on && it->fx.shadow.inner);
        // Hostile numbers are held to what a page can show.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:effectLst><a:outerShdw blurRad=\"99999999999999\" dist=\"-5\" dir=\"junk\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                                                       "<a:softEdge rad=\"99999999999999\"/></a:effectLst>")));
        QVERIFY(it->fx.shadow.blur <= 1584 && it->fx.shadow.distance >= 0 && it->fx.softEdge <= 1584);
        QVERIFY(std::isfinite(it->fx.shadow.angle));
        // None is none.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QVERIFY(!it->fx.any());
    }

    void dmlShapeScalesAGradientAngleToItsShape()
    {
        DmlEnv env;
        const QString grad = QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"0000FF\"/></a:gs></a:gsLst>"
                                            "<a:lin ang=\"2700000\" scaled=\"%1\"/></a:gradFill>");
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 400, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + grad.arg(QStringLiteral("1"))));
        QVERIFY2(near(it->fill.angle, std::atan2(1.0 / 100, 1.0 / 400) * 180 / M_PI, 1e-6), qPrintable(QString::number(it->fill.angle)));
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 400, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + grad.arg(QStringLiteral("0"))));
        QVERIFY(near(it->fill.angle, 45));
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:prstGeom prst=\"rect\"/>") + grad.arg(QStringLiteral("1"))));
        QVERIFY(near(it->fill.angle, 45, 1e-6));
    }

    void dmlGroupMapsItsChildrenIntoItsFrame()
    {
        DmlEnv env;
        const auto rect = [](double x, double y, double w, double h, const QString &attrs = QString(), const QString &id = QStringLiteral("2")) {
            return dmlSp(dmlXfrm(x, y, w, h, attrs) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), QString(), QString(), QString(), QString(), QString(), id);
        };
        // Twice the size, and moved: the child space is 100 by 50, the frame 200 by 100.
        ItemList list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50)), rect(10, 10, 20, 10)));
        QCOMPARE(int(list.size()), 1);
        auto g = dmlAs<GroupItem>(list[0]);
        QVERIFY(g);
        QCOMPARE(int(g->children.size()), 1);
        QVERIFY2(dmlRectIs(g->children[0]->rect, 120, 120, 40, 20), qPrintable(dmlRectText(g->children[0]->rect)));
        QVERIFY2(dmlRectIs(g->rect, 120, 120, 40, 20), qPrintable(dmlRectText(g->rect)));
        QCOMPARE(g->name, QStringLiteral("Group 20"));
        QCOMPARE(g->altText, QStringLiteral("group alt"));
        // The child space doesn't start at the origin.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(50, 50, 50, 50)), rect(60, 70, 10, 10)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 20, 40, 20, 20), qPrintable(dmlRectText(dmlAs<GroupItem>(list[0])->children[0]->rect)));
        // A different scale on each side, and a child turned a quarter: its width follows the other scale.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 100), QRectF(0, 0, 100, 100)), rect(10, 10, 20, 20, QStringLiteral("rot=\"5400000\""))));
        const ItemPtr turned = dmlAs<GroupItem>(list[0])->children[0];
        QVERIFY2(dmlRectIs(turned->rect, 30, 0, 20, 40, 1e-6), qPrintable(dmlRectText(turned->rect)));
        QVERIFY(near(turned->rotation, 90));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 100), QRectF(0, 0, 100, 100)), rect(10, 10, 20, 10)));
        QVERIFY(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 20, 10, 40, 10));
        // The group turned a quarter about the middle of its frame: its children go round with it.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("rot=\"5400000\"")), rect(10, 10, 20, 10)));
        const ItemPtr round = dmlAs<GroupItem>(list[0])->children[0];
        QVERIFY2(dmlRectIs(round->rect, 200, 80, 40, 20, 1e-6), qPrintable(dmlRectText(round->rect)));
        QVERIFY(near(round->rotation, 90));
        // Turned itself, a child's turn adds to the group's.
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("rot=\"5400000\"")),
                                    rect(10, 10, 20, 10, QStringLiteral("rot=\"1800000\""))));
        QVERIFY(near(dmlAs<GroupItem>(list[0])->children[0]->rotation, 120));
        // Flipped, a child goes to the other side, and is flipped too (its own turn reverses).
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("flipH=\"1\"")),
                                    rect(10, 10, 20, 10) + rect(10, 10, 20, 10, QStringLiteral("rot=\"1800000\""))));
        g = dmlAs<GroupItem>(list[0]);
        QVERIFY2(dmlRectIs(g->children[0]->rect, 240, 120, 40, 20), qPrintable(dmlRectText(g->children[0]->rect)));
        QVERIFY(g->children[0]->flipH && !g->children[0]->flipV);
        QVERIFY(g->children[1]->flipH);
        QVERIFY(near(g->children[1]->rotation, 330));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("flipV=\"1\"")), rect(10, 10, 20, 10)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 120, 160, 40, 20), qPrintable(dmlRectText(dmlAs<GroupItem>(list[0])->children[0]->rect)));
        QVERIFY(dmlAs<GroupItem>(list[0])->children[0]->flipV);
        // Groups in groups: each maps into the next.
        const QString inner = dmlGrp(dmlGroupFrame(QRectF(10, 10, 20, 20), QRectF(0, 0, 20, 20)), rect(0, 0, 10, 10), QStringLiteral("21"));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 200), QRectF(0, 0, 100, 100)), inner));
        auto outer = dmlAs<GroupItem>(list[0]);
        QVERIFY(outer);
        QVERIFY2(dmlRectIs(outer->children[0]->rect, 20, 20, 20, 20), qPrintable(dmlRectText(outer->children[0]->rect)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(outer->children[0])->children[0]->rect, 20, 20, 20, 20), qPrintable(dmlRectText(outer->children[0]->rect)));
        const QString innerTurned = dmlGrp(dmlGroupFrame(QRectF(10, 10, 20, 20), QRectF(0, 0, 20, 20), QStringLiteral("rot=\"5400000\"")), rect(0, 0, 10, 10), QStringLiteral("21"));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 200, 200), QRectF(0, 0, 100, 100)), innerTurned));
        outer = dmlAs<GroupItem>(list[0]);
        QVERIFY2(dmlRectIs(outer->children[0]->rect, 40, 20, 20, 20, 1e-6), qPrintable(dmlRectText(outer->children[0]->rect)));
        QVERIFY(near(dmlAs<GroupItem>(outer->children[0])->children[0]->rotation, 90));
        // A group that says nothing of its frame leaves what is in it where it is; one with no child space is not scaled.
        list = dmlTree(env, QStringLiteral("<p:grpSp><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"g\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>") + rect(10, 20, 30, 40) + QStringLiteral("</p:grpSp>"));
        QVERIFY(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 10, 20, 30, 40));
        list = dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(10, 10, 100, 100), QRectF(0, 0, 0, 0)), rect(5, 5, 20, 20)));
        QVERIFY2(dmlRectIs(dmlAs<GroupItem>(list[0])->children[0]->rect, 15, 15, 20, 20), qPrintable(dmlRectText(dmlAs<GroupItem>(list[0])->children[0]->rect)));
        // No children, no group.
        QVERIFY(dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10)), QString())).empty());
        QVERIFY(!dmlOne(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10)), QString())));
        // A group is a shape too.
        QVERIFY(dmlAs<GroupItem>(dmlOne(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 10, 10), QRectF(0, 0, 10, 10)), rect(0, 0, 5, 5)))));
    }

    void dmlGroupScalesWhatIsInIt()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50));
        const QString triangle = QStringLiteral("<a:custGeom><a:pathLst><a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"100\"/></a:moveTo><a:lnTo><a:pt x=\"50\" y=\"0\"/></a:lnTo>"
                                                "<a:lnTo><a:pt x=\"100\" y=\"100\"/></a:lnTo><a:close/></a:path></a:pathLst></a:custGeom>");
        const QString crop = QStringLiteral("<a:srcRect l=\"10000\" r=\"10000\"/>");
        ItemList list = dmlTree(env, dmlGrp(frame, dmlSp(dmlXfrm(0, 0, 20, 10) + triangle + dmlSolid(QStringLiteral("FF0000"))) +
                                                    dmlCxn(dmlXfrm(10, 10, 40, 20) + QStringLiteral("<a:prstGeom prst=\"line\"/>")) +
                                                    dmlPic(QString(), dmlXfrm(0, 20, 50, 20) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), crop)));
        auto g = dmlAs<GroupItem>(list[0]);
        QVERIFY(g && g->children.size() == 3);
        auto s = dmlAs<ShapeItem>(g->children[0]);
        QVERIFY(s);
        QVERIFY2(dmlRectIs(s->rect, 100, 100, 40, 20), qPrintable(dmlRectText(s->rect)));
        QVERIFY2(dmlRectIs(s->customPath.boundingRect(), 0, 0, 40, 20), qPrintable(dmlRectText(s->customPath.boundingRect())));
        auto l = dmlAs<LineItem>(g->children[1]);
        QVERIFY(l);
        QVERIFY(near(l->p1.x(), 120) && near(l->p1.y(), 120) && near(l->p2.x(), 200) && near(l->p2.y(), 160));
        auto p = dmlAs<PictureItem>(g->children[2]);
        QVERIFY(p);
        QVERIFY2(dmlRectIs(p->rect, 100, 140, 100, 40), qPrintable(dmlRectText(p->rect)));
        // The picture is 125 wide behind a 100 frame cropped a tenth each side: still so in the larger frame.
        QVERIFY2(dmlRectIs(p->imgRect, -12.5, 0, 125, 40), qPrintable(dmlRectText(p->imgRect)));
        // Lines and shadows keep their weight.
        list = dmlTree(env, dmlGrp(frame, dmlSp(dmlXfrm(0, 0, 20, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:ln w=\"25400\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000"))))));
        QVERIFY(near(dmlAs<GroupItem>(list[0])->children[0]->stroke.width, 2));
    }

    void dmlGroupPassesItsFillDown()
    {
        DmlEnv env;
        const QString inGroup = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:grpFill/>"));
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        ItemList list = dmlTree(env, dmlGrp(frame + dmlSolid(QStringLiteral("00FF00")), inGroup));
        QCOMPARE(dmlHex(dmlAs<GroupItem>(list[0])->children[0]->fill.color), QStringLiteral("00FF00"));
        // A group in the group, with no fill of its own, passes the outer's on.
        list = dmlTree(env, dmlGrp(frame + dmlSolid(QStringLiteral("00FF00")), dmlGrp(frame, inGroup, QStringLiteral("21"))));
        QCOMPARE(dmlHex(dmlAs<GroupItem>(dmlAs<GroupItem>(list[0])->children[0])->children[0]->fill.color), QStringLiteral("00FF00"));
        // One with its own fill is a different group's.
        list = dmlTree(env, dmlGrp(frame + dmlSolid(QStringLiteral("00FF00")), dmlGrp(frame + dmlSolid(QStringLiteral("0000FF")), inGroup, QStringLiteral("21"))));
        QCOMPARE(dmlHex(dmlAs<GroupItem>(dmlAs<GroupItem>(list[0])->children[0])->children[0]->fill.color), QStringLiteral("0000FF"));
        // A group with no fill gives none, and one outside groups has no group's to take.
        list = dmlTree(env, dmlGrp(frame, inGroup));
        QVERIFY(dmlAs<GroupItem>(list[0])->children[0]->fill.isNone());
        list = dmlTree(env, inGroup);
        QVERIFY(list[0]->fill.isNone());
    }

    void dmlPictureReadsCropMaskAndAdjustments()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlXfrm(10, 20, 100, 50);
        const auto pic = [&](const QString &blip, const QString &spPr = QString(), const QString &blipFill = QStringLiteral("<a:stretch><a:fillRect/></a:stretch>")) {
            return dmlAs<PictureItem>(dmlOne(env, dmlPic(blip, frame + (spPr.isEmpty() ? QStringLiteral("<a:prstGeom prst=\"rect\"/>") : spPr), blipFill)));
        };
        auto p = pic(QString());
        QVERIFY(p);
        QCOMPARE(p->imageId, QStringLiteral("img1"));
        QCOMPARE(env.images.size(), 1);
        QVERIFY2(dmlRectIs(p->rect, 10, 20, 100, 50), qPrintable(dmlRectText(p->rect)));
        QVERIFY2(dmlRectIs(p->imgRect, 0, 0, 100, 50), qPrintable(dmlRectText(p->imgRect)));
        QCOMPARE(p->maskShape, QStringLiteral("rect"));
        QCOMPARE(p->name, QStringLiteral("Picture 5"));
        QCOMPARE(p->altText, QStringLiteral("a picture"));
        QVERIFY(p->wrap.mode == jp::Wrap::None);
        QVERIFY(p->recolor == PictureItem::NoRecolor && p->transparency == 0 && p->brightness == 0 && p->contrast == 0 && !p->hasTransparentColor);
        // A tenth cut from the left and right, a fifth from the bottom: the whole picture is 125 wide and 62.5 high, begun 12.5 left of the frame.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"10000\" r=\"10000\" b=\"20000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, -12.5, 0, 125, 62.5), qPrintable(dmlRectText(p->imgRect)));
        // Cut from the top, too.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect t=\"50000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, 0, -50, 100, 100), qPrintable(dmlRectText(p->imgRect)));
        // Negative cuts are space around the picture.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"-20000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, 100.0 / 1.2 * 0.2, 0, 100 / 1.2, 50), qPrintable(dmlRectText(p->imgRect)));
        // The picture stretched into part of the frame.
        p = pic(QString(), QString(), QStringLiteral("<a:stretch><a:fillRect l=\"10000\" t=\"20000\" r=\"30000\" b=\"0\"/></a:stretch>"));
        QVERIFY2(dmlRectIs(p->imgRect, 10, 10, 60, 40), qPrintable(dmlRectText(p->imgRect)));
        // Crops that cut away all of the picture (or more) are not applied.
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"60000\" r=\"60000\"/><a:stretch><a:fillRect/></a:stretch>"));
        QVERIFY(dmlRectIs(p->imgRect, 0, 0, 100, 50));
        p = pic(QString(), QString(), QStringLiteral("<a:srcRect l=\"99999999999\" t=\"-99999999999\"/>"));
        QVERIFY(std::isfinite(p->imgRect.x()) && std::isfinite(p->imgRect.width()) && qAbs(p->imgRect.width()) <= 1e6 && qAbs(p->imgRect.x()) <= 1e6);
        // The shape the picture is cut to.
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"))->maskShape, QStringLiteral("ellipse"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"roundRect\"/>"))->maskShape, QStringLiteral("roundRect"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"star5\"/>"))->maskShape, QStringLiteral("star5"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"flowChartTerminator\"/>"))->maskShape, QStringLiteral("fcTerminator"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:prstGeom prst=\"gear6\"/>"))->maskShape, QStringLiteral("rect"));
        QCOMPARE(pic(QString(), QStringLiteral("<a:custGeom><a:pathLst/></a:custGeom>"))->maskShape, QStringLiteral("rect"));
        QVERIFY(jp::shapeDef(pic(QString(), QStringLiteral("<a:prstGeom prst=\"heart\"/>"))->maskShape) != nullptr);
        // Transparency, gray, black and white, brightness, contrast.
        p = pic(QStringLiteral("<a:alphaModFix amt=\"25000\"/>"));
        QVERIFY(near(p->transparency, 0.75));
        p = pic(QStringLiteral("<a:grayscl/>"));
        QVERIFY(p->recolor == PictureItem::Grayscale);
        p = pic(QStringLiteral("<a:biLevel thresh=\"50000\"/>"));
        QVERIFY(p->recolor == PictureItem::BlackWhite);
        p = pic(QStringLiteral("<a:lum bright=\"20000\" contrast=\"-30000\"/>"));
        QVERIFY(near(p->brightness, 20) && near(p->contrast, -30));
        p = pic(QStringLiteral("<a:lum bright=\"99999999\" contrast=\"-99999999\"/>"));
        QVERIFY(p->brightness == 100 && p->contrast == -100);
        // One color made clear.
        p = pic(QStringLiteral("<a:clrChange><a:clrFrom><a:srgbClr val=\"FFFFFF\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"FFFFFF\"><a:alpha val=\"0\"/></a:srgbClr></a:clrTo></a:clrChange>"));
        QVERIFY(p->hasTransparentColor);
        QCOMPARE(p->transparentColor.name(), QStringLiteral("#ffffff"));
        // Outline and turn.
        p = dmlAs<PictureItem>(dmlOne(env, dmlPic(QString(), dmlXfrm(0, 0, 10, 10, QStringLiteral("rot=\"10800000\" flipV=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"rect\"/><a:ln w=\"19050\">%1</a:ln>").arg(dmlSolid(QStringLiteral("00FF00"))))));
        QVERIFY(near(p->rotation, 180) && p->flipV && !p->flipH);
        QVERIFY(near(p->stroke.width, 1.5));
        // Two pictures of one picture keep it once.
        env.images.clear();
        const ItemList both = dmlTree(env, dmlPic(QString(), frame + QStringLiteral("<a:prstGeom prst=\"rect\"/>")) + dmlPic(QString(), frame + QStringLiteral("<a:prstGeom prst=\"rect\"/>")));
        QCOMPARE(int(both.size()), 2);
        QCOMPARE(env.images.size(), 1);
        QCOMPARE(dmlAs<PictureItem>(both[0])->imageId, dmlAs<PictureItem>(both[1])->imageId);
    }

    void dmlPictureThatCannotBeReadIsAPlaceholder()
    {
        DmlEnv env;
        env.usePackage();
        const QString frame = dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        for (const char *embed : {"rId9", "rId2", "", "rIdX"}) {
            auto p = dmlAs<PictureItem>(dmlOne(env, dmlPic(QString(), frame, QStringLiteral("<a:stretch/>"), QString(), QString::fromLatin1(embed))));
            QVERIFY2(p, embed);
            QVERIFY2(p->imageId.isEmpty(), embed);
            QVERIFY(dmlRectIs(p->rect, 10, 20, 100, 50));
            QVERIFY(dmlRectIs(p->imgRect, 0, 0, 100, 50));
        }
        // Bytes that are no picture the program knows, a part without bytes, and no package at all.
        DmlEnv junk;
        junk.usePackage(QByteArray("this is not a picture at all, nor any kind of image"));
        auto p = dmlAs<PictureItem>(dmlOne(junk, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
        DmlEnv empty;
        empty.usePackage(QByteArray());
        p = dmlAs<PictureItem>(dmlOne(empty, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
        DmlEnv bare;
        p = dmlAs<PictureItem>(dmlOne(bare, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
        // A picture with no blip at all.
        p = dmlAs<PictureItem>(dmlOne(env, QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"5\" name=\"P\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill/><p:spPr>%1</p:spPr></p:pic>").arg(frame)));
        QVERIFY(p && p->imageId.isEmpty());
        p = dmlAs<PictureItem>(dmlOne(env, QStringLiteral("<p:pic/>")));
        QVERIFY(p && p->imageId.isEmpty());
        // Without a way to keep pictures, none is kept.
        DmlEnv noKeep;
        noKeep.usePackage();
        noKeep.ctx.addImage = nullptr;
        p = dmlAs<PictureItem>(dmlOne(noKeep, dmlPic(QString(), frame)));
        QVERIFY(p && p->imageId.isEmpty());
    }

    void dmlConnectorsBecomeLines()
    {
        DmlEnv env;
        env.useFormats();
        const QString style = dmlStyle(QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("accent1"));
        // The frame's corners, upside down.
        ItemPtr it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50, QStringLiteral("flipV=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"straightConnector1\"/>"), QString(), style));
        auto l = dmlAs<LineItem>(it);
        QVERIFY(l);
        QVERIFY(near(l->p1.x(), 10) && near(l->p1.y(), 70) && near(l->p2.x(), 110) && near(l->p2.y(), 20));
        QVERIFY(l->route == LineItem::Straight);
        QCOMPARE(l->name, QStringLiteral("Connector 9"));
        QVERIFY(l->wrap.mode == jp::Wrap::None);
        QCOMPARE(dmlHex(l->stroke.color), QStringLiteral("4472C4"));
        // Turned a quarter about its center.
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50, QStringLiteral("rot=\"5400000\"")) + QStringLiteral("<a:prstGeom prst=\"line\"/>"), QString(), style));
        l = dmlAs<LineItem>(it);
        QVERIFY2(near(l->p1.x(), 85) && near(l->p1.y(), -5) && near(l->p2.x(), 35) && near(l->p2.y(), 95), qPrintable(QStringLiteral("%1,%2 %3,%4").arg(l->p1.x()).arg(l->p1.y()).arg(l->p2.x()).arg(l->p2.y())));
        // Arrowheads and weight, from the line properties.
        it = dmlOne(env, dmlCxn(dmlXfrm(0, 0, 100, 0) + QStringLiteral("<a:prstGeom prst=\"line\"/><a:ln w=\"38100\">%1<a:headEnd type=\"oval\"/><a:tailEnd type=\"triangle\" w=\"lg\" len=\"lg\"/></a:ln>")
                                   .arg(dmlSolid(QStringLiteral("FF0000")))));
        l = dmlAs<LineItem>(it);
        QVERIFY(near(l->stroke.width, 3));
        QVERIFY(l->stroke.startArrow == Arrow::Oval && l->stroke.endArrow == Arrow::Triangle);
        QVERIFY(near(l->p1.x(), 0) && near(l->p2.x(), 100) && near(l->p1.y(), 0) && near(l->p2.y(), 0));
        // A connector that says nothing of its shape is a line from corner to corner.
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50)));
        l = dmlAs<LineItem>(it);
        QVERIFY(l && near(l->p1.x(), 10) && near(l->p1.y(), 20) && near(l->p2.x(), 110) && near(l->p2.y(), 70));
        // A line drawn as a shape is a line.
        it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 80, 20) + QStringLiteral("<a:prstGeom prst=\"line\"/><a:ln w=\"12700\">%1</a:ln>").arg(dmlSolid(QStringLiteral("000000")))));
        l = dmlAs<LineItem>(it);
        QVERIFY(l && near(l->p2.x(), 90) && near(l->p2.y(), 30));
        // Elbows and curves keep their bend and the way they leave.
        const auto route = [&](const char *prst, const QString &attrs, const QString &adj = QString()) {
            return dmlAs<LineItem>(dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50, attrs) + QStringLiteral("<a:prstGeom prst=\"%1\"><a:avLst>%2</a:avLst></a:prstGeom>").arg(QString::fromLatin1(prst), adj), QString(), style)));
        };
        l = route("bentConnector3", QString(), QStringLiteral("<a:gd name=\"adj1\" fmla=\"val 25000\"/>"));
        QVERIFY(l && l->route == LineItem::Elbow);
        QVERIFY(near(l->bend, 0.25));
        QVERIFY(!l->startVertical && !l->endVertical);
        l = route("bentConnector3", QStringLiteral("rot=\"5400000\""));
        QVERIFY(near(l->bend, 0.5));
        QVERIFY(l->startVertical && l->endVertical);
        l = route("bentConnector3", QStringLiteral("rot=\"10800000\""));
        QVERIFY(!l->startVertical && !l->endVertical);
        l = route("bentConnector3", QStringLiteral("rot=\"16200000\" flipH=\"1\""));
        QVERIFY(l->startVertical && l->endVertical);
        l = route("bentConnector3", QString(), QStringLiteral("<a:gd name=\"adj1\" fmla=\"val -300000\"/>"));
        QVERIFY(l->bend >= -10 && l->bend <= 11);
        l = route("bentConnector2", QString());
        QVERIFY(l->route == LineItem::Elbow && !l->startVertical && l->endVertical);
        l = route("bentConnector2", QStringLiteral("rot=\"5400000\""));
        QVERIFY(l->startVertical && !l->endVertical);
        l = route("curvedConnector3", QString());
        QVERIFY(l->route == LineItem::Curved && !l->startVertical && !l->endVertical);
        l = route("curvedConnector2", QString());
        QVERIFY(l->route == LineItem::Curved && !l->startVertical && l->endVertical);
        // With more bends than a line has, a path.
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"bentConnector4\"/>"), QString(), style));
        auto s = dmlAs<ShapeItem>(it);
        QVERIFY(s);
        QVERIFY(s->fill.isNone());
        QVERIFY(!s->stroke.isNone());
        QVERIFY(s->customPath.elementCount() == 5);
        QVERIFY2(dmlRectIs(s->rect, 10, 20, 100, 50), qPrintable(dmlRectText(s->rect)));
        it = dmlOne(env, dmlCxn(dmlXfrm(10, 20, 100, 50) + QStringLiteral("<a:prstGeom prst=\"curvedConnector5\"/>"), QString(), style));
        QVERIFY(dmlAs<ShapeItem>(it) && !dmlAs<ShapeItem>(it)->customPath.isEmpty());
    }

    void dmlConnectorsKeepWhatTheyAreJoinedTo()
    {
        DmlEnv env;
        const QString rect = QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        const QString shapes = dmlSp(dmlXfrm(10, 10, 50, 50) + rect, QString(), QString(), QString(), QString(), QString(), QStringLiteral("3")) +
                               dmlSp(dmlXfrm(200, 10, 50, 50) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), QString(), QString(), QString(), QString(), QString(), QStringLiteral("4"));
        const auto joined = [&](const QString &ends) {
            const ItemList list = dmlTree(env, shapes + dmlCxn(dmlXfrm(60, 35, 140, 0) + QStringLiteral("<a:prstGeom prst=\"straightConnector1\"/>"), ends));
            return QPair<ItemList, std::shared_ptr<LineItem>>(list, dmlAs<LineItem>(list.back()));
        };
        auto result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"3\"/><a:endCxn id=\"4\" idx=\"2\"/>"));
        QCOMPARE(int(result.first.size()), 3);
        auto l = result.second;
        QVERIFY(l);
        QCOMPARE(l->start.id, result.first[0]->id);
        QCOMPARE(l->start.site, 3);          // the right side of the rectangle
        QCOMPARE(l->end.id, result.first[1]->id);
        QCOMPARE(l->end.site, 1);            // the ellipse's third point is its left
        result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"0\"/><a:endCxn id=\"4\" idx=\"6\"/>"));
        QCOMPARE(result.second->start.site, 0);
        QCOMPARE(result.second->end.site, 3);
        result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"2\"/>"));
        QCOMPARE(result.second->start.site, 2);
        QVERIFY(result.second->end.id.isEmpty());
        // A point that the library's shape has no place for, a shape that isn't there, and a place that is not one: not joined.
        result = joined(QStringLiteral("<a:stCxn id=\"4\" idx=\"3\"/><a:endCxn id=\"77\" idx=\"0\"/>"));
        QVERIFY(result.second->start.id.isEmpty() && result.second->end.id.isEmpty());
        result = joined(QStringLiteral("<a:stCxn id=\"3\" idx=\"9\"/><a:endCxn id=\"3\" idx=\"-1\"/>"));
        QVERIFY(result.second->start.id.isEmpty() && result.second->end.id.isEmpty());
        result = joined(QStringLiteral("<a:stCxn id=\"x\" idx=\"y\"/>"));
        QVERIFY(result.second->start.id.isEmpty());
        // Joined to something inside a group, and to something that comes after the line.
        const QString grouped = dmlGrp(dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100)), dmlSp(dmlXfrm(10, 10, 50, 50) + rect, QString(), QString(), QString(), QString(), QString(), QStringLiteral("30")));
        const ItemList list = dmlTree(env, dmlCxn(dmlXfrm(60, 35, 140, 0) + QStringLiteral("<a:prstGeom prst=\"straightConnector1\"/>"), QStringLiteral("<a:stCxn id=\"30\" idx=\"3\"/>")) + grouped);
        QCOMPARE(dmlAs<LineItem>(list[0])->start.id, dmlAs<GroupItem>(list[1])->children[0]->id);
    }

    void dmlHyperlinksAreTheirExternalTargets()
    {
        DmlEnv env;
        env.usePackage();
        const QString link = QStringLiteral("<a:hlinkClick r:id=\"rId2\"/>");
        const QString frame = dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        QCOMPARE(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), link))->hyperlink, QStringLiteral("https://example.com/x"));
        QCOMPARE(dmlOne(env, dmlSp(frame, dmlBody(), QString(), QStringLiteral("txBox=\"1\""), QString(), link))->hyperlink, QStringLiteral("https://example.com/x"));
        QCOMPARE(dmlOne(env, dmlPic(QString(), frame, QStringLiteral("<a:stretch/>"), link))->hyperlink, QStringLiteral("https://example.com/x"));
        const QString connector = QStringLiteral("<p:cxnSp><p:nvCxnSpPr><p:cNvPr id=\"9\" name=\"c\">%1</p:cNvPr><p:cNvCxnSpPr/><p:nvPr/></p:nvCxnSpPr><p:spPr>%2<a:prstGeom prst=\"line\"/></p:spPr></p:cxnSp>")
                                    .arg(link, dmlXfrm(0, 0, 10, 10));
        QCOMPARE(dmlOne(env, connector)->hyperlink, QStringLiteral("https://example.com/x"));
        // Nothing to go to: a link to a part inside the file, one that isn't there, and one with no target.
        QVERIFY(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), QStringLiteral("<a:hlinkClick r:id=\"rId1\"/>")))->hyperlink.isEmpty());
        QVERIFY(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), QStringLiteral("<a:hlinkClick r:id=\"rId9\"/>")))->hyperlink.isEmpty());
        QVERIFY(dmlOne(env, dmlSp(frame, QString(), QString(), QString(), QString(), QStringLiteral("<a:hlinkClick/>")))->hyperlink.isEmpty());
        QVERIFY(dmlOne(env, dmlSp(frame))->hyperlink.isEmpty());
        DmlEnv bare;
        QVERIFY(dmlOne(bare, dmlSp(frame, QString(), QString(), QString(), QString(), link))->hyperlink.isEmpty());
    }

    void dmlTreeSkipsWhatIsHiddenAndTakesTheFallback()
    {
        DmlEnv env;
        const QString frame = dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>");
        // A hidden shape isn't drawn.
        QString hidden = dmlSp(frame);
        hidden.replace(QStringLiteral("name=\"Shape 2\""), QStringLiteral("name=\"Shape 2\" hidden=\"1\""));
        QVERIFY(dmlTree(env, hidden).empty());
        QVERIFY(!dmlOne(env, hidden));
        QCOMPARE(int(dmlTree(env, hidden + dmlSp(frame)).size()), 1);
        // What a newer program wrote with a fallback for older ones: the fallback.
        const QString alt = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1</mc:Choice>"
                                           "<mc:Fallback>%2</mc:Fallback></mc:AlternateContent>");
        ItemList list = dmlTree(env, dmlSp(frame) + alt.arg(dmlSp(frame, QString(), QString(), QString(), QString(), QString(), QStringLiteral("7")), dmlSp(frame, QString(), QString(), QString(), QString(), QString(), QStringLiteral("8")) +
                                                                dmlSp(frame, QString(), QString(), QString(), QString(), QString(), QStringLiteral("9"))));
        QCOMPARE(int(list.size()), 3);
        QCOMPARE(list[1]->name, QStringLiteral("Shape 8"));
        QCOMPARE(list[2]->name, QStringLiteral("Shape 9"));
        // What isn't read yet (tables, charts) and the tree's own properties give nothing.
        list = dmlTree(env, QStringLiteral("<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/><p:graphicFrame/><p:contentPart/><p:unknown/>") + dmlSp(frame));
        QCOMPARE(int(list.size()), 1);
        QVERIFY(!dmlOne(env, QStringLiteral("<p:graphicFrame/>")));
        QVERIFY(!dmlOne(env, QStringLiteral("<p:unknown/>")));
        QVERIFY(!readShape(QDomElement(), env.ctx));
        QVERIFY(readShapeTree(QDomElement(), env.ctx).empty());
        // Shapes of a document's drawing are read the same, in their own namespace.
        QDomDocument doc;
        const QByteArray wps = QStringLiteral("<drawing xmlns:a=\"%1\" xmlns:wps=\"urn:test:wordprocessing-shape\"><wps:wsp><wps:cNvSpPr/>"
                                              "<wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"254000\" cy=\"127000\"/></a:xfrm><a:prstGeom prst=\"ellipse\"/></wps:spPr></wps:wsp></drawing>")
                                   .arg(QString(ns::A)).toUtf8();
        QVERIFY(parseXml(wps, &doc));
        const ItemList docShapes = readShapeTree(doc.documentElement(), env.ctx);
        QCOMPARE(int(docShapes.size()), 1);
        QCOMPARE(dmlAs<ShapeItem>(docShapes[0])->shape, QStringLiteral("ellipse"));
        QVERIFY(dmlRectIs(docShapes[0]->rect, 0, 0, 20, 10));
    }

    void dmlHostileShapesAreHeldToWhatAPageCanShow()
    {
        DmlEnv env;
        const auto check = [&](const ItemList &list) {
            jp::walkItems(list, [&](const ItemPtr &it) {
                QVERIFY(std::isfinite(it->rect.x()) && std::isfinite(it->rect.y()) && std::isfinite(it->rect.width()) && std::isfinite(it->rect.height()));
                QVERIFY(qAbs(it->rect.x()) <= 1e6 && qAbs(it->rect.y()) <= 1e6 && it->rect.width() >= 0 && it->rect.height() >= 0);
                if (it->type() != ItemType::Line && it->type() != ItemType::Group) QVERIFY(it->rect.width() <= 1e6 && it->rect.height() <= 1e6);
                QVERIFY(std::isfinite(it->rotation) && it->rotation >= 0 && it->rotation < 360);
                if (it->type() == ItemType::Line) {
                    auto *l = static_cast<LineItem *>(it.get());
                    QVERIFY(std::isfinite(l->p1.x()) && std::isfinite(l->p2.y()) && qAbs(l->p1.x()) <= 1e6 && qAbs(l->p2.y()) <= 1e6);
                }
            });
        };
        for (const char *huge : {"99999999999999999999", "-99999999999999999999", "1e300", "nan", "inf", "-5", "0", "9223372036854775807", "-9223372036854775808"}) {
            const QString v = QString::fromLatin1(huge);
            const QString xfrm = QStringLiteral("<a:xfrm rot=\"%1\"><a:off x=\"%1\" y=\"%1\"/><a:ext cx=\"%1\" cy=\"%1\"/></a:xfrm>").arg(v);
            const QString geometries[] = {QStringLiteral("<a:prstGeom prst=\"rect\"/>"), QStringLiteral("<a:prstGeom prst=\"line\"/>"), QStringLiteral("<a:prstGeom prst=\"pie\"/>"),
                                          QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val %1\"/></a:avLst></a:prstGeom>").arg(v)};
            const QString groupFrame = QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%1\"/><a:ext cx=\"%1\" cy=\"%1\"/><a:chOff x=\"%1\" y=\"%1\"/><a:chExt cx=\"%1\" cy=\"%1\"/></a:xfrm>").arg(v);
            for (const QString &geometry : geometries) {
                const QString all = dmlSp(xfrm + geometry) + dmlCxn(xfrm + geometry) + dmlPic(QString(), xfrm + geometry);
                check(dmlTree(env, dmlSp(xfrm + geometry, dmlBody(), QString(), QStringLiteral("txBox=\"1\"")) + all + dmlGrp(groupFrame, all)));
            }
        }
        // A group scaled to a speck of its child space or from one: positions stay where they can.
        check(dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 1e5, 1e5), QRectF(0, 0, 1e-4, 1e-4)), dmlSp(dmlXfrm(1, 1, 5, 5) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")))));
        check(dmlTree(env, dmlGrp(dmlGroupFrame(QRectF(0, 0, 1e-4, 1e-4), QRectF(0, 0, 1e5, 1e5)), dmlSp(dmlXfrm(1, 1, 5, 5) + QStringLiteral("<a:prstGeom prst=\"rect\"/>")))));
    }

    void dmlGroupsAreReadToALimitedDepth()
    {
        DmlEnv env;
        const QString leaf = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        const auto nested = [&](int levels) {
            QString xml = leaf;
            for (int i = 0; i < levels; ++i) xml = dmlGrp(frame, xml, QString::number(100 + i));
            return xml;
        };
        const auto depth = [](const ItemList &list) {
            int deepest = 0;
            std::function<void(const ItemList &, int)> walk = [&](const ItemList &l, int d) {
                for (const ItemPtr &it : l)
                    if (it->type() == ItemType::Group) walk(static_cast<GroupItem *>(it.get())->children, d + 1);
                    else deepest = qMax(deepest, d);
            };
            walk(list, 0);
            return deepest;
        };
        QCOMPARE(depth(dmlTree(env, nested(10))), 10);
        QCOMPARE(depth(dmlTree(env, nested(64))), 64);
        QVERIFY(depth(dmlTree(env, nested(65))) <= 64);
        QVERIFY(depth(dmlTree(env, nested(200))) <= 64);
        // A single group is read to the same limit.
        ItemList single;
        if (const ItemPtr one = dmlOne(env, nested(100))) single.push_back(one);
        QVERIFY(depth(single) <= 64);
    }

    void dmlTreesAreReadToALimitedSize()
    {
        DmlEnv env;
        const QString leaf = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        QString many;
        for (int i = 0; i < 10001; ++i) many += leaf;
        int count = 0;
        jp::walkItems(dmlTree(env, many), [&](const ItemPtr &) { ++count; });
        QCOMPARE(count, 10000);
        // Counting shapes inside groups, and the groups themselves.
        QString groups;
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        for (int g = 0; g < 200; ++g) {
            QString inside;
            for (int i = 0; i < 100; ++i) inside += leaf;
            groups += dmlGrp(frame, inside, QString::number(1000 + g));
        }
        count = 0;
        jp::walkItems(dmlTree(env, groups), [&](const ItemPtr &) { ++count; });
        QVERIFY2(count <= 10000, qPrintable(QString::number(count)));
        QVERIFY(count > 9000);
        // A million small shapes asked for through a tree of repeated pictures costs no more than the limit.
        env.usePackage();
        env.images.clear();
        QString pics;
        for (int i = 0; i < 3000; ++i) pics += dmlPic(QString(), dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        QCOMPARE(int(dmlTree(env, pics).size()), 3000);
        QCOMPARE(env.images.size(), 1);
    }

    void dmlShapesSurviveRandomDamage()
    {
        DmlEnv env;
        env.useFormats();
        env.usePackage();
        const QString tree = QStringLiteral("<p:spTree %1>").arg(dmlNamespaces()) +
            dmlSp(dmlXfrm(10, 10, 100, 50, QStringLiteral("rot=\"5400000\" flipH=\"1\"")) + QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 30000\"/></a:avLst></a:prstGeom>") +
                  QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"FF0000\"/></a:gs><a:gs pos=\"100000\"><a:schemeClr val=\"accent1\"/></a:gs></a:gsLst><a:lin ang=\"2700000\" scaled=\"1\"/></a:gradFill>"
                                 "<a:ln w=\"12700\">%1<a:prstDash val=\"dash\"/><a:tailEnd type=\"triangle\"/></a:ln><a:effectLst><a:outerShdw blurRad=\"50800\" dist=\"38100\" dir=\"2700000\"><a:srgbClr val=\"000000\"/></a:outerShdw></a:effectLst>")
                      .arg(dmlSolid(QStringLiteral("00FF00"))),
                  dmlBody(QStringLiteral("Text"), QStringLiteral("<a:bodyPr lIns=\"0\" anchor=\"ctr\" numCol=\"2\"><a:normAutofit fontScale=\"50000\"/></a:bodyPr>")),
                  dmlStyle(QStringLiteral("2"), QStringLiteral("2"), QStringLiteral("3")), QStringLiteral("txBox=\"1\""), QString(), QStringLiteral("<a:hlinkClick r:id=\"rId2\"/>")) +
            dmlGrp(dmlGroupFrame(QRectF(100, 100, 200, 100), QRectF(0, 0, 100, 50), QStringLiteral("rot=\"600000\" flipV=\"1\"")) + QStringLiteral("<a:grpFill/>"),
                   dmlSp(dmlXfrm(1, 2, 3, 4) + QStringLiteral("<a:custGeom><a:avLst><a:gd name=\"a\" fmla=\"val 5\"/></a:avLst><a:gdLst><a:gd name=\"x\" fmla=\"*/ w a 100\"/></a:gdLst><a:pathLst>"
                                                              "<a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:arcTo wR=\"x\" hR=\"hd2\" stAng=\"0\" swAng=\"5400000\"/>"
                                                              "<a:cubicBezTo><a:pt x=\"1\" y=\"2\"/><a:pt x=\"3\" y=\"4\"/><a:pt x=\"5\" y=\"6\"/></a:cubicBezTo><a:close/></a:path></a:pathLst></a:custGeom><a:grpFill/>")) +
                   dmlCxn(dmlXfrm(1, 2, 30, 40) + QStringLiteral("<a:prstGeom prst=\"bentConnector3\"><a:avLst><a:gd name=\"adj1\" fmla=\"val 20000\"/></a:avLst></a:prstGeom>"), QStringLiteral("<a:stCxn id=\"2\" idx=\"1\"/>")) +
                   dmlSp(dmlXfrm(1, 2, 3, 4) + QStringLiteral("<a:prstGeom prst=\"wedgeRectCallout\"/>"))) +
            dmlPic(QStringLiteral("<a:alphaModFix amt=\"50000\"/><a:lum bright=\"10000\"/>"), dmlXfrm(5, 5, 60, 40) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"),
                   QStringLiteral("<a:srcRect l=\"10000\" t=\"5000\"/><a:stretch><a:fillRect/></a:stretch>")) +
            QStringLiteral("</p:spTree>");
        QDomDocument pristine;
        QVERIFY(parseXml(tree.toUtf8(), &pristine));
        QVERIFY(!readShapeTree(pristine.documentElement(), env.ctx).empty());
        QRandomGenerator gen(2024);
        const QByteArray source = tree.toUtf8();
        const QList<QByteArray> numbers = {"0", "1", "-1", "5400000", "21600000", "99999999999999999999", "-99999999999999999999", "100000", "12700", "7", "000"};
        const QRegularExpression digits(QStringLiteral("-?[0-9]+"));
        QList<QPair<int, int>> runs;   // where each number is written
        for (auto m = digits.globalMatch(QString::fromUtf8(source)); m.hasNext();) {
            const auto match = m.next();
            runs.append({int(match.capturedStart()), int(match.capturedLength())});
        }
        int read = 0;
        for (int i = 0; i < 900; ++i) {
            QByteArray bad = source;
            if (i % 3 == 0) {
                // Bytes of the file at random.
                for (int k = 0, n = 1 + int(gen.bounded(4)); k < n; ++k) bad[int(gen.bounded(quint32(bad.size())))] = char(gen.bounded(256u));
            } else if (i % 3 == 1) {
                // Letters and digits for others, which leaves most of it XML.
                for (int k = 0, n = 1 + int(gen.bounded(4)); k < n; ++k) {
                    const int at = int(gen.bounded(quint32(bad.size())));
                    if (QChar(bad[at]).isLetterOrNumber()) bad[at] = "0123456789abcxyzAZ-"[gen.bounded(19u)];
                }
            } else {
                // A number written another way, as many as three of them.
                for (int k = 0, n = 1 + int(gen.bounded(3)); k < n; ++k) {
                    const auto run = runs[int(gen.bounded(quint32(runs.size())))];
                    bad.replace(run.first, run.second, QByteArray(run.second, ' '));   // the length stays, so the others' places hold
                    bad.replace(run.first, 1, numbers[int(gen.bounded(quint32(numbers.size())))].left(run.second));
                }
            }
            QDomDocument doc;
            if (!parseXml(bad, &doc)) continue;
            ++read;
            const ItemList list = readShapeTree(doc.documentElement(), env.ctx);
            jp::walkItems(list, [&](const ItemPtr &it) {
                QVERIFY(std::isfinite(it->rect.x()) && std::isfinite(it->rect.width()) && std::isfinite(it->rotation));
                QVERIFY(qAbs(it->rect.x()) <= 1e6 && it->rect.width() >= 0);
            });
            // The same damage in the pieces' own forms.
            for (QDomElement e = doc.documentElement().firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) readShape(e, env.ctx, {}, {});
            env.images.clear();
            env.stories.clear();
        }
        QVERIFY2(read > 400, qPrintable(QString::number(read)));   // enough of the damage left XML to read
    }

    void dmlReadsATreeAsTheProgramsWriteIt()
    {
        DmlEnv env;
        env.useFormats();
        env.usePackage();
        // A slide's shape tree, written with the extra elements and attributes real files carry.
        const QString tree = QStringLiteral(R"(<p:spTree>
  <p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
  <p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/><a:chOff x="0" y="0"/><a:chExt cx="0" cy="0"/></a:xfrm></p:grpSpPr>
  <p:sp>
    <p:nvSpPr><p:cNvPr id="2" name="Title 1"><a:extLst><a:ext uri="{FF2B5EF4-FFF2-40B4-BE49-F238E27FC236}"/></a:extLst></p:cNvPr>
      <p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr><p:ph type="title"/></p:nvPr></p:nvSpPr>
    <p:spPr/>
    <p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang="en-US" dirty="0"/><a:t>A title</a:t></a:r><a:endParaRPr lang="en-US" dirty="0"/></a:p></p:txBody>
  </p:sp>
  <p:sp>
    <p:nvSpPr><p:cNvPr id="3" name="TextBox 2"/><p:cNvSpPr txBox="1"/><p:nvPr/></p:nvSpPr>
    <p:spPr><a:xfrm><a:off x="1143000" y="1524000"/><a:ext cx="2286000" cy="369332"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom><a:noFill/></p:spPr>
    <p:txBody><a:bodyPr wrap="none" rtlCol="0"><a:spAutoFit/></a:bodyPr><a:lstStyle/><a:p><a:r><a:rPr lang="en-US" dirty="0"/><a:t>Some text</a:t></a:r></a:p></p:txBody>
  </p:sp>
  <p:sp>
    <p:nvSpPr><p:cNvPr id="4" name="Rounded Rectangle 3"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>
    <p:spPr><a:xfrm><a:off x="1143000" y="2286000"/><a:ext cx="2743200" cy="914400"/></a:xfrm><a:prstGeom prst="roundRect"><a:avLst/></a:prstGeom></p:spPr>
    <p:style><a:lnRef idx="2"><a:schemeClr val="accent1"><a:shade val="50000"/></a:schemeClr></a:lnRef><a:fillRef idx="1"><a:schemeClr val="accent1"/></a:fillRef>
      <a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef></p:style>
    <p:txBody><a:bodyPr rtlCol="0" anchor="ctr"/><a:lstStyle/><a:p><a:pPr algn="ctr"/><a:r><a:rPr lang="en-US" dirty="0"/><a:t>Shape</a:t></a:r></a:p></p:txBody>
  </p:sp>
  <p:pic>
    <p:nvPicPr><p:cNvPr id="5" name="Picture 4" descr="A red square"/><p:cNvPicPr><a:picLocks noChangeAspect="1"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>
    <p:blipFill><a:blip r:embed="rId1"><a:extLst><a:ext uri="{28A0092B-C50C-407E-A947-70E740481C1C}"/></a:extLst></a:blip><a:stretch><a:fillRect/></a:stretch></p:blipFill>
    <p:spPr><a:xfrm><a:off x="4572000" y="1524000"/><a:ext cx="1828800" cy="1371600"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom></p:spPr>
  </p:pic>
  <p:grpSp>
    <p:nvGrpSpPr><p:cNvPr id="10" name="Group 9"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
    <p:grpSpPr><a:xfrm><a:off x="6400800" y="1524000"/><a:ext cx="2286000" cy="1143000"/><a:chOff x="6400800" y="1524000"/><a:chExt cx="2286000" cy="1143000"/></a:xfrm></p:grpSpPr>
    <p:sp><p:nvSpPr><p:cNvPr id="11" name="Oval 10"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x="6400800" y="1524000"/><a:ext cx="914400" cy="1143000"/></a:xfrm><a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom></p:spPr>
      <p:style><a:lnRef idx="2"><a:schemeClr val="accent1"/></a:lnRef><a:fillRef idx="1"><a:schemeClr val="accent2"/></a:fillRef><a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef></p:style></p:sp>
    <p:sp><p:nvSpPr><p:cNvPr id="12" name="Right Arrow 11"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr><a:xfrm><a:off x="7543800" y="1752600"/><a:ext cx="1143000" cy="685800"/></a:xfrm><a:prstGeom prst="rightArrow"><a:avLst><a:gd name="adj1" fmla="val 50000"/><a:gd name="adj2" fmla="val 50000"/></a:avLst></a:prstGeom></p:spPr></p:sp>
  </p:grpSp>
  <p:cxnSp>
    <p:nvCxnSpPr><p:cNvPr id="13" name="Elbow Connector 12"/><p:cNvCxnSpPr><a:stCxn id="4" idx="3"/><a:endCxn id="5" idx="1"/></p:cNvCxnSpPr><p:nvPr/></p:nvCxnSpPr>
    <p:spPr><a:xfrm><a:off x="3886200" y="2209800"/><a:ext cx="685800" cy="762000"/></a:xfrm><a:prstGeom prst="bentConnector3"><a:avLst><a:gd name="adj1" fmla="val 50000"/></a:avLst></a:prstGeom><a:ln w="19050"><a:tailEnd type="triangle"/></a:ln></p:spPr>
    <p:style><a:lnRef idx="1"><a:schemeClr val="accent1"/></a:lnRef><a:fillRef idx="0"><a:schemeClr val="accent1"/></a:fillRef><a:effectRef idx="0"><a:schemeClr val="accent1"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="tx1"/></a:fontRef></p:style>
  </p:cxnSp>
  <p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id="14" name="Table 13"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr><p:xfrm><a:off x="0" y="0"/><a:ext cx="1" cy="1"/></p:xfrm></p:graphicFrame>
</p:spTree>)");
        const QString layoutTitle = dmlSp(dmlXfrm(100, 30, 760, 80), QString(), QString(), QString(), QStringLiteral("<p:ph type=\"title\"/>"));
        QDomElement spTree = env.element(tree);
        QVERIFY(!spTree.isNull());
        // The tree reads in the order it is written, the table left for the caller to keep.
        const ItemList list = readShapeTree(spTree, env.ctx);
        QCOMPARE(int(list.size()), 6);
        QVERIFY(dmlAs<TextItem>(list[0]));            // the title, with no place of its own (a placeholder read through a tree)
        auto box = dmlAs<TextItem>(list[1]);
        QVERIFY(box);
        QVERIFY(dmlRectIs(box->rect, 90, 120, 180, 369332.0 / 12700, 1e-6));
        QVERIFY(box->fill.isNone() && box->stroke.isNone());
        QVERIFY(box->autofit == TextItem::GrowBox);
        auto rounded = dmlAs<TextItem>(list[2]);      // a rectangle would be a text frame; this one is rounded, so a shape
        QVERIFY(!rounded);
        auto shape = dmlAs<ShapeItem>(list[2]);
        QVERIFY(shape);
        QCOMPARE(shape->shape, QStringLiteral("roundRect"));
        QCOMPARE(dmlHex(shape->fill.color), QStringLiteral("4472C4"));
        QVERIFY(near(shape->stroke.width, 1));
        QVERIFY(shape->valign == VAlign::Middle);
        QVERIFY(!shape->storyId.isEmpty());
        auto picture = dmlAs<PictureItem>(list[3]);
        QVERIFY(picture && !picture->imageId.isEmpty());
        QCOMPARE(picture->altText, QStringLiteral("A red square"));
        auto group = dmlAs<GroupItem>(list[4]);
        QVERIFY(group && group->children.size() == 2);
        QVERIFY(dmlRectIs(group->children[0]->rect, 504, 120, 72, 90));
        QVERIFY(dmlRectIs(group->rect, 504, 120, 180, 90));
        auto elbow = dmlAs<LineItem>(list[5]);
        QVERIFY(elbow && elbow->route == LineItem::Elbow);
        QVERIFY(elbow->stroke.endArrow == Arrow::Triangle);
        QCOMPARE(elbow->start.id, list[2]->id);
        QCOMPARE(elbow->start.site, 3);
        QCOMPARE(elbow->end.id, list[3]->id);
        QCOMPARE(elbow->end.site, 1);
        // What is read can be saved and opened again by the program's own format.
        jp::walkItems(list, [&](const ItemPtr &it) {
            const ItemPtr again = Item::fromJsonAny(it->toJson());
            QVERIFY(again && again->type() == it->type());
            QVERIFY(dmlRectIs(again->rect, it->rect.x(), it->rect.y(), it->rect.width(), it->rect.height(), 1e-9));
        });
        // The same title as a placeholder with a layout to take its place from.
        QDomElement titleElement = spTree.firstChildElement();
        while (!titleElement.isNull() && titleElement.localName() != QLatin1String("sp")) titleElement = titleElement.nextSiblingElement();
        QVERIFY(!titleElement.isNull());
        const ItemPtr placed = readShape(titleElement, env.ctx, {}, {env.element(layoutTitle)});
        QVERIFY2(dmlAs<TextItem>(placed), qPrintable(placed ? QString::number(int(placed->type())) : QStringLiteral("none")));
        QVERIFY(dmlRectIs(placed->rect, 100, 30, 760, 80));
    }

    void dmlReadShapesDrawWhereTheyAre()
    {
        DmlEnv env;
        const auto render = [](const ItemPtr &it, const QSize &size) {
            QImage img(size, QImage::Format_ARGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            auto s = dmlAs<ShapeItem>(it);
            const QPainterPath path = s->customPath.isEmpty() ? jp::shapePath(s->shape, s->rect.size(), s->adj) : s->customPath;
            p.fillPath(path, s->fill.brush(path.boundingRect(), jp::ColorScheme()));
            p.end();
            return img;
        };
        // A rounded rectangle is ink in the middle and at the edge of a side, but not in the corner.
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(10, 10, 100, 60) + QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst><a:gd name=\"adj\" fmla=\"val 50000\"/></a:avLst></a:prstGeom>") + dmlSolid(QStringLiteral("FF0000"))));
        QImage img = render(it, QSize(100, 60));
        QCOMPARE(img.pixelColor(50, 30), QColor(255, 0, 0));
        QCOMPARE(img.pixelColor(50, 2), QColor(255, 0, 0));
        QCOMPARE(img.pixelColor(2, 30), QColor(255, 0, 0));
        QCOMPARE(img.pixelColor(1, 1), QColor(255, 255, 255));
        QCOMPARE(img.pixelColor(98, 58), QColor(255, 255, 255));
        // A pie is ink in three quarters.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:prstGeom prst=\"pie\"/>") + dmlSolid(QStringLiteral("0000FF"))));
        img = render(it, QSize(100, 100));
        QCOMPARE(img.pixelColor(25, 25), QColor(0, 0, 255));
        QCOMPARE(img.pixelColor(75, 75), QColor(0, 0, 255));
        QCOMPARE(img.pixelColor(75, 25), QColor(255, 255, 255));
        // A shape of its own path, drawn only inside the path.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:custGeom><a:pathLst><a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"100\"/></a:moveTo>"
                                                                      "<a:lnTo><a:pt x=\"50\" y=\"0\"/></a:lnTo><a:lnTo><a:pt x=\"100\" y=\"100\"/></a:lnTo><a:close/></a:path></a:pathLst></a:custGeom>") + dmlSolid(QStringLiteral("00FF00"))));
        img = render(it, QSize(100, 100));
        QCOMPARE(img.pixelColor(50, 70), QColor(0, 255, 0));
        QCOMPARE(img.pixelColor(5, 20), QColor(255, 255, 255));
        // A star from the library, with the standard's own inner size.
        it = dmlOne(env, dmlSp(dmlXfrm(0, 0, 100, 100) + QStringLiteral("<a:prstGeom prst=\"star5\"/>") + dmlSolid(QStringLiteral("FF00FF"))));
        img = render(it, QSize(100, 100));
        QCOMPARE(img.pixelColor(50, 50), QColor(255, 0, 255));
        QCOMPARE(img.pixelColor(2, 2), QColor(255, 255, 255));
    }

    void dmlPresetGeometryUsesTheLibraryWhereItIsAsGood()
    {
        for (const char *name : {"rect", "roundRect", "ellipse", "triangle", "rightArrow", "star5", "hexagon", "flowChartProcess"})
            QVERIFY2(!prefersPresetGeometry(QString::fromLatin1(name)), name);
        for (const char *name : {"pie", "chord", "arc", "blockArc", "wedgeRectCallout", "wedgeRoundRectCallout", "wedgeEllipseCallout", "borderCallout1",
                                 "rightArrowCallout", "downArrowCallout"})
            QVERIFY2(prefersPresetGeometry(QString::fromLatin1(name)) && hasPresetGeometry(QString::fromLatin1(name)), name);
        QVERIFY(!hasPresetGeometry(QStringLiteral("rect")));
        QVERIFY(presetGeometry(QStringLiteral("rect"), {}, QSizeF(10, 10)).isEmpty());
        QVERIFY(presetGeometry(QStringLiteral("pie"), {}, QSizeF(0, 0)).isEmpty());
        QVERIFY(presetGeometry(QStringLiteral("pie"), {}, QSizeF(-5, 10)).isEmpty());
    }

    void dmlEveryPresetDefinitionSurvivesAnySizeAndAdjustment()
    {
        const QList<double> values = {-1e18, -100000, 0, 1, 50000, 100000, 21600000, 1e18};
        const QList<QSizeF> sizes = {QSizeF(100, 100), QSizeF(0, 50), QSizeF(50, 0), QSizeF(1e6, 1), QSizeF(1, 1e6), QSizeF(0.001, 0.001)};
        for (const QString &prst : dmlAllPresets()) {
            if (!hasPresetGeometry(prst)) continue;
            for (double v : values) {
                QHash<QString, double> adj;
                for (const char *name : {"adj", "adj1", "adj2", "adj3", "adj4", "adj5", "adj6", "adj7", "adj8"}) adj.insert(QString::fromLatin1(name), v);
                for (const QSizeF &size : sizes) {
                    const Geometry g = presetGeometry(prst, adj, size);
                    for (const GeometryPath &gp : g.paths)
                        for (int i = 0; i < gp.path.elementCount(); ++i) {
                            const auto e = gp.path.elementAt(i);
                            QVERIFY2(std::isfinite(e.x) && std::isfinite(e.y) && qAbs(e.x) <= 1e6 + 1 && qAbs(e.y) <= 1e6 + 1, qPrintable(prst));
                        }
                }
            }
        }
    }
};

JP_TEST_CLASS(OfficeDmlShapeTests)
#include "tst_office_dmlshape.moc"
