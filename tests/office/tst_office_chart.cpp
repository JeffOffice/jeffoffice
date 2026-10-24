// Tests of libs/office: charts: reading them and drawing them as SVG.
#include "officehelpers.h"
#include "../timing.h"

class OfficeChartTests : public QObject {
    Q_OBJECT


private Q_SLOTS:

    // ---- charts

    void chartReadsClusteredColumns()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("Q1"), {"East", "West", "North"}, {10, 20, 40}, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>"))
                             + chartSeries(1, QStringLiteral("Q2"), {"East", "West", "North"}, {5, 15, 25})
                             + QStringLiteral("<c:gapWidth val=\"80\"/><c:overlap val=\"-10\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        const Theme theme = defaultTheme();
        QVERIFY2(readChart(chartPart(plot), theme, ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Bar));
        QCOMPARE(int(c.type()), int(ChartType::Bar));
        QVERIFY(!p.horizontal);
        QCOMPARE(int(p.grouping), int(ChartGrouping::Clustered));
        QCOMPARE(p.gapWidth, 80);
        QCOMPARE(p.overlap, -10);
        QVERIFY(!p.varyColors);
        QCOMPARE(int(p.series.size()), 2);
        QCOMPARE(p.series[0].name, QStringLiteral("Q1"));
        QCOMPARE(p.series[0].categories, QStringList({"East", "West", "North"}));
        QCOMPARE(p.series[0].values, (QVector<double>{10, 20, 40}));
        QCOMPARE(p.series[1].name, QStringLiteral("Q2"));
        QCOMPARE(p.series[1].values, (QVector<double>{5, 15, 25}));
        // The axes: where the file says they are, and which the bars are drawn against.
        QCOMPARE(int(c.axes.size()), 2);
        QCOMPARE(int(p.axisIds[0]), 111);
        QCOMPARE(int(p.axisIds[1]), 222);
        QVERIFY(c.axis(111) && c.axis(222));
        QCOMPARE(int(c.axis(111)->kind), int(ChartAxis::Category));
        QCOMPARE(int(c.axis(222)->kind), int(ChartAxis::Value));
        QVERIFY(c.axis(222)->majorGridlines);
        QVERIFY(!c.axis(111)->majorGridlines);
        QVERIFY(c.axis(222)->autoMin && c.axis(222)->autoMax);
    }

    void chartReadsStackedBars()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"stacked\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("A"), {"x", "y"}, {1, 2}) + chartSeries(1, QStringLiteral("B"), {"x", "y"}, {3, 4})
                             + QStringLiteral("<c:gapWidth val=\"150\"/><c:overlap val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        QVERIFY(c.plots[0].horizontal);
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::Stacked));
        QCOMPARE(c.plots[0].overlap, 100);
        // Percent stacked, and the three-dimensional kinds, which are drawn flat.
        QString percent = plot;
        percent.replace(QStringLiteral("\"stacked\""), QStringLiteral("\"percentStacked\""));
        QVERIFY2(readChart(chartPart(percent), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::PercentStacked));
        QString threeD = plot;
        threeD.replace(QStringLiteral("barChart>"), QStringLiteral("bar3DChart>"));
        QVERIFY2(readChart(chartPart(threeD), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
    }

    void chartReadsLinesWithMarkers()
    {
        const QString marked = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00AA00\"/></a:solidFill></c:spPr></c:marker>");
        const QString bare = QStringLiteral("<c:marker><c:symbol val=\"none\"/></c:marker>");
        const QString plot = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/><c:varyColors val=\"0\"/>")
                             + chartSeries(0, QStringLiteral("One"), {"a", "b", "c"}, {1, 3, 2}, QStringLiteral("<c:spPr><a:ln w=\"38100\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill><a:prstDash val=\"dash\"/></a:ln></c:spPr>"),
                                           marked, QStringLiteral("<c:smooth val=\"1\"/>"))
                             + chartSeries(1, QStringLiteral("Two"), {"a", "b", "c"}, {2, 2, 4}, QString(), bare, QStringLiteral("<c:smooth val=\"0\"/>"))
                             + QStringLiteral("<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Line));
        QCOMPARE(int(p.grouping), int(ChartGrouping::Standard));
        QVERIFY(p.markers);
        QCOMPARE(int(p.series[0].marker.symbol), int(ChartMarker::Circle));
        QCOMPARE(p.series[0].marker.size, 9.0);
        QCOMPARE(p.series[0].marker.fill, QColor(0x00, 0xAA, 0x00));
        QVERIFY(p.series[0].smooth);
        QCOMPARE(p.series[0].color, QColor(0, 0, 0xFF));   // a line's color is its stroke's
        QCOMPARE(p.series[0].outline.width, 3.0);
        QCOMPARE(p.series[0].outline.dash, QStringLiteral("dash"));
        QCOMPARE(int(p.series[1].marker.symbol), int(ChartMarker::None));
        QVERIFY(!p.series[1].smooth);
        QCOMPARE(p.series[1].color, defaultTheme().colors[Theme::Accent2]);
    }

    void chartReadsPiesWithDataLabels()
    {
        const QString labels = QStringLiteral("<c:dLbls><c:numFmt formatCode=\"0%\" sourceLinked=\"0\"/><c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1200\" b=\"1\"/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>"
                                              "<c:dLblPos val=\"outEnd\"/><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"1\"/><c:showBubbleSize val=\"0\"/>"
                                              "<c:showLeaderLines val=\"1\"/></c:dLbls>");
        const QString point = QStringLiteral("<c:dPt><c:idx val=\"2\"/><c:bubble3D val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:dPt>");
        const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>")
                             + chartSeries(0, QStringLiteral("Share"), {"a", "b", "c", "d"}, {1, 2, 3, 4}, QString(), point + labels)
                             + QStringLiteral("<c:firstSliceAng val=\"90\"/></c:pieChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Pie));
        QVERIFY(p.varyColors);
        QCOMPARE(p.firstSliceAngle, 90);
        const ChartSeries &s = p.series[0];
        QVERIFY(s.labels.percent);
        QVERIFY(!s.labels.value && !s.labels.category && !s.labels.series);
        QCOMPARE(s.labels.format, QStringLiteral("0%"));
        QCOMPARE(s.labels.position, QStringLiteral("outEnd"));
        QCOMPARE(s.labels.font.size, 12.0);
        QCOMPARE(s.labels.font.bold, 1);
        QCOMPARE(int(s.pointColors.size()), 1);
        QCOMPARE(s.pointColors.value(2), QColor(0, 0xFF, 0));
        QVERIFY(c.axes.isEmpty());   // a pie has no axes
        // A doughnut has its hole.
        QString ring = plot;
        ring.replace(QStringLiteral("pieChart"), QStringLiteral("doughnutChart")).replace(QStringLiteral("<c:firstSliceAng val=\"90\"/>"), QStringLiteral("<c:firstSliceAng val=\"0\"/><c:holeSize val=\"60\"/>"));
        QVERIFY2(readChart(chartPart(ring), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Doughnut));
        QCOMPARE(c.plots[0].holeSize, 60);
    }

    void chartReadsScatterPlots()
    {
        const QString xs = QStringLiteral("<c:xVal><c:numRef><c:f>Sheet1!$A$2:$A$5</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"4\"/>"
                                          "<c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2.5</c:v></c:pt><c:pt idx=\"2\"><c:v>3</c:v></c:pt><c:pt idx=\"3\"><c:v>7</c:v></c:pt></c:numCache></c:numRef></c:xVal>");
        const QString ys = QStringLiteral("<c:yVal><c:numLit><c:formatCode>General</c:formatCode><c:ptCount val=\"4\"/>"
                                          "<c:pt idx=\"0\"><c:v>2</c:v></c:pt><c:pt idx=\"1\"><c:v>4</c:v></c:pt><c:pt idx=\"3\"><c:v>9</c:v></c:pt></c:numLit></c:yVal>");
        const QString series = QStringLiteral("<c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>Trial</c:v></c:tx><c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"
                                              "<c:marker><c:symbol val=\"diamond\"/><c:size val=\"7\"/></c:marker>") + xs + ys + QStringLiteral("<c:smooth val=\"0\"/></c:ser>");
        const QString axes = QStringLiteral("<c:valAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"10\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>"
                                            "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>");
        const QString plot = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:varyColors val=\"0\"/>") + series
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>") + axes;
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const ChartPlot &p = c.plots[0];
        QCOMPARE(int(p.type), int(ChartType::Scatter));
        QCOMPARE(p.series[0].name, QStringLiteral("Trial"));
        QCOMPARE(p.series[0].xValues, (QVector<double>{1, 2.5, 3, 7}));
        QCOMPARE(int(p.series[0].values.size()), 4);
        QCOMPARE(p.series[0].values[0], 2.0);
        QCOMPARE(p.series[0].values[1], 4.0);
        QVERIFY(std::isnan(p.series[0].values[2]));   // a point the file has no value for
        QCOMPARE(p.series[0].values[3], 9.0);
        QVERIFY(p.series[0].outline.none);            // markers alone
        QCOMPARE(int(p.series[0].marker.symbol), int(ChartMarker::Diamond));
        QCOMPARE(int(c.axes.size()), 2);
        QCOMPARE(int(c.axis(111)->kind), int(ChartAxis::Value));
        QVERIFY(!c.axis(111)->autoMin && !c.axis(111)->autoMax);
        QCOMPARE(c.axis(111)->min, 0.0);
        QCOMPARE(c.axis(111)->max, 10.0);
        QVERIFY(c.axis(222)->autoMax);
    }

    void chartTakesColorsFromTheFileAndTheTheme()
    {
        const QString accent = QStringLiteral("<c:spPr><a:solidFill><a:schemeClr val=\"accent4\"/></a:solidFill></c:spPr>");
        const QString darker = QStringLiteral("<c:spPr><a:solidFill><a:schemeClr val=\"accent3\"><a:lumMod val=\"50000\"/></a:schemeClr></a:solidFill></c:spPr>");
        QString series = chartSeries(0, QStringLiteral("S0"), {"a"}, {1}, accent) + chartSeries(1, QStringLiteral("S1"), {"a"}, {1}, darker);
        for (int i = 2; i < 14; ++i) series += chartSeries(i, QStringLiteral("S%1").arg(i), {"a"}, {1});
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + series + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        const Theme theme = defaultTheme();
        QVERIFY2(readChart(chartPart(plot), theme, ColorMap(), &c, &err), qPrintable(err));
        const QVector<ChartSeries> &s = c.plots[0].series;
        QCOMPARE(int(s.size()), 14);
        QCOMPARE(s[0].color, theme.colors[Theme::Accent4]);
        QVERIFY(s[0].colorFromFile);
        // A transform of a theme color is applied.
        QVERIFY(s[1].colorFromFile);
        QVERIFY(s[1].color.lightnessF() < theme.colors[Theme::Accent3].lightnessF() * 0.7);
        QVERIFY(qAbs(s[1].color.hueF() - theme.colors[Theme::Accent3].hueF()) < 0.02);
        // The rest take the accents in order (the file's own colors don't use them up).
        for (int i = 2; i < 6; ++i) {
            QVERIFY(!s[i].colorFromFile);
            QCOMPARE(s[i].color, theme.colors[Theme::Accent1 + i]);
        }
        // More series than accents: the accents again, each darker, then lighter.
        QVERIFY(qAbs(s[6].color.hueF() - theme.colors[Theme::Accent1].hueF()) < 0.02);
        QVERIFY(s[6].color != theme.colors[Theme::Accent1]);
        QVERIFY(s[6].color.lightnessF() < theme.colors[Theme::Accent1].lightnessF());
        QVERIFY(s[12].color.lightnessF() > s[6].color.lightnessF());
        QVERIFY(s[12].color != theme.colors[Theme::Accent1]);
        // The color map renames what the file asks for: a dark slide's "tx1" is its light one.
        ColorMap dark;
        dark.tx1 = Theme::Light1;
        dark.bg1 = Theme::Dark1;
        const QString text = QStringLiteral("<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill></a:defRPr></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>");
        QVERIFY2(readChart(chartPart(plot, QString(), QString(), text), theme, dark, &c, &err), qPrintable(err));
        QCOMPARE(c.font.color, theme.colors[Theme::Light1]);
    }

    void chartStyleColorsTheSeriesTheFileLeavesBare()
    {
        QString series;
        for (int i = 0; i < 4; ++i) series += chartSeries(i, QStringLiteral("S%1").arg(i), {"a", "b"}, {1.0 + i, 2.0});
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + series + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const Theme theme = defaultTheme();
        auto styled = [&](int style) {
            QByteArray xml = chartPart(plot);
            xml.replace("<c:roundedCorners val=\"0\"/>", QByteArray("<c:roundedCorners val=\"0\"/><c:style val=\"") + QByteArray::number(style) + "\"/>");
            Chart c;
            QString err;
            [&] { QVERIFY2(readChart(xml, theme, ColorMap(), &c, &err), qPrintable(err)); }();
            return c;
        };
        // Style 2, and what has none, takes the accents; so do the styles that count up from it eight at a time.
        for (int style : {0, 2, 10, 26, 42}) {
            const Chart c = styled(style);
            QCOMPARE(c.style, style == 0 ? 2 : style);
            for (int i = 0; i < 4; ++i) QCOMPARE(c.plots[0].series[i].color, theme.colors[Theme::Accent1 + i]);
        }
        // Style 1 is shades of gray, darkest first.
        const Chart gray = styled(1);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(gray.plots[0].series[i].color.saturationF() < 0.05, qPrintable(gray.plots[0].series[i].color.name()));
            if (i > 0) QVERIFY(gray.plots[0].series[i].color.lightnessF() > gray.plots[0].series[i - 1].color.lightnessF());
        }
        // Style 3 is shades of the first accent, style 8 of the sixth (and 19 again of the first).
        for (const QPair<int, int> &p : {qMakePair(3, 0), qMakePair(8, 5), qMakePair(19, 0)}) {
            const Chart c = styled(p.first);
            const QColor accent = theme.colors[Theme::Accent1 + p.second];
            for (int i = 0; i < 4; ++i) {
                const QColor shade = c.plots[0].series[i].color;
                QVERIFY2(qAbs(shade.hueF() - accent.hueF()) < 0.03 || accent.saturationF() < 0.1, qPrintable(QString("%1: %2").arg(p.first).arg(shade.name())));
                if (i > 0) QVERIFY(shade.lightnessF() > c.plots[0].series[i - 1].color.lightnessF());
            }
            QVERIFY(c.plots[0].series[0].color.lightnessF() < accent.lightnessF());
            QVERIFY(c.plots[0].series[3].color.lightnessF() > accent.lightnessF());
        }
        // A color the file gives stays.
        const QString own = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                            + chartSeries(0, QStringLiteral("A"), {"a"}, {1}, QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>")) + chartSeries(1, QStringLiteral("B"), {"a"}, {2})
                            + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        QByteArray xml = chartPart(own);
        xml.replace("<c:roundedCorners val=\"0\"/>", "<c:roundedCorners val=\"0\"/><c:style val=\"3\"/>");
        Chart c;
        QString err;
        QVERIFY2(readChart(xml, theme, ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(c.plots[0].series[0].color, QColor(0xFF, 0, 0));
        QVERIFY(!c.plots[0].series[1].colorFromFile);
        QVERIFY(qAbs(c.plots[0].series[1].color.hueF() - theme.colors[Theme::Accent1].hueF()) < 0.03);
    }

    void chartReadsTitleLegendAndAxes()
    {
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1400\" b=\"1\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1800\" b=\"0\"/><a:t>Sales by </a:t></a:r>"
                                             "<a:r><a:rPr lang=\"en-US\" sz=\"1800\" b=\"0\"/><a:t>region</a:t></a:r></a:p><a:p><a:r><a:rPr lang=\"en-US\" sz=\"1000\"/><a:t>in thousands</a:t></a:r></a:p></c:rich></c:tx>"
                                             "<c:overlay val=\"0\"/></c:title><c:autoTitleDeleted val=\"0\"/>");
        const QString axes = QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/>"
                                            "<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:t>Region</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"
                                            "<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"none\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"low\"/>"
                                            "<c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/><c:tickLblSkip val=\"2\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                                            "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"50\"/><c:min val=\"-10\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/>"
                                            "<c:majorGridlines><c:spPr><a:ln w=\"9525\"><a:solidFill><a:srgbClr val=\"D9D9D9\"/></a:solidFill></a:ln></c:spPr></c:majorGridlines>"
                                            "<c:title><c:tx><c:rich><a:bodyPr rot=\"-5400000\" vert=\"horz\"/><a:p><a:r><a:t>Units</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>"
                                            "<c:numFmt formatCode=\"#,##0&quot; units&quot;\" sourceLinked=\"0\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/>"
                                            "<c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"900\"/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p></c:txPr>"
                                            "<c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/><c:majorUnit val=\"10\"/></c:valAx>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Q1"), {"East", "West"}, {10, 20})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + axes;
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, title, QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"0\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.title.present);
        QCOMPARE(c.title.text, QStringLiteral("Sales by region\nin thousands"));
        QCOMPARE(c.title.font.size, 18.0);   // the first run's, over the paragraph's default
        QCOMPARE(c.title.font.bold, 0);
        QVERIFY(!c.title.overlay);
        QVERIFY(c.legend.shown);
        QCOMPARE(c.legend.position, QStringLiteral("r"));
        const ChartAxis *cat = c.axis(111), *val = c.axis(222);
        QVERIFY(cat && val);
        QCOMPARE(cat->title.text, QStringLiteral("Region"));
        QCOMPARE(cat->labelPosition, QStringLiteral("low"));
        QCOMPARE(cat->majorTick, QStringLiteral("none"));
        QCOMPARE(cat->labelSkip, 2);
        QCOMPARE(cat->position, QStringLiteral("b"));
        QCOMPARE(val->title.text, QStringLiteral("Units"));
        QCOMPARE(val->position, QStringLiteral("l"));
        QVERIFY(!val->autoMin && !val->autoMax);
        QCOMPARE(val->min, -10.0);
        QCOMPARE(val->max, 50.0);
        QCOMPARE(val->majorUnit, 10.0);
        QVERIFY(val->majorGridlines);
        QCOMPARE(val->gridline.color, QColor(0xD9, 0xD9, 0xD9));
        QCOMPARE(val->gridline.width, 0.75);
        QCOMPARE(val->format, QStringLiteral("#,##0\" units\""));
        QVERIFY(!val->formatLinked);
        QCOMPARE(val->font.size, 9.0);
        QVERIFY(val->between);
        // No title element: none, and no legend: none.
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(!c.title.present);
        QVERIFY(!c.legend.shown);
    }

    void chartReadsAreasAndLiterals()
    {
        // A category the file stores as numbers, as text.
        const QString cats = QStringLiteral("<c:cat><c:numRef><c:f>Sheet1!$A$2:$A$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"3\"/>"
                                            "<c:pt idx=\"0\"><c:v>1774</c:v></c:pt><c:pt idx=\"2\"><c:v>1790</c:v></c:pt></c:numCache></c:numRef></c:cat>");
        const QString vals = QStringLiteral("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$4</c:f><c:numCache><c:formatCode>0.0</c:formatCode><c:ptCount val=\"3\"/>"
                                            "<c:pt idx=\"1\"><c:v>0.217</c:v></c:pt><c:pt idx=\"2\"><c:v>7.8718E-2</c:v></c:pt></c:numCache></c:numRef></c:val>");
        const QString plot = QStringLiteral("<c:areaChart><c:grouping val=\"stacked\"/><c:varyColors val=\"0\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>") + cats + vals
                             + QStringLiteral("</c:ser><c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots[0].type), int(ChartType::Area));
        QCOMPARE(int(c.plots[0].grouping), int(ChartGrouping::Stacked));
        const ChartSeries &s = c.plots[0].series[0];
        QCOMPARE(s.categories, QStringList({"1774", "", "1790"}));
        QCOMPARE(int(s.values.size()), 3);
        QVERIFY(std::isnan(s.values[0]));
        QCOMPARE(s.values[1], 0.217);
        QCOMPARE(s.values[2], 0.078718);
        QCOMPARE(s.format, QStringLiteral("0.0"));
        QCOMPARE(s.name, QStringLiteral("Series1"));   // the file names none
        QCOMPARE(c.blanksAs, QStringLiteral("gap"));
    }

    void chartKeepsWhatItCannotDrawAsColumns()
    {
        // Kinds not drawn yet read as columns of their values, with the file's name for them.
        const QString plot = QStringLiteral("<c:radarChart><c:radarStyle val=\"marker\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b", "c"}, {3, 1, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:radarChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 1);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Other));
        QCOMPARE(c.plots[0].element, QStringLiteral("radarChart"));
        QCOMPARE(c.plots[0].series[0].values, (QVector<double>{3, 1, 2}));
        // Two kinds at once: columns with a line over them.
        const QString both = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Bars"), {"a", "b"}, {3, 1})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart><c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(1, QStringLiteral("Line"), {"a", "b"}, {2, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        QVERIFY2(readChart(chartPart(both), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QCOMPARE(int(c.plots.size()), 2);
        QCOMPARE(int(c.plots[0].type), int(ChartType::Bar));
        QCOMPARE(int(c.plots[1].type), int(ChartType::Line));
        // The colors continue from one plot to the next.
        QCOMPARE(c.plots[0].series[0].color, defaultTheme().colors[Theme::Accent1]);
        QCOMPARE(c.plots[1].series[0].color, defaultTheme().colors[Theme::Accent2]);
    }

    void chartRefusesWhatIsNotAChart()
    {
        Chart c;
        QString err;
        QVERIFY(!readChart(QByteArray("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>"), defaultTheme(), ColorMap(), &c, &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!readChart(QByteArray("this is not XML"), defaultTheme(), ColorMap(), &c, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!readChart(QByteArray(), defaultTheme(), ColorMap(), &c, nullptr));
        // A chart with no series is one: it draws as an empty chart.
        QVERIFY2(readChart(chartPart(QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes()),
                           defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series.isEmpty());
        QVERIFY(chartSvg(c, QSizeF(300, 200)).contains("<svg"));
    }

    void chartFramesAreFoundByTheirUri()
    {
        const QString frame = QStringLiteral("<p:graphicFrame xmlns:p=\"%1\" xmlns:a=\"%2\" xmlns:r=\"%3\"><p:nvGraphicFramePr><p:cNvPr id=\"4\" name=\"Chart 3\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                            "<p:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"100\" cy=\"100\"/></p:xfrm><a:graphic><a:graphicData uri=\"%4\"><c:chart xmlns:c=\"%4\" r:id=\"rId7\"/></a:graphicData></a:graphic></p:graphicFrame>");
        QDomDocument doc;
        const QString chartUri = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/chart");
        QVERIFY(parseXml(frame.arg(QString(ns::P), QString(ns::A), QString(ns::R), chartUri).toUtf8(), &doc));
        QVERIFY(isChartFrame(doc.documentElement()));
        QCOMPARE(chartFrameRelId(doc.documentElement()), QStringLiteral("rId7"));
        QVERIFY(parseXml(frame.arg(QString(ns::P), QString(ns::A), QString(ns::R), QStringLiteral("urn:test:other")).toUtf8(), &doc));
        QVERIFY(!isChartFrame(doc.documentElement()));
        QVERIFY(!isChartFrame(QDomElement()));
    }

    void chartFormatsNumbersAsTheirCodesSay()
    {
        struct Case { double value; const char *code, *text; };
        const Case cases[] = {
            {1234.5, "General", "1234.5"}, {0.1 + 0.2, "General", "0.3"}, {0, "General", "0"}, {16.112566, "", "16.112566"}, {-2, "General", "-2"},
            {1234.567, "0", "1235"}, {1234.567, "0.0", "1234.6"}, {1234.567, "#,##0.00", "1,234.57"}, {1234567, "#,##0", "1,234,567"}, {12, "000", "012"},
            {0.256, "0%", "26%"}, {0.256, "0.0%", "25.6%"}, {1, "0.0%", "100.0%"}, {-1.5, "0.0", "-1.5"},
            {5, "$#,##0.00;($#,##0.00)", "$5.00"}, {-5, "$#,##0.00;($#,##0.00)", "($5.00)"}, {0, "0;-0;\"none\"", "none"},
            {5, "0\" units\"", "5 units"}, {1500, "0.0,\"k\"", "1.5k"}, {2500000, "0.0,,\"M\"", "2.5M"}, {12345.678, "0.00E+00", "1.23E+04"}, {0.5, "#.##", ".5"}, {3, "0.0#", "3.0"},
            {45000, "m/d/yyyy", "3/15/2023"}, {45000, "yyyy-mm-dd", "2023-03-15"}, {45000, "mmm-yy", "Mar-23"}, {45000, "dddd", "Wednesday"}, {45000.75, "h:mm AM/PM", "6:00 PM"},
            {45000.75, "d mmmm yyyy, hh:mm", "15 March 2023, 18:00"}};
        for (const Case &c : cases) QCOMPARE(formatChartNumber(c.value, QString::fromLatin1(c.code)), QString::fromLatin1(c.text));
        QVERIFY(formatChartNumber(std::nan(""), QStringLiteral("0")).isEmpty());
    }

    void chartSvgIsInPointsWithTheTextAsText()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Revenue"), {"East", "West"}, {10, 20})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>Quarterly &amp; yearly</a:t></a:r></a:p></c:rich></c:tx><c:overlay val=\"0\"/></c:title>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, title, QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QByteArray svg = chartSvg(c, QSizeF(432, 252));
        QSvgRenderer r(svg);
        QVERIFY(r.isValid());
        QCOMPARE(r.viewBoxF(), QRectF(0, 0, 432, 252));
        // Text is text, in the chart's font.
        const QString text = QString::fromUtf8(svg);
        QVERIFY(text.contains(QStringLiteral("<text")));
        QVERIFY(text.contains(QStringLiteral(">Quarterly &amp; yearly<")));
        QVERIFY(text.contains(QStringLiteral(">East<")));
        QVERIFY(text.contains(QStringLiteral(">West<")));
        QVERIFY(text.contains(QStringLiteral(">Revenue<")));   // the legend
        QVERIFY(text.contains(QStringLiteral("font-family=\"%1\"").arg(defaultTheme().minorLatin)));
        // An empty size draws nothing and doesn't fail.
        QVERIFY(chartSvg(c, QSizeF(0, 0)).contains("<svg"));
    }

    void chartSvgDrawsColumnsInProportion()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                             + QStringLiteral("<c:gapWidth val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        // Ink in the plot area besides the bars: gridlines and labels.
        int inked = 0, redPixels = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                if (isColor(p, Qt::red, 40)) ++redPixels;
                else if (qRed(p) < 235 || qGreen(p) < 235 || qBlue(p) < 235) ++inked;
            }
        QVERIFY(inked > 400);
        QVERIFY(redPixels > 1000);
        const QVector<QRect> bars = colorRuns(img, Qt::red, false);
        QCOMPARE(int(bars.size()), 3);
        // Standing on one line, equally wide, in the order of their values and 1 : 2 : 4 high.
        QCOMPARE(bars[0].bottom(), bars[1].bottom());
        QCOMPARE(bars[1].bottom(), bars[2].bottom());
        QVERIFY(qAbs(bars[0].width() - bars[1].width()) <= 2 && qAbs(bars[1].width() - bars[2].width()) <= 2);
        QVERIFY(bars[0].left() < bars[1].left() && bars[1].left() < bars[2].left());
        QVERIFY2(qAbs(bars[1].height() / double(bars[0].height()) - 2) < 0.08, qPrintable(QString::number(bars[1].height() / double(bars[0].height()))));
        QVERIFY2(qAbs(bars[2].height() / double(bars[0].height()) - 4) < 0.15, qPrintable(QString::number(bars[2].height() / double(bars[0].height()))));
        // Gaps of 100% leave a bar's width between bars.
        QVERIFY2(qAbs((bars[1].left() - bars[0].right()) / double(bars[0].width()) - 1) < 0.1, qPrintable(QString::number((bars[1].left() - bars[0].right()) / double(bars[0].width()))));
        // The bars stand inside the picture, below the top and clear of the edges.
        QVERIFY(bars[0].left() > 20 && bars[2].right() < img.width() - 10);
        QVERIFY(bars[2].top() > 10 && bars[2].bottom() < img.height() - 20);
    }

    void chartSvgStacksColumns()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString blue = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"stacked\"/>") + chartSeries(0, QStringLiteral("R"), {"a", "b"}, {10, 20}, red)
                             + chartSeries(1, QStringLiteral("B"), {"a", "b"}, {30, 10}, blue) + QStringLiteral("<c:gapWidth val=\"50\"/><c:overlap val=\"100\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> reds = colorRuns(img, Qt::red, false), blues = colorRuns(img, Qt::blue, false);
        QCOMPARE(int(reds.size()), 2);
        QCOMPARE(int(blues.size()), 2);
        // Each blue segment sits on its red one; the heights are the values, 10 + 30 and 20 + 10.
        QVERIFY(qAbs(blues[0].bottom() - reds[0].top()) <= 2);
        QVERIFY(qAbs(blues[1].bottom() - reds[1].top()) <= 2);
        QVERIFY2(qAbs(blues[0].height() / double(reds[0].height()) - 3) < 0.15, qPrintable(QString::number(blues[0].height() / double(reds[0].height()))));
        QVERIFY2(qAbs(reds[1].height() / double(blues[1].height()) - 2) < 0.1, qPrintable(QString::number(reds[1].height() / double(blues[1].height()))));
        const double total0 = reds[0].height() + blues[0].height(), total1 = reds[1].height() + blues[1].height();
        QVERIFY2(qAbs(total0 / total1 - 4.0 / 3.0) < 0.06, qPrintable(QString::number(total0 / total1)));
        // Percent stacked: both stacks reach the top.
        QString percent = plot;
        percent.replace(QStringLiteral("\"stacked\""), QStringLiteral("\"percentStacked\""));
        QVERIFY2(readChart(chartPart(percent), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage full = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> a = colorRuns(full, Qt::red, false), b = colorRuns(full, Qt::blue, false);
        QCOMPARE(int(b.size()), 2);
        QVERIFY(qAbs(b[0].top() - b[1].top()) <= 2);
        QVERIFY2(qAbs(a[0].height() / double(b[0].height()) - 1.0 / 3.0) < 0.05, qPrintable(QString::number(a[0].height() / double(b[0].height()))));
    }

    void chartSvgDrawsBarsAcross()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                             + QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/>"
                                              "<c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>"
                                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/></c:valAx>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].horizontal);
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QRect> bars = colorRuns(img, Qt::red, true);   // from the top
        QCOMPARE(int(bars.size()), 3);
        // The first category is at the bottom: from the top the bars are 40, 20, and 10 wide, from one edge.
        QCOMPARE(bars[0].left(), bars[1].left());
        QCOMPARE(bars[1].left(), bars[2].left());
        QVERIFY2(qAbs(bars[0].width() / double(bars[1].width()) - 2) < 0.08, qPrintable(QString::number(bars[0].width() / double(bars[1].width()))));
        QVERIFY2(qAbs(bars[1].width() / double(bars[2].width()) - 2) < 0.12, qPrintable(QString::number(bars[1].width() / double(bars[2].width()))));
        QVERIFY(qAbs(bars[0].height() - bars[2].height()) <= 2);
    }

    void chartSvgPutsReversedCategoriesFirstAtTheTop()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        auto bars = [&](const QString &order, const QString &crosses) {
            return QStringLiteral("<c:barChart><c:barDir val=\"bar\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {10, 20, 40}, red)
                   + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                   + QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"%1\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/>"
                                    "<c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/><c:lblOffset val=\"100\"/></c:catAx>"
                                    "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                    "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"%2\"/><c:crossBetween val=\"between\"/></c:valAx>").arg(order, crosses);
        };
        // Where the value axis' labels are: dark text along the bottom of the picture, or along its top. (The bars are red, the lines gray.)
        auto darkIn = [](const QImage &img, int from, int to) {
            for (int y = from; y < to; ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qRed(img.pixel(x, y)) < 100 && qGreen(img.pixel(x, y)) < 100 && qBlue(img.pixel(x, y)) < 120) return true;
            return false;
        };
        Chart plain, turned, atTop;
        QString err;
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("minMax"), QStringLiteral("autoZero"))), defaultTheme(), ColorMap(), &plain, &err), qPrintable(err));
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("maxMin"), QStringLiteral("max"))), defaultTheme(), ColorMap(), &turned, &err), qPrintable(err));
        QVERIFY2(readChart(chartPart(bars(QStringLiteral("maxMin"), QStringLiteral("autoZero"))), defaultTheme(), ColorMap(), &atTop, &err), qPrintable(err));
        QVERIFY(plain.axis(111) && !plain.axis(111)->reversed);
        QVERIFY(turned.axis(111)->reversed);
        // The first category is at the bottom, or at the top once the categories are turned round.
        const QVector<QRect> up = colorRuns(renderChart(plain, QSizeF(400, 300), 2), Qt::red, true), down = colorRuns(renderChart(turned, QSizeF(400, 300), 2), Qt::red, true);
        QCOMPARE(int(up.size()), 3);
        QCOMPARE(int(down.size()), 3);
        QVERIFY(up[0].width() > up[2].width());
        QVERIFY(down[0].width() < down[2].width());
        // The value axis crosses at the first category, which turned round is the top, unless it is said to cross at the last.
        const QImage a = renderChart(plain, QSizeF(400, 300), 2), b = renderChart(turned, QSizeF(400, 300), 2), t = renderChart(atTop, QSizeF(400, 300), 2);
        QVERIFY(darkIn(a, 540, 600) && !darkIn(a, 0, 60));
        QVERIFY(darkIn(b, 540, 600) && !darkIn(b, 0, 60));
        QVERIFY(darkIn(t, 0, 60) && !darkIn(t, 540, 600));
    }

    void chartSvgDrawsPieSlices()
    {
        const QString colored = QStringLiteral("<c:dPt><c:idx val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr></c:dPt>"
                                               "<c:dPt><c:idx val=\"1\"/><c:spPr><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></c:spPr></c:dPt>"
                                               "<c:dPt><c:idx val=\"2\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:dPt>");
        const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, 1, 2}, QString(), colored)
                             + QStringLiteral("<c:firstSliceAng val=\"0\"/></c:pieChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(300, 300), 2);
        const QColor green(0, 0xFF, 0);
        const double r = colorArea(img, Qt::red), b = colorArea(img, Qt::blue), g = colorArea(img, green);
        QVERIFY(r > 5000);
        QVERIFY2(qAbs(b / r - 1) < 0.08, qPrintable(QString::number(b / r)));
        QVERIFY2(qAbs(g / r - 2) < 0.12, qPrintable(QString::number(g / r)));
        // The first slice starts at twelve o'clock and runs clockwise: red upper right, blue lower right, green the left half.
        const QPointF cr = colorCentroid(img, Qt::red), cb = colorCentroid(img, Qt::blue), cg = colorCentroid(img, green);
        QVERIFY(cr.x() > cg.x() && cb.x() > cg.x());
        QVERIFY(cr.y() < cb.y());
        QVERIFY(qAbs(cg.y() - (cr.y() + cb.y()) / 2) < 12);
        // A pie is round: its sides are as wide as it is high.
        const QRect box = colorBounds(img, QColor(), true);
        QVERIFY2(qAbs(box.width() - box.height()) <= 3, qPrintable(QString("%1 x %2").arg(box.width()).arg(box.height())));
        // Percentages as the slices' labels, as text.
        QString labeled = plot;
        labeled.replace(QStringLiteral("<c:firstSliceAng"),
                        QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"0\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"1\"/><c:showBubbleSize val=\"0\"/></c:dLbls><c:firstSliceAng"));
        QVERIFY2(readChart(chartPart(labeled), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series[0].labels.percent);   // the plot's labels are the series'
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(300, 300)));
        QVERIFY(svg.contains(QStringLiteral(">50%<")));
        QCOMPARE(int(svg.count(QStringLiteral(">25%<"))), 2);
    }

    void chartSvgDrawsLinesAndMarkers()
    {
        const QString line = QStringLiteral("<c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>");
        const QString plot = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c", "d"}, {1, 2, 3, 4}, line,
                                                                                                      QStringLiteral("<c:marker><c:symbol val=\"square\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></c:spPr></c:marker>"))
                             + QStringLiteral("<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        QVERIFY(colorArea(img, Qt::red) > 500);
        // Four square markers, in the file's color, along the rising line.
        const QVector<QPointF> markers = colorComponents(img, QColor(0, 0xFF, 0), 20);
        QCOMPARE(int(markers.size()), 4);
        for (int i = 1; i < markers.size(); ++i) {
            QVERIFY(markers[i].x() > markers[i - 1].x());
            QVERIFY(markers[i].y() < markers[i - 1].y());
        }
        // Equal steps in the values are equal steps up the page.
        QVERIFY2(qAbs((markers[1].y() - markers[0].y()) - (markers[3].y() - markers[2].y())) < 2, "unequal steps");
        // Points sit in the middle of their categories.
        QVERIFY(qAbs((markers[1].x() - markers[0].x()) - (markers[3].x() - markers[2].x())) < 2);
    }

    void chartSvgDrawsScatterPoints()
    {
        const QString plot = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/>")
                             + QStringLiteral("<c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:tx><c:v>P</c:v></c:tx><c:spPr><a:ln w=\"19050\"><a:noFill/></a:ln></c:spPr>"
                                              "<c:marker><c:symbol val=\"circle\"/><c:size val=\"9\"/><c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln><a:noFill/></a:ln></c:spPr></c:marker>"
                                              "<c:xVal><c:numLit><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2</c:v></c:pt><c:pt idx=\"2\"><c:v>4</c:v></c:pt></c:numLit></c:xVal>"
                                              "<c:yVal><c:numLit><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>4</c:v></c:pt><c:pt idx=\"2\"><c:v>9</c:v></c:pt></c:numLit></c:yVal></c:ser>")
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>")
                             + QStringLiteral("<c:valAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"5\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>"
                                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"10\"/><c:min val=\"0\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"midCat\"/></c:valAx>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        const QVector<QPointF> dots = colorComponents(img, Qt::red, 20);
        QCOMPARE(int(dots.size()), 3);   // markers alone, not joined
        // Placed by their values on both axes: x at 1, 2, 4 of 0 to 5, and y at 1, 4, 9 of 0 to 10.
        const double dx1 = dots[1].x() - dots[0].x(), dx2 = dots[2].x() - dots[1].x();
        const double dy1 = dots[0].y() - dots[1].y(), dy2 = dots[1].y() - dots[2].y();
        QVERIFY2(qAbs(dx2 / dx1 - 2) < 0.05, qPrintable(QString::number(dx2 / dx1)));
        QVERIFY2(qAbs(dy2 / dy1 - 5.0 / 3.0) < 0.05, qPrintable(QString::number(dy2 / dy1)));
        // The file's 0 to 10 is labeled in steps of 1, and goes no further.
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        QVERIFY(svg.contains(QStringLiteral(">10<")));
        QVERIFY(!svg.contains(QStringLiteral(">12<")));
    }

    void chartSvgScalesTheAxisToTheValues()
    {
        // The largest value, 16.1, gets an axis to 18 in steps of 2, from 0.
        const QString plot = QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {0.2, 16.112566, 5.72})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString svg = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        for (const char *label : {"0", "2", "4", "8", "16", "18"}) QVERIFY2(svg.contains(QStringLiteral(">%1<").arg(QLatin1String(label))), label);
        QVERIFY(!svg.contains(QStringLiteral(">20<")));
        QVERIFY(!svg.contains(QStringLiteral(">1<")));
        // Labels in the axis' number format.
        QString thousands = plot;
        thousands.replace(QStringLiteral("<c:numFmt formatCode=\"General\" sourceLinked=\"1\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/>"),
                        QStringLiteral("<c:numFmt formatCode=\"0.0&quot;k&quot;\" sourceLinked=\"0\"/><c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/>"));
        QVERIFY2(readChart(chartPart(thousands), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        const QString formatted = QString::fromUtf8(chartSvg(c, QSizeF(400, 300)));
        QVERIFY2(formatted.contains(QStringLiteral(">18.0k<")), qPrintable(formatted.left(200)));
    }

    void chartSvgPlacesTheLegend()
    {
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></c:spPr>");
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), {"a", "b"}, {1, 0}, red)
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        for (const QString &pos : {QStringLiteral("b"), QStringLiteral("r"), QStringLiteral("t"), QStringLiteral("l")}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"%1\"/></c:legend>").arg(pos)), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            QCOMPARE(c.legend.position, pos);
            const QImage img = renderChart(c, QSizeF(400, 300), 2);
            // The key is a small red square beside the series' name, at that side; the bar of value 0 is not drawn.
            const QVector<QPointF> bits = colorComponents(img, Qt::red, 4);
            QVERIFY2(!bits.isEmpty(), qPrintable(pos));
            QPointF key = bits.last();
            for (const QPointF &p : bits) {
                if (pos == QLatin1String("b") && p.y() > key.y()) key = p;
                if (pos == QLatin1String("t") && p.y() < key.y()) key = p;
                if (pos == QLatin1String("r") && p.x() > key.x()) key = p;
                if (pos == QLatin1String("l") && p.x() < key.x()) key = p;
            }
            if (pos == QLatin1String("b")) QVERIFY2(key.y() > img.height() * 0.85, qPrintable(QString::number(key.y())));
            if (pos == QLatin1String("t")) QVERIFY2(key.y() < img.height() * 0.15, qPrintable(QString::number(key.y())));
            // At a side, the key is one name's width in from the edge, and names are wider with some systems' fonts
            // (Windows: 68% across); a legend at the top or bottom has it left of the middle.
            if (pos == QLatin1String("r")) QVERIFY2(key.x() > img.width() * 0.6, qPrintable(QString::number(key.x())));
            if (pos == QLatin1String("l")) QVERIFY2(key.x() < img.width() * 0.4, qPrintable(QString::number(key.x())));
            QVERIFY(QString::fromUtf8(chartSvg(c, QSizeF(400, 300))).contains(QStringLiteral(">Only series<")));
        }
    }

    // ---- the places a file gives to the legend and the titles

    // A bar chart of one series of the color, `values` over their names, whose columns touch (so that a column covers what is behind it), with an axis title if asked.
    static QString oneSeriesPlot(const QString &color, const QVector<double> &values, const QString &axisTitle = QString(), const QString &axisTitleXml = QString(), const QString &catTitleXml = QString())
    {
        QStringList names;
        for (int i = 0; i < values.size(); ++i) names << QString(QChar('a' + i));
        const QString fill = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill></c:spPr>").arg(color);
        QString axes = chartAxes();
        // (the value axis stops at the largest value, so that the tallest column reaches the top of the plot)
        axes.replace(QStringLiteral("<c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling>"), QStringLiteral("<c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"1\"/><c:min val=\"0\"/></c:scaling>"));
        if (!axisTitle.isEmpty()) axes.replace(QStringLiteral("<c:majorGridlines/>"), QStringLiteral("<c:majorGridlines/>") + axisTitleXml.arg(axisTitle));
        if (!catTitleXml.isEmpty()) axes.replace(QStringLiteral("<c:axPos val=\"b\"/>"), QStringLiteral("<c:axPos val=\"b\"/>") + catTitleXml);
        return QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), names, values, fill) + QStringLiteral("<c:gapWidth val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + axes;
    }
    static QString manualLayout(double x, double y, double w, double h, const QString &target = QString())
    {
        return QStringLiteral("<c:layout><c:manualLayout>%1<c:xMode val=\"edge\"/><c:yMode val=\"edge\"/><c:x val=\"%2\"/><c:y val=\"%3\"/>%4</c:manualLayout></c:layout>")
            .arg(target.isEmpty() ? QString() : QStringLiteral("<c:layoutTarget val=\"%1\"/>").arg(target)).arg(x).arg(y)
            .arg(w > 0 ? QStringLiteral("<c:w val=\"%1\"/><c:h val=\"%2\"/>").arg(w).arg(h) : QString());
    }
    static QByteArray withPlotLayout(const QByteArray &part, const QString &layout)
    {
        QByteArray out = part;
        out.replace("<c:plotArea><c:layout/>", ("<c:plotArea>" + layout).toUtf8());
        return out;
    }
    static Chart readOrFail(const QByteArray &part)
    {
        Chart c;
        QString err;
        if (!readChart(part, defaultTheme(), ColorMap(), &c, &err)) qFatal("%s", qPrintable(err));
        return c;
    }

    // The legend's own place (c:legend's c:layout) is read: where its corner is, and how large it is if the file says; a file that gives no place, or none that can be used, has none.
    void chartReadsTheLegendsOwnPlace()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {1, 0});
        auto legend = [](const QString &layout) { return QStringLiteral("<c:legend><c:legendPos val=\"r\"/>") + layout + QStringLiteral("<c:overlay val=\"0\"/></c:legend>"); };
        Chart c = readOrFail(chartPart(plot, QString(), legend(manualLayout(0.15, 0.2, 0.3, 0.1))));
        QVERIFY(c.legend.manual);
        QVERIFY(c.legend.edge);
        QCOMPARE(c.legend.x, 0.15);
        QCOMPARE(c.legend.y, 0.2);
        QCOMPARE(c.legend.w, 0.3);
        QCOMPARE(c.legend.h, 0.1);
        // only a corner: its size is the entries'
        c = readOrFail(chartPart(plot, QString(), legend(manualLayout(0.4, 0.5, 0, 0))));
        QVERIFY(c.legend.manual);
        QCOMPARE(c.legend.w, 0.0);
        // a place that is how far to move it from where its position puts it
        QString factor = manualLayout(0.1, -0.05, 0.2, 0.2);
        factor.replace(QStringLiteral("<c:xMode val=\"edge\"/><c:yMode val=\"edge\"/>"), QStringLiteral("<c:xMode val=\"factor\"/><c:yMode val=\"factor\"/>"));
        c = readOrFail(chartPart(plot, QString(), legend(factor)));
        QVERIFY(c.legend.manual);
        QVERIFY(!c.legend.edge);
        QCOMPARE(c.legend.y, -0.05);
        // none, and what is out of range
        QVERIFY(!readOrFail(chartPart(plot, QString(), legend(QString()))).legend.manual);
        c = readOrFail(chartPart(plot, QString(), legend(manualLayout(7, -3, 9, 0.2))));
        QVERIFY(c.legend.manual);
        QVERIFY(c.legend.x >= 0 && c.legend.x <= 1 && c.legend.y >= 0 && c.legend.y <= 1 && c.legend.w <= 1);
        // the same for the legend's own fill and line: read when the file gives them
        QString filled = legend(QString());
        filled.replace(QStringLiteral("<c:overlay val=\"0\"/>"), QStringLiteral("<c:overlay val=\"0\"/><c:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></a:ln></c:spPr>"));
        c = readOrFail(chartPart(plot, QString(), filled));
        QCOMPARE(c.legend.fill, QColor(0, 255, 0));
        QVERIFY(c.legend.border.set);
        QCOMPARE(c.legend.border.color, QColor(0, 0, 255));
    }

    // The legend is where the file puts it: its key (a small square in the series' color) is inside the place, and not at the side its position says.
    void aLegendWithItsOwnPlaceIsDrawnThere()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {1, 0});
        const Chart c = readOrFail(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/>") + manualLayout(0.55, 0.05, 0.4, 0.15) + QStringLiteral("<c:overlay val=\"0\"/></c:legend>")));
        const QSizeF size(400, 300);
        const QImage img = renderChart(c, size, 2);
        const QVector<QPointF> keys = colorComponents(img, Qt::red, 4);
        // (the first column is the red at the left; the key is the red patch at the right)
        QVERIFY(keys.size() >= 2);
        const QPointF key = keys.last();
        const QRectF place(0.55 * size.width() * 2, 0.05 * size.height() * 2, 0.4 * size.width() * 2, 0.15 * size.height() * 2);
        QVERIFY2(place.contains(key), qPrintable(QStringLiteral("%1,%2").arg(key.x()).arg(key.y())));
        // a legend of its own size and place (the file gives the width and height) draws its name inside the place
        QRect text;
        for (int y = int(place.top()); y < int(place.bottom()); ++y)
            for (int x = int(place.left()); x < int(place.right()); ++x)
                if (qRed(img.pixel(x, y)) < 90 && qGreen(img.pixel(x, y)) < 90 && qBlue(img.pixel(x, y)) < 90) text = text.isNull() ? QRect(x, y, 1, 1) : text.united(QRect(x, y, 1, 1));
        QVERIFY(!text.isNull());
        QVERIFY(text.left() > key.x());
    }

    // A legend over the plot (its overlay on) is drawn after it, so a column does not cover its name.
    void aLegendOverThePlotIsNotCovered()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {0, 1});
        const Chart c = readOrFail(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"tr\"/><c:overlay val=\"1\"/></c:legend>")));
        const QSizeF size(400, 300);
        const QImage img = renderChart(c, size, 2);
        // the column at the right is red from the top of the plot down, under the legend; the legend's name is dark on it
        int dark = 0;
        for (int y = 0; y < int(0.12 * img.height()); ++y)
            for (int x = int(0.55 * img.width()); x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                if (qRed(p) < 80 && qGreen(p) < 80 && qBlue(p) < 80) ++dark;
            }
        QVERIFY2(dark > 30, qPrintable(QString::number(dark)));
        // and what is laid over the column is the legend's own white, when the file gives the legend a fill
        QString filled = QStringLiteral("<c:legend><c:legendPos val=\"tr\"/><c:overlay val=\"1\"/><c:spPr><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill></c:spPr></c:legend>");
        const QImage white = renderChart(readOrFail(chartPart(plot, QString(), filled)), size, 2);
        // (inside the legend's corner: the column's red where the legend has no fill, the legend's white where it has)
        QCOMPARE(white.pixel(img.width() - 40, 48), QColor(Qt::white).rgb());
        QCOMPARE(img.pixel(img.width() - 40, 48), QColor(Qt::red).rgb());
    }

    // A plot the file fixes (as a rectangle of the chart) does not run under a legend that takes room at its side.
    void aPlotTheFileFixesStaysOffTheLegend()
    {
        const QString plot = oneSeriesPlot(QStringLiteral("FF0000"), {1, 1, 1});
        const QByteArray part = withPlotLayout(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"0\"/></c:legend>")), manualLayout(0.05, 0.05, 0.9, 0.8, QStringLiteral("inner")));
        const Chart c = readOrFail(part);
        QVERIFY(c.plotManual);
        const QImage img = renderChart(c, QSizeF(400, 300), 2);
        // the columns touch and fill the plot's width: where they end is where the plot does; the key (a red square) is at the legend, and the plot stops short of it (apart from it: two
        // patches of red, not one that the plot runs into the legend with)
        const QVector<QRect> reds = colorRuns(img, Qt::red, false);
        QCOMPARE(reds.size(), 2);
        QVERIFY2(reds[0].right() < reds[1].left() - 4, qPrintable(QStringLiteral("%1 %2").arg(reds[0].right()).arg(reds[1].left())));
        // with the legend moved over the plot, nothing is taken from it
        const Chart over = readOrFail(withPlotLayout(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/><c:overlay val=\"1\"/></c:legend>")), manualLayout(0.05, 0.05, 0.9, 0.8, QStringLiteral("inner"))));
        const QImage full = renderChart(over, QSizeF(400, 300), 2);
        QVERIFY(colorBounds(full, Qt::red, false).right() > full.width() * 0.9);
    }

    // The title of an axis stands at the edge of the chart, outside the plot whatever the plot's own place, and where its own layout puts it, if it has one.
    void axisTitlesStandOutsideAPlotTheFileFixes()
    {
        const QString title = QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1000\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1000\"><a:solidFill><a:srgbClr val=\"FF00FF\"/></a:solidFill></a:rPr><a:t>%1</a:t></a:r></a:p></c:rich></c:tx>%2<c:overlay val=\"0\"/></c:title>");
        const QString plot = oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5}, QStringLiteral("Dollars"), QString(title).replace(QStringLiteral("%2"), QString()));
        const QSizeF size(400, 300);
        const QString inner = manualLayout(0.2, 0.1, 0.75, 0.7, QStringLiteral("inner"));
        const Chart c = readOrFail(withPlotLayout(chartPart(plot), inner));
        QVERIFY(c.plotManual && c.plotInner);
        const QImage img = renderChart(c, size, 2);
        const QRect text = colorBounds(img, QColor(255, 0, 255), false);
        QVERIFY(!text.isNull());
        // the title is a column of turned text at the left edge of the chart, left of the plot (and of the labels along it)
        QVERIFY2(text.right() < 0.2 * img.width() - 12, qPrintable(QString::number(text.right())));
        QVERIFY2(text.left() < 0.08 * img.width(), qPrintable(QString::number(text.left())));
        // the same with a plot whose place is the whole of what the plot and its labels take (an outer target)
        const Chart outerPlot = readOrFail(withPlotLayout(chartPart(plot), manualLayout(0.2, 0.1, 0.75, 0.7, QStringLiteral("outer"))));
        const QRect outerText = colorBounds(renderChart(outerPlot, size, 2), QColor(255, 0, 255), false);
        QVERIFY(!outerText.isNull());
        QVERIFY2(outerText.left() >= 0.2 * 2 * size.width() - 6 && outerText.left() < 0.2 * 2 * size.width() + 20, qPrintable(QString::number(outerText.left())));
        // a title with a place of its own: its corner is where the layout says (as fractions of the chart)
        const QString placed = QString(title).replace(QStringLiteral("%2"), manualLayout(0.02, 0.30, 0, 0));
        const Chart own = readOrFail(withPlotLayout(chartPart(oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5}, QStringLiteral("Dollars"), placed)), inner));
        QVERIFY(own.axes.size() >= 2);
        bool found = false;
        for (const ChartAxis &a : own.axes)
            if (a.title.manual) {
                found = true;
                QCOMPARE(a.title.x, 0.02);
                QCOMPARE(a.title.y, 0.30);
            }
        QVERIFY(found);
        const QRect put = colorBounds(renderChart(own, size, 2), QColor(255, 0, 255), false);
        QVERIFY(!put.isNull());
        // (the box of the turned text starts there; the letters a little in from its sides)
        QVERIFY2(put.left() >= 0.02 * 2 * size.width() - 2 && put.left() <= 0.02 * 2 * size.width() + 12, qPrintable(QString::number(put.left())));
        QVERIFY2(put.top() >= 0.30 * 2 * size.height() - 2 && put.top() <= 0.30 * 2 * size.height() + 12, qPrintable(QString::number(put.top())));
        // the title of the category axis, the same way
        const QString catPlaced = QString(title).replace(QStringLiteral("%2"), manualLayout(0.40, 0.90, 0, 0));
        const Chart cat = readOrFail(withPlotLayout(chartPart(oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5}, QString(), QString(), catPlaced.arg(QStringLiteral("Months")))), inner));
        const QRect catPut = colorBounds(renderChart(cat, size, 2), QColor(255, 0, 255), false);
        QVERIFY(!catPut.isNull());
        QVERIFY2(catPut.left() >= 0.40 * 2 * size.width() - 2 && catPut.left() <= 0.40 * 2 * size.width() + 12 && catPut.top() >= 0.90 * 2 * size.height() - 2 && catPut.top() <= 0.90 * 2 * size.height() + 14,
                 qPrintable(QStringLiteral("%1,%2").arg(catPut.left()).arg(catPut.top())));
    }

    // The title of the chart is across the top, or where its own layout puts its corner (as fractions of the chart); the plot is laid out the same either way.
    void aChartTitleStandsWhereItsLayoutPutsIt()
    {
        auto titled = [](const QString &layout) {
            return QStringLiteral("<c:title><c:tx><c:rich><a:bodyPr/><a:p><a:pPr><a:defRPr sz=\"1400\"/></a:pPr><a:r><a:rPr lang=\"en-US\" sz=\"1400\"><a:solidFill><a:srgbClr val=\"FF00FF\"/></a:solidFill></a:rPr><a:t>Sales</a:t></a:r></a:p></c:rich></c:tx>%1<c:overlay val=\"0\"/></c:title>")
                .arg(layout);
        };
        const QString plot = oneSeriesPlot(QStringLiteral("0000FF"), {1, 0.5});
        const QSizeF size(400, 300);
        const QColor magenta(255, 0, 255), blue(0, 0, 255);
        Chart usual = readOrFail(chartPart(plot, titled(QString())));
        QVERIFY(usual.title.present && !usual.title.manual);
        const QImage across = renderChart(usual, size, 2);
        const QRect centered = colorBounds(across, magenta, false);
        QVERIFY(!centered.isNull());
        QVERIFY2(qAbs(centered.center().x() - int(size.width())) <= 8, qPrintable(QString::number(centered.center().x())));   // (the middle of the 800 pixels)
        QVERIFY2(centered.top() < 40, qPrintable(QString::number(centered.top())));
        Chart own = readOrFail(chartPart(plot, titled(manualLayout(0.55, 0.30, 0, 0))));
        QVERIFY(own.title.manual);
        QCOMPARE(own.title.x, 0.55);
        QCOMPARE(own.title.y, 0.30);
        const QImage placed = renderChart(own, size, 2);
        const QRect put = colorBounds(placed, magenta, false);
        QVERIFY(!put.isNull());
        // (the box of the text starts at the corner; the letters a little in from it)
        QVERIFY2(put.left() >= 0.55 * 2 * size.width() - 2 && put.left() <= 0.55 * 2 * size.width() + 12, qPrintable(QString::number(put.left())));
        QVERIFY2(put.top() >= 0.30 * 2 * size.height() - 2 && put.top() <= 0.30 * 2 * size.height() + 14, qPrintable(QString::number(put.top())));
        // the plot is where it was (the room the title would have taken is kept)
        QCOMPARE(colorBounds(placed, blue, false), colorBounds(across, blue, false));
    }

    // A pie of negative values is drawn by their size, as the common programs draw it: the same as the pie of their absolute values.
    void aPieOfNegativeValuesIsDrawnByTheirSize()
    {
        auto pie = [&](const QVector<double> &values) {
            const QString plot = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, values) + QStringLiteral("<c:firstSliceAng val=\"0\"/></c:pieChart>");
            return renderChart(readOrFail(chartPart(plot)), QSizeF(300, 300), 2);
        };
        const QImage positive = pie({3, 1, 2});
        QVERIFY(colorBounds(positive, QColor(), true).width() > 200);
        QCOMPARE(pie({-3, -1, -2}), positive);
        QCOMPARE(pie({3, -1, 2}), positive);
        // values of nothing are no slice, and nothing at all is a blank page
        QVERIFY(colorBounds(pie({3, 0, 2}), QColor(), true).width() > 200);
        QVERIFY(colorBounds(pie({0, 0, 0}), QColor(), true).isNull());
    }

    // A number format code is at most 255 characters (the spreadsheet limit); a longer one is not applied, and the numbers show as they are.
    void chartIgnoresFormatCodesPastTheSpreadsheetLimit()
    {
        const QString fits = QStringLiteral("0.0") + QString(252, QLatin1Char('x')), tooLong = fits + QLatin1Char('x');
        QCOMPARE(fits.size(), 255);
        QCOMPARE(formatChartNumber(1234.5, fits), QStringLiteral("1234.5") + QString(252, QLatin1Char('x')));
        QCOMPARE(formatChartNumber(1234.5, tooLong), QStringLiteral("1234.5"));
        // The same through a chart: 100 categories that are numbers, with a code of 20,000 characters, read in no time.
        QString points;
        for (int i = 0; i < 100; ++i) points += QStringLiteral("<c:pt idx=\"%1\"><c:v>%1.5</c:v></c:pt>").arg(i);
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>"
                                            "<c:cat><c:numRef><c:f>Sheet1!$A$2:$A$101</c:f><c:numCache><c:formatCode>0.0%1</c:formatCode><c:ptCount val=\"100\"/>%2</c:numCache></c:numRef></c:cat>"
                                            "<c:val><c:numRef><c:f>Sheet1!$B$2:$B$101</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"100\"/>%2</c:numCache></c:numRef></c:val></c:ser>"
                                            "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>")
                                 .arg(QString(20000, QLatin1Char('x')), points) + chartAxes();
        Chart c;
        QString err;
        const qint64 ms = bestNanoseconds([&] {
            QVERIFY2(readChart(chartPart(plot), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        }, 5, [&] { c = Chart(); }) / 1000000;
        QVERIFY2(ms < 500, qPrintable(QString::number(ms)));
        QCOMPARE(c.plots[0].series[0].categories[3], QStringLiteral("3.5"));
    }

    // The text a chart keeps for a label is cut at a few hundred characters, and what can't be written in a picture is left out of it.
    void chartKeepsLabelsShortAndWritable()
    {
        const QString huge(100000, QLatin1Char('m'));
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                             + chartSeries(0, huge, {huge, QStringLiteral("b")}, {1, 2}) + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.plots[0].series[0].categories[0].size() <= kChartMaxLabel);
        QVERIFY(c.plots[0].series[0].name.size() <= kChartMaxLabel);
        QVERIFY2(chartSvg(c, QSizeF(400, 300)).size() < 100000, "the picture repeats the long text");
        // Text with characters XML has no place for (control characters, half a surrogate pair) still makes a picture a reader can parse.
        c.plots[0].series[0].categories[1] = QStringLiteral("a") + QChar(1) + QStringLiteral("b") + QChar(7) + QStringLiteral("c") + QChar(0xD800) + QChar(0xFFFE) + QStringLiteral("<&\">'");
        c.plots[0].series[0].name = QStringLiteral("n") + QChar(2) + QChar(0xDC00);
        const QByteArray svg = chartSvg(c, QSizeF(400, 300));
        QXmlStreamReader xml(svg);
        while (!xml.atEnd()) xml.readNext();
        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QVERIFY(QSvgRenderer(svg).isValid());
    }

    // However much data a chart holds, its picture is a few thousand tags at most: the time QSvgRenderer takes to read a picture grows with the
    // square of them, and 20,000 columns took minutes.
    void chartDrawsHugeDataInFewPieces()
    {
        const int n = 6000;
        QStringList names;
        QVector<double> values, blanks;
        for (int i = 0; i < n; ++i) {
            names << QStringLiteral("c%1").arg(i);
            values << 10 + (i * 7) % 50;
            blanks << (i % 2 ? values.last() : std::nan(""));
        }
        const QString labels = QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"1\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/></c:dLbls>");
        const QString bar = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const QString area = QStringLiteral("<c:areaChart><c:grouping val=\"standard\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:areaChart>");
        const QString pie = QStringLiteral("<c:pieChart><c:varyColors val=\"1\"/>%1</c:pieChart>");
        const QString legend = QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>");
        QString xs, ys;
        for (int i = 0; i < n; ++i) {
            xs += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg((i * 37) % 1000);
            ys += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(values[i]);
        }
        const QString scatter = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:xVal><c:numRef><c:f>S!$A$1</c:f><c:numCache><c:formatCode>General</c:formatCode>"
                                               "<c:ptCount val=\"%1\"/>%2</c:numCache></c:numRef></c:xVal><c:yVal><c:numRef><c:f>S!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"%1\"/>%3</c:numCache></c:numRef></c:yVal></c:ser>"
                                               "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>").arg(n).arg(xs, ys);
        QString everyLabel = chartAxes(), tinyUnit = chartAxes();
        everyLabel.replace(QStringLiteral("<c:crossAx val=\"222\"/>"), QStringLiteral("<c:crossAx val=\"222\"/><c:tickLblSkip val=\"1\"/>"));
        tinyUnit.replace(QStringLiteral("<c:crossBetween val=\"between\"/>"), QStringLiteral("<c:crossBetween val=\"between\"/><c:majorUnit val=\"0.001\"/>"));
        struct Case {
            const char *name;
            QString plot, legend;
        };
        const Case cases[] = {
            {"columns", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes(), QString()},
            {"columns with a label on each", bar.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)) + chartAxes(), legend},
            {"columns with every category named", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + everyLabel, QString()},
            {"columns on a scale of tiny steps", bar.arg(chartSeries(0, QStringLiteral("S"), names, values)) + tinyUnit, QString()},
            {"many series of columns", bar.arg(chartSeries(0, QStringLiteral("A"), names.mid(0, 2000), values.mid(0, 2000)) + chartSeries(1, QStringLiteral("B"), names.mid(0, 2000), values.mid(0, 2000))
                                               + chartSeries(2, QStringLiteral("C"), names.mid(0, 2000), values.mid(0, 2000))) + chartAxes(), legend},
            {"line with markers", line.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes(), legend},
            {"line with a label on each", line.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)) + chartAxes(), QString()},
            {"area broken at blanks", area.arg(chartSeries(0, QStringLiteral("S"), names, blanks)) + chartAxes(), QString()},
            {"scatter", scatter + chartAxes(), legend},
            {"pie", pie.arg(chartSeries(0, QStringLiteral("S"), names, values)), legend},
            {"pie with a label on each", pie.arg(chartSeries(0, QStringLiteral("S"), names, values, QString(), labels)), QString()},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.plot, QString(), k.legend), defaultTheme(), ColorMap(), &c, &err), k.name);
            QByteArray svg;
            std::unique_ptr<QSvgRenderer> r;
            QImage img(600, 400, QImage::Format_ARGB32_Premultiplied);
            const qint64 ms = bestNanoseconds([&] {
                svg = chartSvg(c, QSizeF(480, 320));
                r = std::make_unique<QSvgRenderer>(svg);
                // And drawn, as a picture is for the page it is on.
                img.fill(Qt::white);
                QPainter painter(&img);
                r->render(&painter, QRectF(QPointF(), QSizeF(img.size())));
                painter.end();
            }, 3, [&] { r.reset(); }) / 1000000;
            const qint64 tags = svg.count('<');
            QVERIFY2(tags <= 8000, qPrintable(QStringLiteral("%1: %2 tags").arg(QLatin1String(k.name)).arg(tags)));
            QVERIFY2(r->isValid(), k.name);
            QVERIFY2(ms < 20000, qPrintable(QStringLiteral("%1: %2 ms").arg(QLatin1String(k.name)).arg(ms)));
            // It is still a picture of the chart: its axis, and (but for a pie) the plot's ink, are there.
            QVERIFY2(tags > 10, k.name);
        }
    }

    // A chart too big to draw says so, and one that is not does not.
    void chartSaysWhenItIsDrawnCoarser()
    {
        QStringList names;
        QVector<double> values;
        for (int i = 0; i < 12000; ++i) {
            names << QStringLiteral("c%1").arg(i);
            values << i % 90;
        }
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(line.arg(chartSeries(0, QStringLiteral("S"), names, values)) + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        bool coarser = false;
        const QByteArray big = chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(coarser);
        QVERIFY(big.size() < 2000000);
        QVERIFY(readChart(chartPart(line.arg(chartSeries(0, QStringLiteral("S"), names.mid(0, 200), values.mid(0, 200))) + chartAxes()), defaultTheme(), ColorMap(), &c, &err));
        coarser = true;
        chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(!coarser);
        // So is one that has points enough, but more labels on them than can be read.
        const QString labels = QStringLiteral("<c:dLbls><c:showLegendKey val=\"0\"/><c:showVal val=\"1\"/><c:showCatName val=\"0\"/><c:showSerName val=\"0\"/><c:showPercent val=\"0\"/></c:dLbls>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        QVERIFY(readChart(chartPart(bars.arg(chartSeries(0, QStringLiteral("S"), names.mid(0, 800), values.mid(0, 800), QString(), labels)) + chartAxes()), defaultTheme(), ColorMap(), &c, &err));
        coarser = false;
        chartSvg(c, QSizeF(480, 320), &coarser);
        QVERIFY(coarser);
    }

    // A font name with the characters an SVG attribute can't hold doesn't make the whole chart vanish.
    void chartSurvivesAnyFontName()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("Only series"), {"a", "b"}, {1, 2})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes();
        const QString legend = QStringLiteral("<c:legend><c:legendPos val=\"r\"/></c:legend>");
        for (const QString &name : {QStringLiteral("Bad\"Name"), QStringLiteral("A<b>C"), QStringLiteral("R&amp;D"), QStringLiteral("It's"), QStringLiteral("x\"/><script>y"), QStringLiteral("a") + QChar(1) + QStringLiteral("b"), QString(5000, QLatin1Char('f'))}) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(plot, QString(), legend), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
            // As the file names it (through a typeface attribute, which XML escapes), and as another reader might set it.
            c.font.family = name;
            c.fontFamily = name;
            const QByteArray svg = chartSvg(c, QSizeF(400, 300));
            QXmlStreamReader xml(svg);
            while (!xml.atEnd()) xml.readNext();
            QVERIFY2(!xml.hasError(), qPrintable(name.left(20) + QLatin1Char(' ') + xml.errorString()));
            QVERIFY2(svg.size() < 100000, "the font name is written once for every text");
            QVERIFY2(svg.contains(">Only series<"), qPrintable(name.left(20)));
            QVERIFY2(!svg.contains("<script"), qPrintable(name.left(20)));
            QVERIFY(QSvgRenderer(svg).isValid());
        }
    }

    // Numbers no picture can hold (a size or an axis of 1e300, values of 1e308) draw a picture and not coordinates that are not numbers.
    void chartSurvivesNumbersBeyondAPicture()
    {
        const QString plot = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, 2, 3})
                             + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        QString huge = chartAxes();
        huge.replace(QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/>"),
                     QStringLiteral("<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/><c:max val=\"1e308\"/><c:min val=\"-1e308\"/>"));
        static const QRegularExpression notANumber(QStringLiteral("(?<![A-Za-z])(nan|-?inf)(?![A-Za-z])"));
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(plot + huge), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QVERIFY(c.axes.size() == 2 && !c.axes[1].autoMax);
        const QByteArray big = chartSvg(c, QSizeF(400, 300));
        QVERIFY2(!QString::fromUtf8(big).contains(notANumber), qPrintable(notANumber.match(QString::fromUtf8(big)).captured()));
        QVERIFY(QSvgRenderer(big).isValid());
        // Values past what an axis can be made of are blanks.
        for (const double v : {1e308, -1e308, 1e200}) {
            Chart d;
            QVERIFY2(readChart(chartPart(QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>") + chartSeries(0, QStringLiteral("S"), {"a", "b", "c"}, {1, v, -v})
                                         + QStringLiteral("<c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>") + chartAxes()), defaultTheme(), ColorMap(), &d, &err), qPrintable(err));
            const QString svg = QString::fromUtf8(chartSvg(d, QSizeF(400, 300)));
            QVERIFY2(!svg.contains(notANumber), qPrintable(QString::number(v)));
            QVERIFY(QSvgRenderer(svg.toUtf8()).isValid());
        }
        // A size is at most a slide's longest side, and a size that is no number is a point.
        const QString huger = QString::fromUtf8(chartSvg(c, QSizeF(1e300, 1e300)));
        QVERIFY2(huger.contains(QStringLiteral("viewBox=\"0 0 4032 4032\"")), qPrintable(huger.left(300)));
        const QString none = QString::fromUtf8(chartSvg(c, QSizeF(std::nan(""), std::numeric_limits<double>::infinity())));
        QVERIFY2(!none.contains(notANumber), qPrintable(none.left(300)));
    }

    // Gridlines, the marks of axes, and markers drawn one by one when a chart has few, and as one path when it has many, and a line of many
    // points drawn as one path or in paths of some segments each, make the same picture.
    void chartMergedPiecesLookTheSame()
    {
        const QString dashed = QStringLiteral("<c:spPr><a:ln w=\"9525\"><a:solidFill><a:srgbClr val=\"808080\"/></a:solidFill><a:prstDash val=\"dash\"/></a:ln></c:spPr>");
        QStringList names;
        QVector<double> a, b, walk;
        double at = 50;
        for (int i = 0; i < 40; ++i) {
            names << QStringLiteral("cat %1").arg(i);
            a << 20 + (i * 13) % 37;
            b << 10 + (i * 29) % 41;
        }
        QStringList manyNames;
        for (int i = 0; i < 1500; ++i) {
            manyNames << QString::number(i);
            at += ((i * 7919) % 31) - 15;
            walk << at;
        }
        QString dashedAxes = chartAxes();
        dashedAxes.replace(QStringLiteral("<c:majorGridlines/>"), QStringLiteral("<c:majorGridlines>") + dashed + QStringLiteral("</c:majorGridlines>"));
        const QString red = QStringLiteral("<c:spPr><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln></c:spPr>");
        const QString bars = QStringLiteral("<c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>%1<c:gapWidth val=\"60\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:barChart>");
        const QString line = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"0\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        struct Case {
            const char *name;
            QString plot;
            int separateLines, segments, separateMarkers;   // what makes the picture another way, against the usual
        };
        const QString dots = QStringLiteral("<c:marker><c:symbol val=\"circle\"/><c:size val=\"6\"/></c:marker>"), crosses = QStringLiteral("<c:marker><c:symbol val=\"x\"/><c:size val=\"6\"/></c:marker>");
        const QString withMarkers = QStringLiteral("<c:lineChart><c:grouping val=\"standard\"/>%1<c:marker val=\"1\"/><c:axId val=\"111\"/><c:axId val=\"222\"/></c:lineChart>");
        const Case cases[] = {
            {"columns with gridlines and ticks", bars.arg(chartSeries(0, QStringLiteral("A"), names, a, red) + chartSeries(1, QStringLiteral("B"), names, b)) + chartAxes(), 0, 0, -1},
            {"columns with dashed gridlines", bars.arg(chartSeries(0, QStringLiteral("A"), names, a)) + dashedAxes, 0, 0, -1},
            {"a line of 1,500 points", line.arg(chartSeries(0, QStringLiteral("A"), manyNames, walk)) + chartAxes(), -1, 20, -1},
            {"a line of 1,500 points, one path", line.arg(chartSeries(0, QStringLiteral("A"), manyNames, walk)) + chartAxes(), -1, 100000, -1},
            {"lines with markers", withMarkers.arg(chartSeries(0, QStringLiteral("A"), names, a, QString(), dots) + chartSeries(1, QStringLiteral("B"), names, b, QString(), crosses)) + chartAxes(), -1, 0, 0},
        };
        for (const Case &k : cases) {
            Chart c;
            QString err;
            QVERIFY2(readChart(chartPart(k.plot, QString(), QStringLiteral("<c:legend><c:legendPos val=\"b\"/></c:legend>")), defaultTheme(), ColorMap(), &c, &err), k.name);
            const QSizeF size(500, 300);
            auto draw = [&](const QByteArray &svg) {
                QSvgRenderer r(svg);
                QImage img(1000, 600, QImage::Format_ARGB32);
                img.fill(Qt::white);
                QPainter p(&img);
                r.render(&p, QRectF(QPointF(), QSizeF(img.size())));
                return img;
            };
            const QByteArray usual = chartSvg(c, size), other = chartSvgTuned(c, size, k.separateLines, k.segments, k.separateMarkers);
            QVERIFY2(usual != other, qPrintable(QStringLiteral("%1 is drawn the same way both times").arg(QLatin1String(k.name))));
            const QImage one = draw(usual), two = draw(other);
            // Where two strokes meet the edges come out a shade apart (as dark as the strokes' own where they overlap); nothing else may: what
            // differs is a small part of the ink of the picture.
            double differs = 0, ink = 0;
            for (int y = 0; y < one.height(); ++y)
                for (int x = 0; x < one.width(); ++x) {
                    ink += 255 - qGray(one.pixel(x, y));
                    differs += qAbs(qGray(one.pixel(x, y)) - qGray(two.pixel(x, y)));
                }
            QVERIFY2(ink > 20000, qPrintable(QStringLiteral("%1 draws no ink").arg(QLatin1String(k.name))));
            QVERIFY2(differs <= 0.02 * ink, qPrintable(QStringLiteral("%1: %2 of %3").arg(QLatin1String(k.name)).arg(differs).arg(ink)));
        }
    }

    // A line that crosses its plot back and forth thousands of times takes seconds to fill; drawn from fewer of its points, it takes a fraction of one.
    void chartDrawsLinesThatCrossTheirPlotQuickly()
    {
        QString xs, ys;
        quint32 state = 12345;
        auto next = [&] { state = state * 1664525u + 1013904223u; return (state >> 8) % 1000; };
        for (int i = 0; i < 2000; ++i) {
            xs += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(next());
            ys += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(next());
        }
        const QString scatter = QStringLiteral("<c:scatterChart><c:scatterStyle val=\"lineMarker\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/><c:spPr><a:ln w=\"28575\"><a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill></a:ln></c:spPr>"
                                               "<c:marker><c:symbol val=\"none\"/></c:marker><c:xVal><c:numRef><c:f>S!$A$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"2000\"/>%1</c:numCache></c:numRef></c:xVal>"
                                               "<c:yVal><c:numRef><c:f>S!$B$1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"2000\"/>%2</c:numCache></c:numRef></c:yVal></c:ser>"
                                               "<c:axId val=\"111\"/><c:axId val=\"222\"/></c:scatterChart>").arg(xs, ys);
        Chart c;
        QString err;
        QVERIFY2(readChart(chartPart(scatter + chartAxes()), defaultTheme(), ColorMap(), &c, &err), qPrintable(err));
        QImage img;
        const qint64 ms = bestNanoseconds([&] { img = renderChart(c, QSizeF(500, 333), 2); }, 3) / 1000000;
        QVERIFY2(ms < 20000, qPrintable(QString::number(ms)));
        // It is still a picture of the line: the plot has plenty of red in it.
        QVERIFY(colorArea(img, Qt::red) > 2000);
    }

private:
    // A chart part: `body` (the plot area's kinds and axes) in a c:chart, after an optional title, with an optional legend.
    static QByteArray chartPart(const QString &body, const QString &title = QString(), const QString &legend = QString(), const QString &chartSpaceExtra = QString())
    {
        return (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" "
                               "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                               "<c:date1904 val=\"0\"/><c:roundedCorners val=\"0\"/><c:chart>")
                + title + (title.isEmpty() ? QStringLiteral("<c:autoTitleDeleted val=\"1\"/>") : QString())
                + QStringLiteral("<c:plotArea><c:layout/>") + body + QStringLiteral("</c:plotArea>") + legend
                + QStringLiteral("<c:plotVisOnly val=\"1\"/><c:dispBlanksAs val=\"gap\"/></c:chart>") + chartSpaceExtra + QStringLiteral("</c:chartSpace>")).toUtf8();
    }
    static QString chartAxes()
    {
        return QStringLiteral("<c:catAx><c:axId val=\"111\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"222\"/><c:crosses val=\"autoZero\"/><c:auto val=\"1\"/><c:lblAlgn val=\"ctr\"/>"
                              "<c:lblOffset val=\"100\"/><c:noMultiLvlLbl val=\"0\"/></c:catAx>"
                              "<c:valAx><c:axId val=\"222\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:numFmt formatCode=\"General\" sourceLinked=\"1\"/>"
                              "<c:majorTickMark val=\"out\"/><c:minorTickMark val=\"none\"/><c:tickLblPos val=\"nextTo\"/><c:crossAx val=\"111\"/><c:crosses val=\"autoZero\"/><c:crossBetween val=\"between\"/></c:valAx>");
    }
    // A series: its name, categories, and values as the file caches them (a NaN value is a point left out), then what the schema puts
    // after the color (markers, points, labels) and after the values.
    static QString chartSeries(int idx, const QString &name, const QStringList &categories, const QVector<double> &values, const QString &spPr = QString(), const QString &beforeData = QString(),
                               const QString &after = QString())
    {
        QString cats, vals;
        for (int i = 0; i < categories.size(); ++i) cats += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(categories[i]);
        for (int i = 0; i < values.size(); ++i)
            if (!std::isnan(values[i])) vals += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(values[i]);
        return QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/><c:tx><c:strRef><c:f>Sheet1!$%2$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>%3</c:v></c:pt></c:strCache></c:strRef></c:tx>")
                   .arg(idx).arg(QChar('B' + idx % 20)).arg(name)
               + spPr + beforeData
               + QStringLiteral("<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$%1</c:f><c:strCache><c:ptCount val=\"%2\"/>%3</c:strCache></c:strRef></c:cat>").arg(categories.size() + 1).arg(categories.size()).arg(cats)
               + QStringLiteral("<c:val><c:numRef><c:f>Sheet1!$B$2:$B$%1</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"%2\"/>%3</c:numCache></c:numRef></c:val>")
                     .arg(values.size() + 1).arg(values.size()).arg(vals)
               + after + QStringLiteral("</c:ser>");
    }
    // The chart drawn into a white picture, `scale` pixels to the point.
    static QImage renderChart(const Chart &chart, const QSizeF &size, int scale)
    {
        QSvgRenderer r(chartSvg(chart, size));
        QImage img(int(size.width() * scale), int(size.height() * scale), QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        r.render(&p, QRectF(QPointF(), QSizeF(img.size())));
        return img;
    }
    static bool isColor(QRgb p, const QColor &c, int tolerance)
    {
        return qAbs(qRed(p) - c.red()) <= tolerance && qAbs(qGreen(p) - c.green()) <= tolerance && qAbs(qBlue(p) - c.blue()) <= tolerance;
    }
    // The boxes of the separate patches of one color at least four pixels across and high (the bars of a chart), in order from the left, or from the top.
    static QVector<QRect> colorRuns(const QImage &img, const QColor &color, bool fromTop)
    {
        QVector<QRect> out;
        QVector<char> seen(img.width() * img.height(), 0);
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !isColor(img.pixel(x0, y0), color, 40)) continue;
                QRect box(x0, y0, 1, 1);
                QVector<QPoint> stack{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = 1;
                while (!stack.isEmpty()) {
                    const QPoint p = stack.takeLast();
                    box = box.united(QRect(p, QSize(1, 1)));
                    for (const QPoint d : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                        const QPoint q = p + d;
                        if (q.x() < 0 || q.y() < 0 || q.x() >= img.width() || q.y() >= img.height() || seen[q.y() * img.width() + q.x()] || !isColor(img.pixel(q), color, 40)) continue;
                        seen[q.y() * img.width() + q.x()] = 1;
                        stack << q;
                    }
                }
                if (box.width() >= 4 && box.height() >= 4) out << box;
            }
        std::sort(out.begin(), out.end(), [fromTop](const QRect &a, const QRect &b) { return fromTop ? a.top() < b.top() : a.left() < b.left(); });
        return out;
    }
    // The box of the pixels of one color; of everything that isn't white or nearly, for an invalid color.
    static QRect colorBounds(const QImage &img, const QColor &color, bool anyInk)
    {
        QRect box;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb p = img.pixel(x, y);
                const bool hit = anyInk || !color.isValid() ? (qRed(p) < 200 || qGreen(p) < 200 || qBlue(p) < 200) : isColor(p, color, 40);
                if (!hit) continue;
                box = box.isNull() ? QRect(x, y, 1, 1) : box.united(QRect(x, y, 1, 1));
            }
        return box;
    }
    static double colorArea(const QImage &img, const QColor &color)
    {
        double n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (isColor(img.pixel(x, y), color, 40)) ++n;
        return n;
    }
    static QPointF colorCentroid(const QImage &img, const QColor &color)
    {
        double n = 0, sx = 0, sy = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (isColor(img.pixel(x, y), color, 40)) {
                    ++n;
                    sx += x;
                    sy += y;
                }
        return n > 0 ? QPointF(sx / n, sy / n) : QPointF();
    }
    // The centers of the separate patches of one color with at least `minPixels` pixels, from the left.
    static QVector<QPointF> colorComponents(const QImage &img, const QColor &color, int minPixels)
    {
        QVector<QPointF> out;
        QVector<char> seen(img.width() * img.height(), 0);
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !isColor(img.pixel(x0, y0), color, 40)) continue;
                double n = 0, sx = 0, sy = 0;
                QVector<QPoint> stack{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = 1;
                while (!stack.isEmpty()) {
                    const QPoint p = stack.takeLast();
                    ++n;
                    sx += p.x();
                    sy += p.y();
                    for (const QPoint d : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                        const QPoint q = p + d;
                        if (q.x() < 0 || q.y() < 0 || q.x() >= img.width() || q.y() >= img.height() || seen[q.y() * img.width() + q.x()] || !isColor(img.pixel(q), color, 40)) continue;
                        seen[q.y() * img.width() + q.x()] = 1;
                        stack << q;
                    }
                }
                if (n >= minPixels) out << QPointF(sx / n, sy / n);
            }
        std::sort(out.begin(), out.end(), [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
        return out;
    }
};

JP_TEST_CLASS(OfficeChartTests)
#include "tst_office_chart.moc"
