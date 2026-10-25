// Tests of libs/office: DrawingML geometry: guides, custom paths, and the preset shapes.
#include "officehelpers.h"

class OfficeDmlGeometryTests : public QObject {
    Q_OBJECT


    // <a:custGeom> holding `inner`, as the first child of a throwaway document.
    static QDomElement dmlElement(QDomDocument *doc, const QString &xml)
    {
        const QByteArray text = QStringLiteral("<root %1>%2</root>").arg(dmlNamespaces(), xml).toUtf8();
        if (!parseXml(text, doc)) return QDomElement();
        return doc->documentElement().firstChildElement();
    }

    static Geometry customGeometryOf(const QString &inner, const QSizeF &size)
    {
        QDomDocument doc;
        const QDomElement e = dmlElement(&doc, QStringLiteral("<a:custGeom>%1</a:custGeom>").arg(inner));
        return readCustomGeometry(e, size);
    }

    // The path "M x y L x y ..." the tests draw lines with, in a path of `w` by `h`.
    static QString linePath(const QString &attrs, const QList<QPair<QString, QString>> &points, bool close = true)
    {
        QString out = QStringLiteral("<a:path %1>").arg(attrs);
        for (int i = 0; i < points.size(); ++i)
            out += QStringLiteral("<a:%1><a:pt x=\"%2\" y=\"%3\"/></a:%1>").arg(i == 0 ? QStringLiteral("moveTo") : QStringLiteral("lnTo"), points[i].first, points[i].second);
        return out + (close ? QStringLiteral("<a:close/>") : QString()) + QStringLiteral("</a:path>");
    }

private Q_SLOTS:
    // ---- DrawingML shapes: guides and geometry

    void dmlGuideFormulas_data()
    {
        QTest::addColumn<QString>("formula");
        QTest::addColumn<double>("expected");
        QTest::newRow("val") << "val 42" << 42.0;
        QTest::newRow("multiply and divide") << "*/ 10 20 4" << 50.0;
        QTest::newRow("add and subtract") << "+- 10 3 4" << 9.0;
        QTest::newRow("add and divide") << "+/ 10 20 5" << 6.0;
        QTest::newRow("if positive") << "?: 1 7 9" << 7.0;
        QTest::newRow("if zero") << "?: 0 7 9" << 9.0;
        QTest::newRow("if negative") << "?: -1 7 9" << 9.0;
        QTest::newRow("abs") << "abs -5" << 5.0;
        QTest::newRow("at2 of 45 degrees") << "at2 1 1" << 2700000.0;
        QTest::newRow("at2 straight down") << "at2 0 1" << 5400000.0;
        QTest::newRow("at2 straight left") << "at2 -1 0" << 10800000.0;
        QTest::newRow("cat2") << "cat2 100 3 4" << 60.0;
        QTest::newRow("sat2") << "sat2 100 3 4" << 80.0;
        QTest::newRow("cos zero") << "cos 100 0" << 100.0;
        QTest::newRow("cos 60 degrees") << "cos 100 3600000" << 50.0;
        QTest::newRow("sin 90 degrees") << "sin 100 5400000" << 100.0;
        QTest::newRow("sin 45 degrees") << "sin 100 2700000" << 70.71067811865476;
        QTest::newRow("tan 45 degrees") << "tan 100 2700000" << 100.0;
        QTest::newRow("max") << "max 3 9" << 9.0;
        QTest::newRow("min") << "min 3 9" << 3.0;
        QTest::newRow("mod") << "mod 3 4 12" << 13.0;
        QTest::newRow("pin inside") << "pin 0 5 10" << 5.0;
        QTest::newRow("pin below") << "pin 0 -5 10" << 0.0;
        QTest::newRow("pin above") << "pin 0 15 10" << 10.0;
        QTest::newRow("sqrt") << "sqrt 144" << 12.0;
        QTest::newRow("sqrt of a negative") << "sqrt -4" << 0.0;
        QTest::newRow("divide by zero") << "*/ 1 2 0" << 0.0;
        QTest::newRow("add and divide by zero") << "+/ 1 2 0" << 0.0;
        QTest::newRow("unknown operator") << "frob 1 2 3" << 0.0;
        QTest::newRow("missing arguments") << "+-" << 0.0;
        QTest::newRow("empty") << "" << 0.0;
        QTest::newRow("extra spaces") << "  */   6   7  2 " << 21.0;
        QTest::newRow("a name never defined") << "+- nothing 3 0" << 3.0;
    }

    void dmlGuideFormulas()
    {
        QFETCH(QString, formula);
        QFETCH(double, expected);
        GuideEvaluator g(1000, 600);
        g.define(QStringLiteral("x"), formula);
        QVERIFY2(near(g.value(QStringLiteral("x")), expected, 1e-9), qPrintable(QString::number(g.value(QStringLiteral("x")))));
    }

    void dmlGuideBuiltInNames()
    {
        GuideEvaluator g(1000, 600);
        const struct { const char *name; double value; } names[] = {
            {"w", 1000}, {"h", 600}, {"l", 0}, {"t", 0}, {"r", 1000}, {"b", 600}, {"hc", 500}, {"vc", 300},
            {"wd2", 500}, {"wd3", 1000.0 / 3}, {"wd4", 250}, {"wd5", 200}, {"wd6", 1000.0 / 6}, {"wd8", 125}, {"wd10", 100}, {"wd32", 31.25},
            {"hd2", 300}, {"hd3", 200}, {"hd4", 150}, {"hd5", 120}, {"hd6", 100}, {"hd8", 75},
            {"ss", 600}, {"ls", 1000}, {"ssd2", 300}, {"ssd4", 150}, {"ssd6", 100}, {"ssd8", 75}, {"ssd16", 37.5}, {"ssd32", 18.75},
            {"cd2", 10800000}, {"cd4", 5400000}, {"cd8", 2700000}, {"3cd4", 16200000}, {"3cd8", 8100000}, {"5cd8", 13500000}, {"7cd8", 18900000}};
        for (const auto &n : names) QVERIFY2(near(g.value(QString::fromLatin1(n.name)), n.value), n.name);
        // A number written out is its value, and a name that was never defined is 0.
        QCOMPARE(g.value(QStringLiteral("123")), 123.0);
        QCOMPARE(g.value(QStringLiteral("-4.5")), -4.5);
        QCOMPARE(g.value(QStringLiteral("nothing")), 0.0);
        QCOMPARE(g.value(QStringLiteral("nan")), 0.0);
        QCOMPARE(g.value(QStringLiteral("inf")), 0.0);
        // The shorter side is ss however the shape is turned.
        GuideEvaluator tall(300, 900);
        QCOMPARE(tall.value(QStringLiteral("ss")), 300.0);
        QCOMPARE(tall.value(QStringLiteral("ls")), 900.0);
    }

    void dmlGuidesReferToEachOther()
    {
        GuideEvaluator g(1000, 600);
        g.set(QStringLiteral("adj"), 25000);
        g.define(QStringLiteral("dx"), QStringLiteral("*/ ss adj 100000"));       // 150
        g.define(QStringLiteral("x1"), QStringLiteral("+- l dx 0"));              // 150
        g.define(QStringLiteral("x2"), QStringLiteral("+- r 0 dx"));              // 850
        g.define(QStringLiteral("early"), QStringLiteral("+- late 1 0"));         // a guide used before it is defined
        g.define(QStringLiteral("late"), QStringLiteral("val 5"));
        QCOMPARE(g.value(QStringLiteral("x1")), 150.0);
        QCOMPARE(g.value(QStringLiteral("x2")), 850.0);
        QCOMPARE(g.value(QStringLiteral("early")), 6.0);
        // The first definition of a name is the one that counts.
        g.define(QStringLiteral("x1"), QStringLiteral("val 1"));
        QCOMPARE(g.value(QStringLiteral("x1")), 150.0);
        // A value set outright wins over a formula for the name.
        g.set(QStringLiteral("dx"), 7);
        QCOMPARE(g.value(QStringLiteral("dx")), 7.0);
    }

    void dmlGuidesSurviveLoopsAndExcess()
    {
        GuideEvaluator g(1000, 600);
        g.define(QStringLiteral("a"), QStringLiteral("+- b 1 0"));
        g.define(QStringLiteral("b"), QStringLiteral("+- a 1 0"));
        g.define(QStringLiteral("self"), QStringLiteral("+- self 1 0"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("a"))));
        QVERIFY(std::isfinite(g.value(QStringLiteral("b"))));
        QVERIFY(std::isfinite(g.value(QStringLiteral("self"))));
        // A chain far longer than any drawing has: it ends, and gives a number.
        g.define(QStringLiteral("g0"), QStringLiteral("val 1"));
        for (int i = 1; i < 20000; ++i) g.define(QStringLiteral("g%1").arg(i), QStringLiteral("+- g%1 1 0").arg(i - 1));
        QVERIFY(std::isfinite(g.value(QStringLiteral("g19999"))));
        // Results are kept finite and of a size a path can use.
        g.define(QStringLiteral("big"), QStringLiteral("*/ 1000000000000 1000000000000 1"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("big"))));
        QVERIFY(qAbs(g.value(QStringLiteral("big"))) <= 1e15);
        g.define(QStringLiteral("tan90"), QStringLiteral("tan 100 5400000"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("tan90"))));
        g.define(QStringLiteral("nanner"), QStringLiteral("mod nan 1 1"));
        QVERIFY(std::isfinite(g.value(QStringLiteral("nanner"))));
    }

    void dmlCustomGeometryScalesItsPath()
    {
        // The path's own 100 by 50 is stretched over a frame of 200 by 100 points.
        const QString p = linePath(QStringLiteral("w=\"100\" h=\"50\""), {{"10", "10"}, {"90", "10"}, {"90", "40"}});
        const Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 1);
        const QRectF r = g.paths[0].path.boundingRect();
        QVERIFY2(near(r.left(), 20) && near(r.top(), 20) && near(r.right(), 180) && near(r.bottom(), 80), qPrintable(QString::number(r.left())));
        QVERIFY(g.paths[0].fill == GeometryPath::Normal);
        QVERIFY(g.paths[0].stroke);
    }

    void dmlCustomGeometryWithoutPathSizeIsInEmu()
    {
        const QString p = linePath(QString(), {{"0", "0"}, {QStringLiteral("2540000"), QStringLiteral("1270000")}}, false);
        const Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(300, 300));
        QCOMPARE(g.paths.size(), 1);
        const QRectF r = g.paths[0].path.boundingRect();
        QVERIFY(near(r.right(), 200) && near(r.bottom(), 100));
        // The frame's size doesn't stretch a path that names none.
        const Geometry g2 = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(30, 30));
        QVERIFY(near(g2.paths[0].path.boundingRect().right(), 200));
    }

    void dmlCustomGeometryEvaluatesGuides()
    {
        // A triangle whose apex is a quarter of the way along, by guides.
        const QString gd = QStringLiteral("<a:avLst><a:gd name=\"adj\" fmla=\"val 25000\"/></a:avLst>"
                                          "<a:gdLst><a:gd name=\"x1\" fmla=\"*/ w adj 100000\"/><a:gd name=\"y1\" fmla=\"*/ h 1 2\"/></a:gdLst>");
        const QString p = linePath(QString(), {{"l", "b"}, {"x1", "t"}, {"r", "y1"}});
        const Geometry g = customGeometryOf(gd + QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(400, 200));
        QCOMPARE(g.paths.size(), 1);
        const QPainterPath &path = g.paths[0].path;
        QVERIFY(near(path.elementAt(0).x, 0) && near(path.elementAt(0).y, 200));
        QVERIFY(near(path.elementAt(1).x, 100) && near(path.elementAt(1).y, 0));
        QVERIFY(near(path.elementAt(2).x, 400) && near(path.elementAt(2).y, 100));
    }

    void dmlCustomGeometryDrawsArcsAndCurves()
    {
        // A whole circle from an arc: left-center, once around.
        const QString circle = QStringLiteral("<a:path><a:moveTo><a:pt x=\"l\" y=\"vc\"/></a:moveTo>"
                                              "<a:arcTo wR=\"wd2\" hR=\"hd2\" stAng=\"cd2\" swAng=\"21600000\"/><a:close/></a:path>");
        Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(circle), QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        QRectF r = g.paths[0].path.boundingRect();
        QVERIFY2(near(r.left(), 0, 1e-3) && near(r.top(), 0, 1e-3) && near(r.right(), 100, 1e-3) && near(r.bottom(), 100, 1e-3),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height())));
        // The center of the circle is inside it and a corner is not.
        QVERIFY(g.paths[0].path.contains(QPointF(50, 50)));
        QVERIFY(!g.paths[0].path.contains(QPointF(3, 3)));
        // A quarter ellipse: from the left-center sweeping a quarter turn clockwise ends at the top-center.
        const QString quarter = QStringLiteral("<a:path><a:moveTo><a:pt x=\"l\" y=\"vc\"/></a:moveTo>"
                                               "<a:arcTo wR=\"wd2\" hR=\"hd2\" stAng=\"cd2\" swAng=\"5400000\"/></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(quarter), QSizeF(300, 100));
        const QPointF end = g.paths[0].path.currentPosition();
        QVERIFY2(near(end.x(), 150, 1e-3) && near(end.y(), 0, 1e-3), qPrintable(QStringLiteral("%1,%2").arg(end.x()).arg(end.y())));
        // A counterclockwise quarter turn ends at the bottom-center.
        const QString back = QStringLiteral("<a:path><a:moveTo><a:pt x=\"l\" y=\"vc\"/></a:moveTo>"
                                            "<a:arcTo wR=\"wd2\" hR=\"hd2\" stAng=\"cd2\" swAng=\"-5400000\"/></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(back), QSizeF(300, 100));
        QVERIFY(near(g.paths[0].path.currentPosition().x(), 150, 1e-3) && near(g.paths[0].path.currentPosition().y(), 100, 1e-3));
        // Quadratic and cubic curves end where they are told to.
        const QString curves = QStringLiteral("<a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo>"
                                              "<a:quadBezTo><a:pt x=\"5\" y=\"0\"/><a:pt x=\"10\" y=\"5\"/></a:quadBezTo>"
                                              "<a:cubicBezTo><a:pt x=\"10\" y=\"8\"/><a:pt x=\"8\" y=\"10\"/><a:pt x=\"5\" y=\"10\"/></a:cubicBezTo></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(curves), QSizeF(100, 100));
        QVERIFY(near(g.paths[0].path.currentPosition().x(), 50) && near(g.paths[0].path.currentPosition().y(), 100));
        QVERIFY(g.paths[0].path.elementCount() >= 6);
    }

    void dmlCustomGeometryKeepsFillAndStrokeModes()
    {
        const QString sq = QStringLiteral("<a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:lnTo><a:pt x=\"10\" y=\"0\"/></a:lnTo><a:lnTo><a:pt x=\"10\" y=\"10\"/></a:lnTo>");
        const QString paths = QStringLiteral("<a:pathLst><a:path w=\"10\" h=\"10\">%1</a:path><a:path w=\"10\" h=\"10\" fill=\"none\">%1</a:path>"
                                             "<a:path w=\"10\" h=\"10\" stroke=\"0\">%1</a:path><a:path w=\"10\" h=\"10\" fill=\"darkenLess\" stroke=\"false\">%1</a:path>"
                                             "<a:path w=\"10\" h=\"10\" fill=\"lighten\">%1</a:path><a:path w=\"10\" h=\"10\" fill=\"lightenLess\">%1</a:path>"
                                             "<a:path w=\"10\" h=\"10\" fill=\"darken\">%1</a:path></a:pathLst>").arg(sq);
        const Geometry g = customGeometryOf(paths, QSizeF(10, 10));
        QCOMPARE(g.paths.size(), 7);
        QVERIFY(g.paths[0].fill == GeometryPath::Normal && g.paths[0].stroke);
        QVERIFY(g.paths[1].fill == GeometryPath::None && g.paths[1].stroke);
        QVERIFY(g.paths[2].fill == GeometryPath::Normal && !g.paths[2].stroke);
        QVERIFY(g.paths[3].fill == GeometryPath::DarkenLess && !g.paths[3].stroke);
        QVERIFY(g.paths[4].fill == GeometryPath::Lighten);
        QVERIFY(g.paths[5].fill == GeometryPath::LightenLess);
        QVERIFY(g.paths[6].fill == GeometryPath::Darken);
    }

    void dmlCustomGeometryReadsTheTextRectangle()
    {
        const QString p = linePath(QStringLiteral("w=\"10\" h=\"10\""), {{"0", "0"}, {"10", "0"}, {"10", "10"}});
        Geometry g;
        g = customGeometryOf(QStringLiteral("<a:rect l=\"wd4\" t=\"hd4\" r=\"r\" b=\"b\"/><a:pathLst>%1</a:pathLst>").arg(p), QSizeF(200, 100));
        QVERIFY2(near(g.textRect.left(), 50) && near(g.textRect.top(), 25) && near(g.textRect.right(), 200) && near(g.textRect.bottom(), 100),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(g.textRect.left()).arg(g.textRect.top()).arg(g.textRect.width()).arg(g.textRect.height())));
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(200, 100));
        QVERIFY(g.textRect.isNull());
    }

    void dmlCustomGeometryToleratesNonsense()
    {
        QVERIFY(customGeometryOf(QString(), QSizeF(10, 10)).isEmpty());
        QVERIFY(customGeometryOf(QStringLiteral("<a:pathLst/>"), QSizeF(10, 10)).isEmpty());
        QVERIFY(readCustomGeometry(QDomElement(), QSizeF(10, 10)).isEmpty());
        // Garbage coordinates are 0, a path with no points is skipped, a zero-size frame has nothing.
        Geometry g = customGeometryOf(QStringLiteral("<a:pathLst><a:path w=\"10\" h=\"10\"><a:moveTo><a:pt x=\"zz\" y=\"\"/></a:moveTo>"
                                                     "<a:lnTo/><a:lnTo><a:pt x=\"5\"/></a:lnTo><a:bogus/></a:path><a:path/></a:pathLst>"), QSizeF(10, 10));
        QVERIFY(g.paths.size() <= 1);
        if (!g.paths.isEmpty()) QVERIFY(g.paths[0].path.boundingRect().width() >= 0);
        const QString p = linePath(QStringLiteral("w=\"10\" h=\"10\""), {{"0", "0"}, {"10", "10"}});
        QVERIFY(customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(0, 0)).isEmpty() ||
                customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(p), QSizeF(0, 0)).paths[0].path.boundingRect().isEmpty());
    }

    void dmlCustomGeometryLimitsHostilePaths()
    {
        // Far more commands than a drawing has are cut off, and the points stay where a page can hold them.
        QString many = QStringLiteral("<a:path w=\"100\" h=\"100\"><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo>");
        for (int i = 0; i < 150000; ++i) many += QStringLiteral("<a:lnTo><a:pt x=\"%1\" y=\"5\"/></a:lnTo>").arg(i % 100);
        many += QStringLiteral("</a:path>");
        Geometry g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(many), QSizeF(100, 100));
        QVERIFY(g.paths.isEmpty() || g.paths[0].path.elementCount() <= 100000);
        const QString far = linePath(QString(), {{"0", "0"}, {QStringLiteral("99999999999999999"), QStringLiteral("-99999999999999999")}}, false);
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(far), QSizeF(100, 100));
        QVERIFY(g.paths.isEmpty() || g.paths[0].path.boundingRect().right() <= 1e6);
        QVERIFY(g.paths.isEmpty() || g.paths[0].path.boundingRect().top() >= -1e6);
        // An arc with radii of nothing, or of billions, neither hangs nor makes a bad number.
        const QString arcs = QStringLiteral("<a:path><a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo><a:arcTo wR=\"0\" hR=\"0\" stAng=\"0\" swAng=\"5400000\"/>"
                                            "<a:arcTo wR=\"99999999999\" hR=\"99999999999\" stAng=\"0\" swAng=\"99999999999\"/></a:path>");
        g = customGeometryOf(QStringLiteral("<a:pathLst>%1</a:pathLst>").arg(arcs), QSizeF(100, 100));
        for (const GeometryPath &p : g.paths)
            for (int i = 0; i < p.path.elementCount(); ++i) QVERIFY(std::isfinite(p.path.elementAt(i).x) && std::isfinite(p.path.elementAt(i).y));
        // Too many guides are ignored from the limit on.
        QString gds;
        for (int i = 0; i < 20000; ++i) gds += QStringLiteral("<a:gd name=\"g%1\" fmla=\"val %1\"/>").arg(i);
        g = customGeometryOf(QStringLiteral("<a:gdLst>%1</a:gdLst><a:pathLst>%2</a:pathLst>").arg(gds, linePath(QString(), {{"g1", "g2"}, {"g3", "g4"}})), QSizeF(100, 100));
        QVERIFY(g.paths.size() <= 1);
    }

    // The whole of an import draws from one budget: what two reads keep between them is what one would.
    void dmlSharedBudgetHoldsWhatManyReadsKeep()
    {
        DmlEnv env;
        DrawingBudget budget;
        budget.left[DrawingBudget::Objects] = 10;
        env.ctx.budget = &budget;
        const QString leaf = dmlSp(dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        QString six;
        for (int i = 0; i < 6; ++i) six += leaf;
        auto count = [&](const ItemList &list) {
            int n = 0;
            jp::walkItems(list, [&](const ItemPtr &) { ++n; });
            return n;
        };
        QCOMPARE(count(dmlTree(env, six)), 6);
        QVERIFY(!budget.ranOut[DrawingBudget::Objects]);
        QCOMPARE(count(dmlTree(env, six)), 4);   // the second read gets what is left
        QVERIFY(budget.ranOut[DrawingBudget::Objects]);
        QCOMPARE(budget.left[DrawingBudget::Objects], qint64(0));
        QCOMPARE(count(dmlTree(env, six)), 0);
        QVERIFY(!dmlOne(env, leaf));
        // A group costs one, and what is in it one each.
        DmlEnv again;
        DrawingBudget small;
        small.left[DrawingBudget::Objects] = 3;
        again.ctx.budget = &small;
        const QString frame = dmlGroupFrame(QRectF(0, 0, 100, 100), QRectF(0, 0, 100, 100));
        const ItemList grouped = dmlTree(again, dmlGrp(frame, leaf + leaf + leaf));
        QCOMPARE(count(grouped), 3);   // the group, and two of its three
        QVERIFY(small.ranOut[DrawingBudget::Objects]);
        // With no budget a read has only its own limits.
        DmlEnv free;
        QCOMPARE(count(dmlTree(free, six + six + six)), 18);
    }

    // A text box or a shape's text is a story; past the budget's stories a box is left out, and a shape stays without its text.
    void dmlSharedBudgetHoldsStoriesAndCells()
    {
        DmlEnv env;
        DrawingBudget budget;
        budget.left[DrawingBudget::Objects] = 100;
        budget.left[DrawingBudget::Stories] = 2;
        env.ctx.budget = &budget;
        const QString box = dmlSp(dmlXfrm(0, 0, 100, 30) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"), dmlBody(), QString(), QStringLiteral("txBox=\"1\""));
        const QString ellipse = dmlSp(dmlXfrm(0, 0, 100, 30) + QStringLiteral("<a:prstGeom prst=\"ellipse\"/>"), dmlBody());
        const ItemList boxes = dmlTree(env, box + box + box + box);
        QCOMPARE(int(boxes.size()), 2);
        QCOMPARE(int(env.stories.size()), 2);
        QVERIFY(budget.ranOut[DrawingBudget::Stories]);
        const ItemList shapes = dmlTree(env, ellipse);
        QCOMPARE(int(shapes.size()), 1);
        QVERIFY(static_cast<ShapeItem &>(*shapes[0]).storyId.isEmpty());
        QCOMPARE(int(env.stories.size()), 2);

        // A table takes a cell each; one that would pass the budget is left to the caller.
        DmlEnv tables;
        DrawingBudget cells;
        cells.left[DrawingBudget::Cells] = 7;
        tables.ctx.budget = &cells;
        const QString grid = QStringLiteral("<a:tblGrid><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/><a:gridCol w=\"%1\"/></a:tblGrid>").arg(emu(50));
        auto row = [&] { return QStringLiteral("<a:tr h=\"%1\">%2%2%2</a:tr>").arg(emu(20), dmlTc(QStringLiteral("x"))); };
        const QString twoRows = dmlTableFrame(QStringLiteral("<a:tblPr/>"), grid, row() + row(), 0, 0, 150, 40);
        QVERIFY(!readTable(tables.element(twoRows), tables.ctx));   // 6 cells and the table's own two, 7 left
        QVERIFY(cells.ranOut[DrawingBudget::Cells]);
        QCOMPARE(cells.left[DrawingBudget::Cells], qint64(7));
        QCOMPARE(int(tables.stories.size()), 0);
        const QString oneRow = dmlTableFrame(QStringLiteral("<a:tblPr/>"), grid, row(), 0, 0, 150, 20);
        QVERIFY(readTable(tables.element(oneRow), tables.ctx));
        QCOMPARE(cells.left[DrawingBudget::Cells], qint64(2));   // 3 cells and the two
        QCOMPARE(int(tables.stories.size()), 3);
    }

    // The paragraphs, runs, and letters of every text body of an import come out of one budget.
    void dmlSharedBudgetHoldsText()
    {
        auto read = [](DrawingBudget *budget, const QString &paragraphs) {
            QDomDocument dom;
            const QString xml = QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle/>%2</p:txBody>").arg(dmlNamespaces(), paragraphs);
            if (!parseXml(xml.toUtf8(), &dom)) qWarning("bad test XML");
            DrawingContext ctx;
            ctx.budget = budget;
            auto doc = std::make_unique<QTextDocument>();
            readTextBody(dom.documentElement(), doc.get(), ctx);
            return doc;
        };
        auto paragraph = [](const QString &text) { return QStringLiteral("<a:p><a:r><a:t>%1</a:t></a:r></a:p>").arg(text); };
        DrawingBudget budget;
        budget.left[DrawingBudget::Paragraphs] = 5;
        budget.left[DrawingBudget::Runs] = 1000;
        budget.left[DrawingBudget::Letters] = 1000;
        QString eight;
        for (int i = 0; i < 8; ++i) eight += paragraph(QStringLiteral("p%1").arg(i));
        QCOMPARE(read(&budget, eight)->blockCount(), 5);
        QVERIFY(budget.ranOut[DrawingBudget::Paragraphs]);
        // What is left when a body is read again is none: the body is one empty paragraph.
        QCOMPARE(read(&budget, eight)->toPlainText(), QString());
        QCOMPARE(read(&budget, eight)->blockCount(), 1);

        DrawingBudget letters;
        letters.left[DrawingBudget::Paragraphs] = 100;
        letters.left[DrawingBudget::Runs] = 100;
        letters.left[DrawingBudget::Letters] = 7;
        const auto cut = read(&letters, paragraph(QStringLiteral("abcd")) + paragraph(QStringLiteral("efgh")) + paragraph(QStringLiteral("ijkl")));
        QCOMPARE(cut->toPlainText().remove(QLatin1Char('\n')).remove(QChar::ParagraphSeparator), QStringLiteral("abcdefg"));
        QVERIFY(letters.ranOut[DrawingBudget::Letters]);
        QCOMPARE(letters.left[DrawingBudget::Letters], qint64(0));

        DrawingBudget runs;
        runs.left[DrawingBudget::Paragraphs] = 100;
        runs.left[DrawingBudget::Runs] = 4;
        runs.left[DrawingBudget::Letters] = 100;
        QString six;
        for (int i = 0; i < 6; ++i) six += QStringLiteral("<a:r><a:t>a</a:t></a:r>");
        QCOMPARE(read(&runs, QStringLiteral("<a:p>%1</a:p>").arg(six))->toPlainText(), QStringLiteral("aaaa"));
        QVERIFY(runs.ranOut[DrawingBudget::Runs]);
    }

    // One run longer than a body may be is cut where the body ends (real bodies have a few thousand letters at most).
    void dmlTextBodyCutsARunPastTheLettersOfABody()
    {
        QDomDocument dom;
        const QString xml = QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>%2</a:t></a:r></a:p></p:txBody>").arg(dmlNamespaces(), QString(2000100, QLatin1Char('z')));
        QVERIFY(parseXml(xml.toUtf8(), &dom));
        QTextDocument doc;
        readTextBody(dom.documentElement(), &doc, DrawingContext());
        QCOMPARE(doc.characterCount() - 1, 2000000);
        // With a budget, which has plenty left, it says that a body was cut, and that nothing ran out.
        DrawingBudget budget;
        for (qint64 &left : budget.left) left = 100000000;
        DrawingContext ctx;
        ctx.budget = &budget;
        readTextBody(dom.documentElement(), &doc, ctx);
        QCOMPARE(doc.characterCount() - 1, 2000000);
        QVERIFY(budget.longBody);
        for (bool out : budget.ranOut) QVERIFY(!out);
        QCOMPARE(budget.left[DrawingBudget::Letters], qint64(100000000 - 2000000));
    }

    // Qt adds a block to a list in time proportional to the list's length, so one long bulleted run is read as lists of a few hundred each,
    // numbered on from one to the next.
    void dmlBulletedParagraphsAreReadInLinearTime()
    {
        auto read = [](const QString &pPr, int count) {
            QString paragraphs;
            for (int i = 0; i < count; ++i) paragraphs += QStringLiteral("<a:p><a:pPr marL=\"285750\" indent=\"-285750\">%1</a:pPr><a:r><a:t>x</a:t></a:r></a:p>").arg(pPr);
            QDomDocument dom;
            const QString xml = QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle/>%2</p:txBody>").arg(dmlNamespaces(), paragraphs);
            if (!parseXml(xml.toUtf8(), &dom)) qWarning("bad test XML");
            auto doc = std::make_unique<QTextDocument>();
            readTextBody(dom.documentElement(), doc.get(), DrawingContext());
            return doc;
        };
        decltype(read(QStringLiteral("<a:buChar char=\"&#8226;\"/>"), 10000)) bullets;
        bullets = read(QStringLiteral("<a:buChar char=\"&#8226;\"/>"), 10000);
        QCOMPARE(bullets->blockCount(), 10000);
        QHash<QTextList *, int> lists;
        for (QTextBlock b = bullets->begin(); b.isValid(); b = b.next()) {
            QVERIFY(b.textList());
            ++lists[b.textList()];
        }
        int longest = 0;
        for (int blocks : std::as_const(lists)) longest = std::max(longest, blocks);
        QVERIFY2(longest <= 500 && lists.size() >= 20, qPrintable(QStringLiteral("%1 lists, the longest of %2").arg(lists.size()).arg(longest)));
        // (What made it slow was each paragraph joining one list of thousands, an insertion that costs the list's length: no list is longer than 500, counted above.)
        // Counted on across the lists they are split into.
        const auto numbered = read(QStringLiteral("<a:buAutoNum type=\"arabicPeriod\"/>"), 1300);
        QCOMPARE(numbered->blockCount(), 1300);
        QCOMPARE(numbered->begin().textList()->itemText(numbered->begin()), QStringLiteral("1."));
        const QTextBlock last = numbered->findBlockByNumber(1299);
        QCOMPARE(last.textList()->itemText(last), QStringLiteral("1300."));
        const QTextBlock middle = numbered->findBlockByNumber(700);
        QCOMPARE(middle.textList()->itemText(middle), QStringLiteral("701."));
        const auto from5 = read(QStringLiteral("<a:buAutoNum type=\"arabicPeriod\" startAt=\"5\"/>"), 1300);
        const QTextBlock end = from5->findBlockByNumber(1299);
        QCOMPARE(end.textList()->itemText(end), QStringLiteral("1304."));
    }

    // One cache of pictures across every read of a file: the same picture on the next shape, or the next page, is handed over once.
    void dmlPicturesAreKeptOnceAcrossReads()
    {
        DmlEnv env;
        env.usePackage();
        QHash<QString, QString> kept;
        env.ctx.images = &kept;
        QString pics;
        for (int i = 0; i < 5; ++i) pics += dmlPic(QString(), dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"));
        for (int page = 0; page < 3; ++page) {
            const ItemList list = dmlTree(env, pics);
            QCOMPARE(int(list.size()), 5);
            QCOMPARE(static_cast<PictureItem &>(*list[4]).imageId, static_cast<PictureItem &>(*list[0]).imageId);
        }
        for (int page = 0; page < 3; ++page) QVERIFY(dmlOne(env, dmlPic(QString(), dmlXfrm(0, 0, 10, 10) + QStringLiteral("<a:prstGeom prst=\"rect\"/>"))));
        QCOMPARE(env.images.size(), 1);   // addImage was called once, for all 18
        QCOMPARE(kept.size(), 1);
    }

private:

    // The presets that no shape and no definition here draws as they should be: the ones the report lists.
    static QStringList dmlUnsupportedPresets()
    {
        return QStringLiteral("leftRightUpArrow quadArrowCallout leftCircularArrow leftRightCircularArrow swooshArrow "
                              "leftRightRibbon gear6 gear9 funnel cornerTabs squareTabs plaqueTabs chartX chartStar chartPlus").split(QLatin1Char(' '));
    }

private Q_SLOTS:
    void dmlPresetNamesMapToTheShapeLibrary_data()
    {
        QTest::addColumn<QString>("preset");
        QTest::addColumn<QString>("shape");
        const struct { const char *preset, *shape; } map[] = {
            {"rect", "rect"}, {"roundRect", "roundRect"}, {"ellipse", "ellipse"}, {"triangle", "triangle"}, {"rtTriangle", "rtTriangle"},
            {"diamond", "diamond"}, {"parallelogram", "parallelogram"}, {"trapezoid", "trapezoid"}, {"pentagon", "pentagon"},
            {"hexagon", "hexagon"}, {"heptagon", "heptagon"}, {"octagon", "octagon"}, {"decagon", "decagon"}, {"dodecagon", "dodecagon"},
            {"snip1Rect", "snip1"}, {"snip2SameRect", "snip2same"}, {"snip2DiagRect", "snip2diag"}, {"snipRoundRect", "snipRound"},
            {"round1Rect", "round1"}, {"round2SameRect", "round2same"}, {"round2DiagRect", "round2diag"},
            {"plaque", "plaque"}, {"teardrop", "teardrop"}, {"homePlate", "homePlate"}, {"chevron", "chevron"}, {"pie", "pie"}, {"chord", "chord"},
            {"blockArc", "blockArc"}, {"arc", "arc"}, {"donut", "donut"}, {"noSmoking", "noSmoking"}, {"frame", "frame"}, {"halfFrame", "halfFrame"},
            {"corner", "corner"}, {"diagStripe", "diagStripe"}, {"plus", "plus"}, {"can", "can"}, {"cube", "cube"}, {"bevel", "bevel"},
            {"foldedCorner", "foldedCorner"}, {"smileyFace", "smiley"}, {"heart", "heart"}, {"lightningBolt", "lightning"}, {"sun", "sun"},
            {"moon", "moon"}, {"cloud", "cloud"}, {"bracketPair", "bracketPair"}, {"bracePair", "bracePair"}, {"leftBracket", "leftBracket"},
            {"rightBracket", "rightBracket"}, {"leftBrace", "leftBrace"}, {"rightBrace", "rightBrace"},
            {"rightArrow", "rightArrow"}, {"leftArrow", "leftArrow"}, {"upArrow", "upArrow"}, {"downArrow", "downArrow"},
            {"leftRightArrow", "leftRightArrow"}, {"upDownArrow", "upDownArrow"}, {"quadArrow", "quadArrow"}, {"bentArrow", "bentArrow"},
            {"uturnArrow", "uturnArrow"}, {"leftUpArrow", "leftUpArrow"}, {"bentUpArrow", "bentUpArrow"}, {"curvedRightArrow", "curvedRightArrow"},
            {"curvedLeftArrow", "curvedLeftArrow"}, {"curvedUpArrow", "curvedUpArrow"}, {"curvedDownArrow", "curvedDownArrow"},
            {"stripedRightArrow", "stripedRightArrow"}, {"notchedRightArrow", "notchedRightArrow"}, {"rightArrowCallout", "rightArrowCallout"},
            {"downArrowCallout", "downArrowCallout"}, {"circularArrow", "circularArrow"},
            {"mathPlus", "mathPlus"}, {"mathMinus", "mathMinus"}, {"mathMultiply", "mathMultiply"}, {"mathDivide", "mathDivide"},
            {"mathEqual", "mathEqual"}, {"mathNotEqual", "mathNotEqual"},
            {"flowChartProcess", "fcProcess"}, {"flowChartAlternateProcess", "fcAltProcess"}, {"flowChartDecision", "fcDecision"},
            {"flowChartInputOutput", "fcData"}, {"flowChartPredefinedProcess", "fcPredefined"}, {"flowChartInternalStorage", "fcInternalStorage"},
            {"flowChartDocument", "fcDocument"}, {"flowChartMultidocument", "fcMultidocument"}, {"flowChartTerminator", "fcTerminator"},
            {"flowChartPreparation", "fcPreparation"}, {"flowChartManualInput", "fcManualInput"}, {"flowChartManualOperation", "fcManualOperation"},
            {"flowChartConnector", "fcConnector"}, {"flowChartOffpageConnector", "fcOffpage"}, {"flowChartPunchedCard", "fcCard"},
            {"flowChartPunchedTape", "fcPunchedTape"}, {"flowChartSummingJunction", "fcSummingJunction"}, {"flowChartOr", "fcOr"},
            {"flowChartCollate", "fcCollate"}, {"flowChartSort", "fcSort"}, {"flowChartExtract", "fcExtract"}, {"flowChartMerge", "fcMerge"},
            {"flowChartOnlineStorage", "fcStoredData"}, {"flowChartDelay", "fcDelay"}, {"flowChartMagneticTape", "fcSequential"},
            {"flowChartMagneticDisk", "fcMagneticDisk"}, {"flowChartMagneticDrum", "fcDirectAccess"}, {"flowChartDisplay", "fcDisplay"},
            {"irregularSeal1", "irregularSeal1"}, {"irregularSeal2", "irregularSeal2"},
            {"star4", "star4"}, {"star5", "star5"}, {"star6", "star6"}, {"star7", "star7"}, {"star8", "star8"}, {"star10", "star10"},
            {"star12", "star12"}, {"star16", "star16"}, {"star24", "star24"}, {"star32", "star32"},
            {"ribbon", "ribbon"}, {"ribbon2", "ribbon2"}, {"ellipseRibbon", "ellipseRibbon"}, {"ellipseRibbon2", "ellipseRibbon2"},
            {"verticalScroll", "verticalScroll"}, {"horizontalScroll", "horizontalScroll"}, {"wave", "wave"}, {"doubleWave", "doubleWave"},
            {"wedgeRectCallout", "wedgeRectCallout"}, {"wedgeRoundRectCallout", "wedgeRoundRectCallout"}, {"wedgeEllipseCallout", "wedgeEllipseCallout"},
            {"borderCallout1", "borderCallout1"}, {"borderCallout2", "borderCallout2"}, {"accentCallout1", "accentCallout1"}, {"callout1", "callout1"},
            {"actionButtonBlank", "actionButtonBlank"}, {"actionButtonHome", "actionButtonBlank"}, {"actionButtonSound", "actionButtonBlank"}};
        for (const auto &m : map) QTest::newRow(m.preset) << QString::fromLatin1(m.preset) << QString::fromLatin1(m.shape);
    }

    void dmlPresetNamesMapToTheShapeLibrary()
    {
        QFETCH(QString, preset);
        QFETCH(QString, shape);
        QCOMPARE(shapeForPreset(preset), shape);
        QVERIFY2(jp::shapeDef(shape) != nullptr, qPrintable(shape));
    }

    void dmlEveryShapeTheLibraryDrawsHasAPreset()
    {
        // The library holds two shapes the standard doesn't name (a tab and a flow arrow);
        // every other has a preset that maps to it.
        QSet<QString> reached;
        for (const QString &prst : dmlAllPresets()) {
            const QString id = shapeForPreset(prst);
            if (!id.isEmpty()) {
                QVERIFY2(jp::shapeDef(id) != nullptr, qPrintable(prst));
                reached.insert(id);
            }
        }
        for (const jp::ShapeDef &d : jp::shapeLibrary())
            if (d.id != QLatin1String("tab") && d.id != QLatin1String("flowArrow")) QVERIFY2(reached.contains(d.id), qPrintable(d.id));
    }

    void dmlEveryPresetHasAShapeOrADefinitionOrIsListed()
    {
        const QStringList unsupported = dmlUnsupportedPresets();
        for (const QString &prst : dmlAllPresets()) {
            const bool line = prst == QLatin1String("line") || prst == QLatin1String("lineInv") || prst == QLatin1String("straightConnector1");
            const bool drawn = !shapeForPreset(prst).isEmpty() || hasPresetGeometry(prst) || line;
            QVERIFY2(drawn != unsupported.contains(prst), qPrintable(prst));
        }
        QVERIFY(shapeForPreset(QStringLiteral("noSuchShape")).isEmpty());
        QVERIFY(shapeForPreset(QString()).isEmpty());
        QVERIFY(shapeForPreset(QStringLiteral("Rect")).isEmpty());   // names are case sensitive
    }

private:
    static QHash<QString, double> dmlAdjust(const QString &list)
    {
        QHash<QString, double> out;
        for (const QString &item : list.split(QLatin1Char(';'), Qt::SkipEmptyParts)) out.insert(item.section(QLatin1Char('='), 0, 0), item.section(QLatin1Char('='), 1).toDouble());
        return out;
    }

private Q_SLOTS:
    void dmlLibraryAdjustments_data()
    {
        QTest::addColumn<QString>("preset");
        QTest::addColumn<QString>("given");
        QTest::addColumn<double>("width");
        QTest::addColumn<double>("height");
        QTest::addColumn<QList<double>>("expected");
        using L = QList<double>;
        QTest::newRow("roundRect default") << "roundRect" << "" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("roundRect half") << "roundRect" << "adj=50000" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("roundRect past half") << "roundRect" << "adj=75000" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("roundRect negative") << "roundRect" << "adj=-100" << 100.0 << 50.0 << L{0.0};
        QTest::newRow("roundRect huge") << "roundRect" << "adj=999999999999" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("triangle default") << "triangle" << "" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("triangle apex at the left") << "triangle" << "adj=0" << 100.0 << 50.0 << L{0.0};
        QTest::newRow("triangle apex at the right") << "triangle" << "adj=100000" << 100.0 << 50.0 << L{1.0};
        QTest::newRow("parallelogram wide") << "parallelogram" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("parallelogram tall") << "parallelogram" << "" << 50.0 << 100.0 << L{0.25};
        QTest::newRow("trapezoid") << "trapezoid" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("trapezoid set") << "trapezoid" << "adj=50000" << 200.0 << 100.0 << L{0.25};
        QTest::newRow("hexagon") << "hexagon" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("octagon") << "octagon" << "" << 100.0 << 50.0 << L{0.29289};
        QTest::newRow("plus") << "plus" << "" << 100.0 << 50.0 << L{0.25};
        QTest::newRow("can") << "can" << "adj=40000" << 100.0 << 50.0 << L{0.4};
        QTest::newRow("cube") << "cube" << "" << 100.0 << 50.0 << L{0.25};
        QTest::newRow("bevel") << "bevel" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("donut") << "donut" << "" << 100.0 << 50.0 << L{0.25};
        QTest::newRow("frame") << "frame" << "" << 100.0 << 50.0 << L{0.125};
        QTest::newRow("foldedCorner") << "foldedCorner" << "" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("chevron") << "chevron" << "" << 100.0 << 50.0 << L{0.5};
        QTest::newRow("homePlate") << "homePlate" << "adj=30000" << 100.0 << 50.0 << L{0.3};
        QTest::newRow("rightArrow default") << "rightArrow" << "" << 100.0 << 50.0 << L{0.5, 0.5};
        QTest::newRow("rightArrow set") << "rightArrow" << "adj1=25000;adj2=75000" << 100.0 << 50.0 << L{0.25, 0.75};
        QTest::newRow("leftArrow") << "leftArrow" << "adj1=30000" << 100.0 << 50.0 << L{0.3, 0.5};
        QTest::newRow("upArrow") << "upArrow" << "adj2=20000" << 100.0 << 50.0 << L{0.5, 0.2};
        QTest::newRow("leftRightArrow") << "leftRightArrow" << "" << 100.0 << 50.0 << L{0.5, 0.5};
        QTest::newRow("quadArrow") << "quadArrow" << "" << 100.0 << 100.0 << L{0.1125, 0.225, 0.225};
        QTest::newRow("star4") << "star4" << "" << 100.0 << 100.0 << L{0.125};
        QTest::newRow("star5") << "star5" << "" << 100.0 << 100.0 << L{0.38196};
        QTest::newRow("star5 set") << "star5" << "adj=25000" << 100.0 << 100.0 << L{0.5};
        QTest::newRow("star6") << "star6" << "" << 100.0 << 100.0 << L{0.57736};
        QTest::newRow("star24") << "star24" << "" << 100.0 << 100.0 << L{0.75};
        QTest::newRow("mathPlus") << "mathPlus" << "" << 100.0 << 100.0 << L{0.2352};
        QTest::newRow("wave") << "wave" << "" << 100.0 << 100.0 << L{0.125};
        QTest::newRow("doubleWave") << "doubleWave" << "" << 100.0 << 100.0 << L{0.0625};
        QTest::newRow("verticalScroll") << "verticalScroll" << "" << 100.0 << 100.0 << L{0.125};
        QTest::newRow("smileyFace") << "smileyFace" << "" << 100.0 << 100.0 << L{0.04653};
        QTest::newRow("sun") << "sun" << "" << 100.0 << 100.0 << L{0.25};
        QTest::newRow("moon") << "moon" << "" << 100.0 << 100.0 << L{0.5};
        QTest::newRow("snip1Rect") << "snip1Rect" << "adj=30000" << 100.0 << 50.0 << L{0.3};
        QTest::newRow("round2DiagRect") << "round2DiagRect" << "" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("a name the preset doesn't use") << "roundRect" << "adj9=5" << 100.0 << 50.0 << L{0.16667};
        QTest::newRow("no adjustments: rect") << "rect" << "adj=5" << 100.0 << 50.0 << L{};
        QTest::newRow("no adjustments: ellipse") << "ellipse" << "" << 100.0 << 50.0 << L{};
        QTest::newRow("no such preset") << "noSuchShape" << "adj=5" << 100.0 << 50.0 << L{};
    }

    void dmlLibraryAdjustments()
    {
        QFETCH(QString, preset);
        QFETCH(QString, given);
        QFETCH(double, width);
        QFETCH(double, height);
        QFETCH(QList<double>, expected);
        const QVector<double> got = libraryAdjustments(preset, dmlAdjust(given), QSizeF(width, height));
        QCOMPARE(got.size(), expected.size());
        for (int i = 0; i < expected.size(); ++i) QVERIFY2(qAbs(got[i] - expected[i]) < 1e-4, qPrintable(QStringLiteral("%1: %2 not %3").arg(i).arg(got[i]).arg(expected[i])));
    }

    void dmlLibraryAdjustmentsStayInRange()
    {
        // Whatever a file says, a library shape gets numbers in the range its handle has.
        const QList<double> hostile = {-1e18, -100000, -1, 0, 1, 99999, 100000, 250000, 1e18};
        for (const QString &prst : dmlAllPresets()) {
            const QString id = shapeForPreset(prst);
            const jp::ShapeDef *def = jp::shapeDef(id);
            if (!def) continue;
            for (double v : hostile) {
                QHash<QString, double> given;
                for (const char *name : {"adj", "adj1", "adj2", "adj3", "adj4", "adj5"}) given.insert(QString::fromLatin1(name), v);
                const QVector<double> adj = libraryAdjustments(prst, given, QSizeF(300, 120));
                // A block arc keeps all three of the standard's adjustments (a file needs them), though the library has one.
                const qsizetype most = prst == QLatin1String("blockArc") ? 3 : def->defaults.size() + 1;
                QVERIFY2(adj.size() <= most, qPrintable(prst));
                for (double a : adj) QVERIFY2(std::isfinite(a) && a >= -1 && a <= 3, qPrintable(QStringLiteral("%1 %2 %3").arg(prst).arg(v).arg(a)));
                // And the shape can be drawn with them (a shape with a thickness of nothing may show nothing).
                const QPainterPath path = jp::shapePath(id, QSizeF(300, 120), adj);
                for (int i = 0; i < path.elementCount(); ++i) QVERIFY2(std::isfinite(path.elementAt(i).x) && std::isfinite(path.elementAt(i).y), qPrintable(prst));
            }
        }
    }

    void dmlPieAndChordFollowTheirAngles()
    {
        // A pie from the standard's defaults is three quarters, clockwise from the right: the top right is missing.
        Geometry g = presetGeometry(QStringLiteral("pie"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        QVERIFY(g.paths[0].path.contains(QPointF(25, 25)));
        QVERIFY(g.paths[0].path.contains(QPointF(75, 75)));
        QVERIFY(g.paths[0].path.contains(QPointF(25, 75)));
        QVERIFY(!g.paths[0].path.contains(QPointF(75, 25)));
        // A quarter from the right to the bottom.
        g = presetGeometry(QStringLiteral("pie"), {{"adj1", 0}, {"adj2", 5400000}}, QSizeF(100, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(75, 75)));
        QVERIFY(!g.paths[0].path.contains(QPointF(25, 25)));
        QVERIFY(!g.paths[0].path.contains(QPointF(75, 25)));
        // The same angles on a wide ellipse still bound the same quarter.
        g = presetGeometry(QStringLiteral("pie"), {{"adj1", 0}, {"adj2", 5400000}}, QSizeF(300, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(225, 75)));
        QVERIFY(!g.paths[0].path.contains(QPointF(75, 25)));
        const QRectF b = g.paths[0].path.boundingRect();
        QVERIFY2(near(b.left(), 150, 1e-3) && near(b.top(), 50, 1e-3) && near(b.right(), 300, 1e-3) && near(b.bottom(), 100, 1e-3),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(b.left()).arg(b.top()).arg(b.width()).arg(b.height())));
        // A chord, from 45 to 270 degrees, is cut off by a straight line from the top to the lower right.
        g = presetGeometry(QStringLiteral("chord"), {}, QSizeF(100, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(20, 50)));
        QVERIFY(!g.paths[0].path.contains(QPointF(80, 30)));
        // Equal start and end angles are nothing odd, and no number makes it fail.
        for (double v : {-1e18, -1.0, 0.0, 21600000.0, 1e18}) {
            g = presetGeometry(QStringLiteral("pie"), {{"adj1", v}, {"adj2", v}}, QSizeF(100, 100));
            QVERIFY(!g.paths.isEmpty());
        }
    }

    void dmlArcDrawsAnOpenArcAndItsSector()
    {
        // The standard's arc is a quarter from the top to the right; its sector is filled, only the arc stroked.
        const Geometry g = presetGeometry(QStringLiteral("arc"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 2);
        QVERIFY(g.paths[0].fill == GeometryPath::Normal && !g.paths[0].stroke);
        QVERIFY(g.paths[1].fill == GeometryPath::None && g.paths[1].stroke);
        const QRectF arc = g.paths[1].path.boundingRect();
        QVERIFY2(near(arc.left(), 50, 1e-3) && near(arc.top(), 0, 1e-3) && near(arc.right(), 100, 1e-3) && near(arc.bottom(), 50, 1e-3),
                 qPrintable(QStringLiteral("%1,%2 %3x%4").arg(arc.left()).arg(arc.top()).arg(arc.width()).arg(arc.height())));
        QVERIFY(near(g.paths[1].path.elementAt(0).x, 50, 1e-3) && near(g.paths[1].path.elementAt(0).y, 0, 1e-3));
        QVERIFY(near(g.paths[1].path.currentPosition().x(), 100, 1e-3) && near(g.paths[1].path.currentPosition().y(), 50, 1e-3));
    }

    void dmlBlockArcIsARingBand()
    {
        const Geometry g = presetGeometry(QStringLiteral("blockArc"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        const QPainterPath &p = g.paths[0].path;
        QVERIFY(p.contains(QPointF(50, 20)));      // in the band at the top
        QVERIFY(p.contains(QPointF(10, 45)));      // and at its left end
        QVERIFY(!p.contains(QPointF(50, 40)));     // not in the hole
        QVERIFY(!p.contains(QPointF(50, 70)));     // not in the missing lower half
        // A thinner band.
        const Geometry thin = presetGeometry(QStringLiteral("blockArc"), {{"adj3", 10000}}, QSizeF(100, 100));
        QVERIFY(thin.paths[0].path.contains(QPointF(50, 5)));
        QVERIFY(!thin.paths[0].path.contains(QPointF(50, 20)));
    }

    void dmlWedgeCalloutPointsAtItsTarget()
    {
        // By default the pointer comes from the bottom edge, toward a point below and left of the center.
        Geometry g = presetGeometry(QStringLiteral("wedgeRectCallout"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 1);
        QPainterPath p = g.paths[0].path;
        QVERIFY(p.contains(QPointF(100, 50)));
        QVERIFY(p.contains(QPointF(58, 106)));
        QVERIFY(!p.contains(QPointF(150, 106)));
        QVERIFY(near(p.boundingRect().bottom(), 112.5, 1e-3));
        QVERIFY(near(p.boundingRect().top(), 0) && near(p.boundingRect().left(), 0) && near(p.boundingRect().right(), 200));
        // A pointer to the right of the box.
        g = presetGeometry(QStringLiteral("wedgeRectCallout"), {{"adj1", 90000}, {"adj2", 10000}}, QSizeF(200, 100));
        p = g.paths[0].path;
        QVERIFY(near(p.boundingRect().right(), 280, 1e-3));
        QVERIFY(p.contains(QPointF(240, 65)));
        QVERIFY(!p.contains(QPointF(240, 40)));
        // Rounded and oval ones reach the same point.
        for (const char *name : {"wedgeRoundRectCallout", "wedgeEllipseCallout"}) {
            g = presetGeometry(QString::fromLatin1(name), {{"adj1", -50000}, {"adj2", 90000}}, QSizeF(200, 100));
            QCOMPARE(g.paths.size(), 1);
            const QRectF r = g.paths[0].path.boundingRect();
            QVERIFY2(near(r.bottom(), 140, 1e-3) && r.left() <= 50 + 1e-3 && r.top() >= -0.1,
                     qPrintable(QStringLiteral("%1: %2,%3 %4x%5").arg(QString::fromLatin1(name)).arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height())));
            QVERIFY2(g.paths[0].path.contains(QPointF(100, 40)), name);
        }
    }

    void dmlLineCalloutsHaveABoxAndALeader()
    {
        // borderCallout1: the box is stroked, the line (from outside the box to its target) is not filled.
        Geometry g = presetGeometry(QStringLiteral("borderCallout1"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 2);
        QVERIFY(g.paths[0].stroke && g.paths[0].fill == GeometryPath::Normal);
        QVERIFY(g.paths[1].fill == GeometryPath::None && g.paths[1].stroke);
        QVERIFY(near(g.paths[1].path.elementAt(0).x, -16.666, 1e-3) && near(g.paths[1].path.elementAt(0).y, 18.75, 1e-3));
        QVERIFY(near(g.paths[1].path.elementAt(1).x, -76.666, 1e-3) && near(g.paths[1].path.elementAt(1).y, 112.5, 1e-3));
        // callout1 has no border; accentCallout1 an accent bar too; with a border as well, the box is stroked.
        g = presetGeometry(QStringLiteral("callout1"), {}, QSizeF(200, 100));
        QVERIFY(!g.paths[0].stroke);
        g = presetGeometry(QStringLiteral("accentCallout1"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths.size(), 3);
        QVERIFY(!g.paths[0].stroke);
        QVERIFY(near(g.paths[1].path.elementAt(0).x, -16.666, 1e-3) && near(g.paths[1].path.elementAt(1).x, -16.666, 1e-3));
        QVERIFY(near(g.paths[1].path.elementAt(1).y, 100));
        g = presetGeometry(QStringLiteral("accentBorderCallout1"), {}, QSizeF(200, 100));
        QVERIFY(g.paths[0].stroke);
        // The second and third kinds bend the leader once and twice more.
        g = presetGeometry(QStringLiteral("borderCallout2"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths[1].path.elementCount(), 3);
        g = presetGeometry(QStringLiteral("borderCallout3"), {}, QSizeF(200, 100));
        QCOMPARE(g.paths[1].path.elementCount(), 4);
        QVERIFY(near(g.paths[1].path.currentPosition().x(), -16.666, 1e-3) && near(g.paths[1].path.currentPosition().y(), 112.963, 1e-3));
    }

    void dmlArrowCalloutsFillTheirFrame()
    {
        for (const char *name : {"rightArrowCallout", "leftArrowCallout", "upArrowCallout", "downArrowCallout", "leftRightArrowCallout", "upDownArrowCallout"}) {
            const Geometry g = presetGeometry(QString::fromLatin1(name), {}, QSizeF(200, 120));
            QCOMPARE(g.paths.size(), 1);
            const QRectF r = g.paths[0].path.boundingRect();
            QVERIFY2(near(r.left(), 0, 1e-3) && near(r.top(), 0, 1e-3) && near(r.right(), 200, 1e-3) && near(r.bottom(), 120, 1e-3), name);
            QVERIFY2(g.paths[0].path.contains(QPointF(100, 60)) || g.paths[0].path.contains(QPointF(60, 60)), name);
        }
        // The right arrow's head is at the right, the box at the left.
        const Geometry right = presetGeometry(QStringLiteral("rightArrowCallout"), {}, QSizeF(200, 120));
        QVERIFY(right.paths[0].path.contains(QPointF(10, 5)));
        QVERIFY(!right.paths[0].path.contains(QPointF(190, 5)));
        QVERIFY(right.paths[0].path.contains(QPointF(180, 60)));
        // Text goes in the box, not on the arrow.
        QVERIFY2(dmlRectIs(right.textRect, 0, 0, 200 * 0.64977, 120, 1e-4), qPrintable(dmlRectText(right.textRect)));
        const Geometry left = presetGeometry(QStringLiteral("leftArrowCallout"), {}, QSizeF(200, 120));
        QVERIFY2(dmlRectIs(left.textRect, 200 * (1 - 0.64977), 0, 200 * 0.64977, 120, 1e-4), qPrintable(dmlRectText(left.textRect)));
        const Geometry down = presetGeometry(QStringLiteral("downArrowCallout"), {}, QSizeF(120, 200));
        QVERIFY(down.paths[0].path.contains(QPointF(5, 10)));
        QVERIFY(!down.paths[0].path.contains(QPointF(5, 190)));
        QVERIFY(down.paths[0].path.contains(QPointF(60, 190)));
    }

    void dmlShapesTheLibraryLacksHaveDefinitions()
    {
        // A quarter pie sits in the lower right corner; the trapezoid is narrower at the top on both sides.
        Geometry g = presetGeometry(QStringLiteral("pieWedge"), {}, QSizeF(100, 100));
        QCOMPARE(g.paths.size(), 1);
        QVERIFY(g.paths[0].path.contains(QPointF(90, 90)));
        QVERIFY(g.paths[0].path.contains(QPointF(5, 95)));
        QVERIFY(!g.paths[0].path.contains(QPointF(10, 10)));
        g = presetGeometry(QStringLiteral("nonIsoscelesTrapezoid"), {}, QSizeF(200, 100));
        QVERIFY(g.paths[0].path.contains(QPointF(100, 5)));
        QVERIFY(!g.paths[0].path.contains(QPointF(5, 5)));
        QVERIFY(g.paths[0].path.contains(QPointF(5, 95)));
    }

    void dmlConnectorPresetsAreOpenPaths()
    {
        // The elbow with one bend goes along the top and down the right; with two, across at the adjusted position.
        Geometry g = presetGeometry(QStringLiteral("bentConnector2"), {}, QSizeF(100, 60));
        QCOMPARE(g.paths.size(), 1);
        QVERIFY(g.paths[0].fill == GeometryPath::None);
        QCOMPARE(g.paths[0].path.elementCount(), 3);
        QVERIFY(near(g.paths[0].path.elementAt(1).x, 100) && near(g.paths[0].path.elementAt(1).y, 0));
        g = presetGeometry(QStringLiteral("bentConnector3"), {{"adj1", 25000}}, QSizeF(100, 60));
        QCOMPARE(g.paths[0].path.elementCount(), 4);
        QVERIFY(near(g.paths[0].path.elementAt(1).x, 25) && near(g.paths[0].path.elementAt(2).x, 25) && near(g.paths[0].path.elementAt(2).y, 60));
        g = presetGeometry(QStringLiteral("bentConnector4"), {}, QSizeF(100, 60));
        QCOMPARE(g.paths[0].path.elementCount(), 5);
        QVERIFY(near(g.paths[0].path.currentPosition().x(), 100) && near(g.paths[0].path.currentPosition().y(), 60));
        g = presetGeometry(QStringLiteral("bentConnector5"), {{"adj1", 25000}, {"adj3", 75000}}, QSizeF(100, 60));
        QCOMPARE(g.paths[0].path.elementCount(), 6);
        QVERIFY(near(g.paths[0].path.elementAt(1).x, 25) && near(g.paths[0].path.elementAt(4).x, 75));
        for (const char *name : {"curvedConnector2", "curvedConnector3", "curvedConnector4", "curvedConnector5"}) {
            g = presetGeometry(QString::fromLatin1(name), {}, QSizeF(100, 60));
            QCOMPARE(g.paths.size(), 1);
            QVERIFY(near(g.paths[0].path.elementAt(0).x, 0) && near(g.paths[0].path.elementAt(0).y, 0));
            QVERIFY2(near(g.paths[0].path.currentPosition().x(), 100) && near(g.paths[0].path.currentPosition().y(), 60), name);
        }
    }
};

JP_TEST_CLASS(OfficeDmlGeometryTests)
#include "tst_office_dmlgeometry.moc"
