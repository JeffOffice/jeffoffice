#include "office/chart_write.h"

#include "office/xml.h"

#include <QDomDocument>
#include <QHash>
#include <QSet>
#include <QXmlStreamWriter>

#include <algorithm>
#include <cmath>

namespace jp::office {

namespace {

const char kCNs[] = "http://schemas.openxmlformats.org/drawingml/2006/chart";
const char kANs[] = "http://schemas.openxmlformats.org/drawingml/2006/main";
const char kRNs[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

// Text an XML 1.0 document can hold: control characters are left out.
QString clean(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const ushort c = text[i].unicode();
        if (c == 0x9 || c == 0xA || c == 0xD || (c >= 0x20 && c < 0xD800) || (c >= 0xE000 && c < 0xFFFE)) out += text[i];
        else if (QChar::isHighSurrogate(c) && i + 1 < text.size() && QChar::isLowSurrogate(text[i + 1].unicode())) {
            out += text[i];
            out += text[++i];
        }
    }
    return out;
}

QString hex(const QColor &c) { return c.name(QColor::HexRgb).mid(1).toUpper(); }

QString number(double v)
{
    if (!std::isfinite(v)) return QStringLiteral("0");
    return QString::number(v, 'g', 15);
}

class Writer {
public:
    explicit Writer(const Chart &chart) : c(chart), w(&out)
    {
        w.setAutoFormatting(false);
    }

    QByteArray run()
    {
        w.writeStartDocument(QStringLiteral("1.0"), true);
        w.writeStartElement(QStringLiteral("c:chartSpace"));
        w.writeNamespace(QString::fromLatin1(kCNs), QStringLiteral("c"));
        w.writeNamespace(QString::fromLatin1(kANs), QStringLiteral("a"));
        w.writeNamespace(QString::fromLatin1(kRNs), QStringLiteral("r"));
        val("c:date1904", QStringLiteral("0"));
        val("c:lang", QStringLiteral("en-US"));
        val("c:roundedCorners", QStringLiteral("0"));
        val("c:style", QString::number(std::clamp(c.style, 1, 48)));
        chart();
        // the chart area
        shapeProps(c.background, false, c.border);
        if (hasFont(c.font)) textProps(c.font, false, 0);
        w.writeEndElement();
        w.writeEndDocument();
        return out;
    }

private:
    const Chart &c;
    QByteArray out;
    QXmlStreamWriter w;
    int m_serial = 0;

    void val(const char *name, const QString &v)
    {
        w.writeEmptyElement(QString::fromLatin1(name));
        w.writeAttribute(QStringLiteral("val"), v);
    }
    void flag(const char *name, bool v) { val(name, v ? QStringLiteral("1") : QStringLiteral("0")); }
    void text(const char *name, const QString &t)
    {
        w.writeTextElement(QString::fromLatin1(name), clean(t));
    }

    static bool hasFont(const ChartFont &f) { return f.size > 0 || f.bold >= 0 || f.italic >= 0 || f.color.isValid() || !f.family.isEmpty(); }

    void colorFill(const QColor &color)
    {
        w.writeStartElement(QStringLiteral("a:solidFill"));
        w.writeEmptyElement(QStringLiteral("a:srgbClr"));
        w.writeAttribute(QStringLiteral("val"), hex(color));
        if (color.alpha() < 255) {
            w.writeEmptyElement(QStringLiteral("a:alpha"));
            w.writeAttribute(QStringLiteral("val"), QString::number(int(std::lround(color.alphaF() * 100000))));
        }
        w.writeEndElement();
    }

    void line(const ChartLine &l, const QColor &fallback = QColor(), double fallbackWidth = 0, bool round = false)
    {
        if (!l.set && !fallback.isValid()) return;
        w.writeStartElement(QStringLiteral("a:ln"));
        const double width = l.width > 0 ? l.width : fallbackWidth;
        if (width > 0) w.writeAttribute(QStringLiteral("w"), QString::number(qint64(std::llround(width * 12700))));
        if (round) w.writeAttribute(QStringLiteral("cap"), QStringLiteral("rnd"));
        if (l.none) {
            w.writeEmptyElement(QStringLiteral("a:noFill"));
        } else {
            const QColor color = l.color.isValid() ? l.color : fallback;
            if (color.isValid()) colorFill(color);
            if (!l.dash.isEmpty()) val("a:prstDash", l.dash);
            if (round) w.writeEmptyElement(QStringLiteral("a:round"));
        }
        w.writeEndElement();
    }

    // c:spPr with a fill (invalid: none written; `noFill`: no fill) and a line.
    void shapeProps(const QColor &fill, bool noFill, const ChartLine &l, const QColor &lineFallback = QColor(), double lineWidth = 0, bool round = false)
    {
        if (!fill.isValid() && !noFill && !l.set && !lineFallback.isValid()) return;
        w.writeStartElement(QStringLiteral("c:spPr"));
        if (noFill) w.writeEmptyElement(QStringLiteral("a:noFill"));
        else if (fill.isValid()) colorFill(fill);
        line(l, lineFallback, lineWidth, round);
        w.writeEndElement();
    }

    void runProps(const char *element, const ChartFont &f, bool withLang)
    {
        w.writeStartElement(QString::fromLatin1(element));
        if (withLang) w.writeAttribute(QStringLiteral("lang"), QStringLiteral("en-US"));
        if (f.size > 0) w.writeAttribute(QStringLiteral("sz"), QString::number(int(std::lround(f.size * 100))));
        if (f.bold >= 0) w.writeAttribute(QStringLiteral("b"), f.bold ? QStringLiteral("1") : QStringLiteral("0"));
        if (f.italic >= 0) w.writeAttribute(QStringLiteral("i"), f.italic ? QStringLiteral("1") : QStringLiteral("0"));
        if (f.color.isValid()) colorFill(f.color);
        if (!f.family.isEmpty()) {
            w.writeEmptyElement(QStringLiteral("a:latin"));
            w.writeAttribute(QStringLiteral("typeface"), clean(f.family));
        }
        w.writeEndElement();
    }

    void bodyProps(bool hasRotation, double rotation)
    {
        w.writeEmptyElement(QStringLiteral("a:bodyPr"));
        if (hasRotation) {
            w.writeAttribute(QStringLiteral("rot"), QString::number(qint64(std::llround(rotation * 60000))));
            w.writeAttribute(QStringLiteral("vert"), QStringLiteral("horz"));
        }
    }

    void textProps(const ChartFont &f, bool hasRotation, double rotation)
    {
        w.writeStartElement(QStringLiteral("c:txPr"));
        bodyProps(hasRotation, rotation);
        w.writeEmptyElement(QStringLiteral("a:lstStyle"));
        w.writeStartElement(QStringLiteral("a:p"));
        w.writeStartElement(QStringLiteral("a:pPr"));
        runProps("a:defRPr", f, false);
        w.writeEndElement();
        w.writeEmptyElement(QStringLiteral("a:endParaRPr"));
        w.writeAttribute(QStringLiteral("lang"), QStringLiteral("en-US"));
        w.writeEndElement();
        w.writeEndElement();
    }

    void manualPlace(bool edge, double x, double y, double wd, double ht, const char *target = nullptr)
    {
        w.writeStartElement(QStringLiteral("c:layout"));
        w.writeStartElement(QStringLiteral("c:manualLayout"));
        if (target) val("c:layoutTarget", QString::fromLatin1(target));
        if (edge) {
            val("c:xMode", QStringLiteral("edge"));
            val("c:yMode", QStringLiteral("edge"));
        }
        val("c:x", number(x));
        val("c:y", number(y));
        if (wd > 0 && ht > 0) {
            val("c:w", number(wd));
            val("c:h", number(ht));
        }
        w.writeEndElement();
        w.writeEndElement();
    }

    void title(const ChartTitle &t, bool isAxis)
    {
        w.writeStartElement(QStringLiteral("c:title"));
        const bool own = !t.automatic && (!t.text.isEmpty() || !t.ref.isEmpty());
        if (own && !t.ref.isEmpty()) {
            w.writeStartElement(QStringLiteral("c:tx"));
            w.writeStartElement(QStringLiteral("c:strRef"));
            text("c:f", t.ref);
            w.writeStartElement(QStringLiteral("c:strCache"));
            val("c:ptCount", QStringLiteral("1"));
            w.writeStartElement(QStringLiteral("c:pt"));
            w.writeAttribute(QStringLiteral("idx"), QStringLiteral("0"));
            text("c:v", t.text);
            w.writeEndElement();
            w.writeEndElement();
            w.writeEndElement();
            w.writeEndElement();
        } else if (own) {
            w.writeStartElement(QStringLiteral("c:tx"));
            w.writeStartElement(QStringLiteral("c:rich"));
            // an axis title's usual turn (the value axis' runs up the side) is the reader's to decide; a turn the file said is written
            bodyProps(t.hasRotation, t.rotation);
            w.writeEmptyElement(QStringLiteral("a:lstStyle"));
            const QStringList lines = t.text.split(QLatin1Char('\n'));
            for (const QString &lineText : lines) {
                w.writeStartElement(QStringLiteral("a:p"));
                w.writeStartElement(QStringLiteral("a:pPr"));
                runProps("a:defRPr", t.font, false);
                w.writeEndElement();
                w.writeStartElement(QStringLiteral("a:r"));
                runProps("a:rPr", t.font, true);
                text("a:t", lineText);
                w.writeEndElement();
                w.writeEndElement();
            }
            w.writeEndElement();
            w.writeEndElement();
        }
        if (t.manual) manualPlace(true, t.x, t.y, 0, 0);
        flag("c:overlay", t.overlay);
        Q_UNUSED(isAxis);
        w.writeEndElement();
    }

    // A c:tx / c:cat / c:val with references and caches.
    void strCache(const QStringList &items)
    {
        w.writeStartElement(QStringLiteral("c:strCache"));
        val("c:ptCount", QString::number(items.size()));
        for (int i = 0; i < items.size(); ++i) {
            if (items[i].isEmpty()) continue;
            w.writeStartElement(QStringLiteral("c:pt"));
            w.writeAttribute(QStringLiteral("idx"), QString::number(i));
            text("c:v", items[i]);
            w.writeEndElement();
        }
        w.writeEndElement();
    }
    void numCache(const char *element, const QVector<double> &items, const QString &format)
    {
        w.writeStartElement(QString::fromLatin1(element));
        text("c:formatCode", format.isEmpty() ? QStringLiteral("General") : format);
        val("c:ptCount", QString::number(items.size()));
        for (int i = 0; i < items.size(); ++i) {
            if (std::isnan(items[i]) || !std::isfinite(items[i])) continue;
            w.writeStartElement(QStringLiteral("c:pt"));
            w.writeAttribute(QStringLiteral("idx"), QString::number(i));
            text("c:v", number(items[i]));
            w.writeEndElement();
        }
        w.writeEndElement();
    }

    void categoryData(const char *element, const ChartSeries &s)
    {
        const bool numeric = s.categoryRefNumeric && !s.categoryNumbers.isEmpty();
        if (s.categoryRef.isEmpty() && s.categories.isEmpty()) return;
        w.writeStartElement(QString::fromLatin1(element));
        if (!s.categoryRef.isEmpty()) {
            w.writeStartElement(QString::fromLatin1(numeric ? "c:numRef" : "c:strRef"));
            text("c:f", s.categoryRef);
            if (numeric) numCache("c:numCache", s.categoryNumbers, s.categoryFormat);
            else strCache(s.categories);
            w.writeEndElement();
        } else {
            w.writeStartElement(QStringLiteral("c:strLit"));
            val("c:ptCount", QString::number(s.categories.size()));
            for (int i = 0; i < s.categories.size(); ++i) {
                if (s.categories[i].isEmpty()) continue;
                w.writeStartElement(QStringLiteral("c:pt"));
                w.writeAttribute(QStringLiteral("idx"), QString::number(i));
                text("c:v", s.categories[i]);
                w.writeEndElement();
            }
            w.writeEndElement();
        }
        w.writeEndElement();
    }

    void numberData(const char *element, const QString &ref, const QVector<double> &items, const QString &format)
    {
        if (ref.isEmpty() && items.isEmpty()) return;
        w.writeStartElement(QString::fromLatin1(element));
        if (!ref.isEmpty()) {
            w.writeStartElement(QStringLiteral("c:numRef"));
            text("c:f", ref);
            numCache("c:numCache", items, format);
            w.writeEndElement();
        } else {
            numCache("c:numLit", items, format);
        }
        w.writeEndElement();
    }

    void marker(const ChartMarker &m, const QColor &seriesColor, bool bare)
    {
        // `bare`: markers are drawn by default; only a marker the model changes is written
        if (bare && m.symbol == ChartMarker::Auto && !m.fill.isValid() && !m.line.isValid() && !m.noFill && !m.noLine) return;
        static const char *names[] = {"auto", "none", "circle", "dash", "diamond", "dot", "plus", "square", "star", "triangle", "x"};
        w.writeStartElement(QStringLiteral("c:marker"));
        val("c:symbol", QString::fromLatin1(names[int(m.symbol)]));
        if (m.symbol != ChartMarker::None) {
            val("c:size", QString::number(int(std::lround(m.size))));
            const QColor fill = m.fill.isValid() ? m.fill : (m.symbol == ChartMarker::Auto ? QColor() : seriesColor);
            ChartLine ln;
            if (m.noLine) {
                ln.set = true;
                ln.none = true;
            } else if (m.line.isValid()) {
                ln.set = true;
                ln.color = m.line;
            }
            shapeProps(fill, m.noFill, ln);
        }
        w.writeEndElement();
    }

    void labels(const ChartDataLabels &l, bool pie)
    {
        if (!l.shown()) return;
        w.writeStartElement(QStringLiteral("c:dLbls"));
        if (!l.format.isEmpty()) {
            w.writeEmptyElement(QStringLiteral("c:numFmt"));
            w.writeAttribute(QStringLiteral("formatCode"), clean(l.format));
            w.writeAttribute(QStringLiteral("sourceLinked"), QStringLiteral("0"));
        }
        if (hasFont(l.font)) textProps(l.font, false, 0);
        if (!l.position.isEmpty()) val("c:dLblPos", l.position);
        flag("c:showLegendKey", false);
        flag("c:showVal", l.value);
        flag("c:showCatName", l.category);
        flag("c:showSerName", l.series);
        flag("c:showPercent", l.percent);
        flag("c:showBubbleSize", false);
        if (!l.separator.isEmpty()) text("c:separator", l.separator);
        if (pie) flag("c:showLeaderLines", true);
        w.writeEndElement();
    }

    void series(const ChartPlot &plot, const ChartSeries &s)
    {
        const bool stroked = plot.type == ChartType::Line || plot.type == ChartType::Scatter;
        const bool scatter = plot.type == ChartType::Scatter;
        const int index = m_serial++;
        w.writeStartElement(QStringLiteral("c:ser"));
        val("c:idx", QString::number(index));
        val("c:order", QString::number(index));
        // its name
        w.writeStartElement(QStringLiteral("c:tx"));
        if (!s.nameRef.isEmpty()) {
            w.writeStartElement(QStringLiteral("c:strRef"));
            text("c:f", s.nameRef);
            strCache(QStringList{s.name});
            w.writeEndElement();
        } else {
            text("c:v", s.name);
        }
        w.writeEndElement();
        // its look
        if (stroked) {
            ChartLine l = s.outline;
            if (scatter && !plot.lines) {
                l.set = true;
                l.none = true;
            }
            shapeProps(QColor(), false, l, s.color, 2.25, true);
        } else {
            shapeProps(s.noFill ? QColor() : s.color, s.noFill, s.outline);
        }
        if (stroked) marker(s.marker, s.color, true);
        if (plot.type == ChartType::Pie || plot.type == ChartType::Doughnut) {
            if (s.explosion > 0) val("c:explosion", QString::number(int(std::lround(s.explosion))));
        } else if (plot.type == ChartType::Bar) {
            flag("c:invertIfNegative", false);
        }
        for (auto it = s.pointColors.cbegin(); it != s.pointColors.cend(); ++it) {
            w.writeStartElement(QStringLiteral("c:dPt"));
            val("c:idx", QString::number(it.key()));
            if (plot.type == ChartType::Bar) flag("c:invertIfNegative", false);
            if (plot.type != ChartType::Pie && plot.type != ChartType::Doughnut) flag("c:bubble3D", false);
            shapeProps(it.value(), false, ChartLine());
            w.writeEndElement();
        }
        labels(s.labels, plot.type == ChartType::Pie || plot.type == ChartType::Doughnut);
        if (scatter) {
            numberData("c:xVal", s.xRef, s.xValues, s.categoryFormat);
            numberData("c:yVal", s.valueRef, s.values, s.format);
        } else {
            categoryData("c:cat", s);
            numberData("c:val", s.valueRef, s.values, s.format);
        }
        if (stroked) flag("c:smooth", s.smooth);
        w.writeEndElement();
    }

    static const char *groupingName(const ChartPlot &p)
    {
        switch (p.grouping) {
        case ChartGrouping::Clustered: return "clustered";
        case ChartGrouping::Stacked: return "stacked";
        case ChartGrouping::PercentStacked: return "percentStacked";
        case ChartGrouping::Standard: return "standard";
        }
        return "standard";
    }

    void plot(const ChartPlot &p)
    {
        const char *element = "c:barChart";
        switch (p.type) {
        case ChartType::Bar: element = "c:barChart"; break;
        case ChartType::Line: element = "c:lineChart"; break;
        case ChartType::Pie: element = "c:pieChart"; break;
        case ChartType::Doughnut: element = "c:doughnutChart"; break;
        case ChartType::Area: element = "c:areaChart"; break;
        case ChartType::Scatter: element = "c:scatterChart"; break;
        case ChartType::Other: element = "c:barChart"; break;
        }
        w.writeStartElement(QString::fromLatin1(element));
        switch (p.type) {
        case ChartType::Bar:
        case ChartType::Other:
            val("c:barDir", QString::fromLatin1(p.horizontal ? "bar" : "col"));
            val("c:grouping", QString::fromLatin1(p.grouping == ChartGrouping::Standard ? "clustered" : groupingName(p)));
            flag("c:varyColors", p.varyColors);
            break;
        case ChartType::Line:
        case ChartType::Area:
            val("c:grouping", QString::fromLatin1(p.grouping == ChartGrouping::Clustered ? "standard" : groupingName(p)));
            flag("c:varyColors", p.varyColors);
            break;
        case ChartType::Scatter:
            val("c:scatterStyle", QString::fromLatin1(p.lines ? "lineMarker" : "marker"));
            flag("c:varyColors", p.varyColors);
            break;
        case ChartType::Pie:
        case ChartType::Doughnut: flag("c:varyColors", true); break;
        }
        for (const ChartSeries &s : p.series) series(p, s);
        switch (p.type) {
        case ChartType::Bar:
        case ChartType::Other:
            val("c:gapWidth", QString::number(p.gapWidth));
            if (p.overlap != 0) val("c:overlap", QString::number(p.overlap));
            break;
        case ChartType::Line: flag("c:marker", p.markers); break;
        case ChartType::Pie: val("c:firstSliceAng", QString::number(p.firstSliceAngle)); break;
        case ChartType::Doughnut:
            val("c:firstSliceAng", QString::number(p.firstSliceAngle));
            val("c:holeSize", QString::number(p.holeSize));
            break;
        default: break;
        }
        if (p.type != ChartType::Pie && p.type != ChartType::Doughnut) {
            val("c:axId", QString::number(p.axisIds[0]));
            val("c:axId", QString::number(p.axisIds[1]));
        }
        w.writeEndElement();
    }

    void axis(const ChartAxis &a)
    {
        const bool value = a.kind == ChartAxis::Value;
        const bool date = a.kind == ChartAxis::Date;
        w.writeStartElement(QString::fromLatin1(value ? "c:valAx" : date ? "c:dateAx" : "c:catAx"));
        val("c:axId", QString::number(a.id));
        w.writeStartElement(QStringLiteral("c:scaling"));
        val("c:orientation", QString::fromLatin1(a.reversed ? "maxMin" : "minMax"));
        if (!a.autoMax) val("c:max", number(a.max));
        if (!a.autoMin) val("c:min", number(a.min));
        w.writeEndElement();
        flag("c:delete", a.deleted);
        val("c:axPos", a.position);
        if (a.majorGridlines) {
            w.writeStartElement(QStringLiteral("c:majorGridlines"));
            shapeProps(QColor(), false, a.gridline);
            w.writeEndElement();
        }
        if (a.minorGridlines) w.writeEmptyElement(QStringLiteral("c:minorGridlines"));
        if (a.title.present) title(a.title, true);
        w.writeEmptyElement(QStringLiteral("c:numFmt"));
        w.writeAttribute(QStringLiteral("formatCode"), clean(a.format.isEmpty() ? QStringLiteral("General") : a.format));
        w.writeAttribute(QStringLiteral("sourceLinked"), a.formatLinked ? QStringLiteral("1") : QStringLiteral("0"));
        val("c:majorTickMark", a.majorTick);
        val("c:minorTickMark", a.minorTick);
        val("c:tickLblPos", a.labelPosition);
        shapeProps(QColor(), false, a.line);
        if (hasFont(a.font) || a.hasRotation) textProps(a.font, a.hasRotation, a.labelRotation);
        val("c:crossAx", QString::number(a.crossId));
        if (a.hasCrossesAt) val("c:crossesAt", number(a.crossesAt));
        else val("c:crosses", a.crosses);
        if (value) {
            val("c:crossBetween", QString::fromLatin1(a.between ? "between" : "midCat"));
            if (a.majorUnit > 0) val("c:majorUnit", number(a.majorUnit));
            if (a.minorUnit > 0) val("c:minorUnit", number(a.minorUnit));
        } else {
            flag("c:auto", true);
            if (!date) val("c:lblAlgn", QStringLiteral("ctr"));
            val("c:lblOffset", QStringLiteral("100"));
            if (!date && a.labelSkip > 0) val("c:tickLblSkip", QString::number(a.labelSkip));
            if (!date) flag("c:noMultiLvlLbl", false);
        }
        w.writeEndElement();
    }

    void legend(const ChartLegend &l)
    {
        w.writeStartElement(QStringLiteral("c:legend"));
        val("c:legendPos", l.position);
        if (l.manual) manualPlace(l.edge, l.x, l.y, l.w, l.h);
        flag("c:overlay", l.overlay);
        shapeProps(l.fill, false, l.border);
        if (hasFont(l.font)) textProps(l.font, false, 0);
        w.writeEndElement();
    }

    void chart()
    {
        w.writeStartElement(QStringLiteral("c:chart"));
        if (c.title.present) title(c.title, false);
        flag("c:autoTitleDeleted", !c.title.present);
        w.writeStartElement(QStringLiteral("c:plotArea"));
        if (c.plotManual) manualPlace(true, c.plotX, c.plotY, c.plotW, c.plotH, c.plotInner ? "inner" : "outer");
        else w.writeEmptyElement(QStringLiteral("c:layout"));
        for (const ChartPlot &p : c.plots) plot(p);
        for (const ChartAxis &a : c.axes) axis(a);
        shapeProps(c.plotFill, false, c.plotBorder);
        w.writeEndElement();
        if (c.legend.shown) legend(c.legend);
        flag("c:plotVisOnly", c.plotVisibleOnly);
        val("c:dispBlanksAs", c.blanksAs);
        flag("c:showDLblsOverMax", false);
        w.writeEndElement();
    }
};

} // namespace

QByteArray writeChartPart(const Chart &chart)
{
    return Writer(chart).run();
}

Chart blankChart(const Theme &theme)
{
    static const QByteArray skeleton =
        "<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart><c:plotArea><c:layout/></c:plotArea></c:chart></c:chartSpace>";
    Chart chart;
    readChart(skeleton, theme, ColorMap(), &chart);
    chart.title = ChartTitle();
    chart.style = 2;
    return chart;
}

namespace {

// Elements that say how a color is made: the model keeps the color that comes of them (and writes it plainly), so they are not lost.
const QSet<QString> &colorElements()
{
    static const QSet<QString> s = {"schemeClr", "sysClr", "prstClr", "scrgbClr", "hslClr", "lumMod", "lumOff", "alpha", "alphaMod", "alphaOff", "tint", "shade", "satMod", "satOff", "hueMod", "hue", "hueOff",
                                    "sat", "lum", "red", "green", "blue", "comp", "inv", "gray", "gamma", "invGamma"};
    return s;
}

// Elements and attributes that are only how a program that wrote the part set it up (fonts for scripts the chart's text is not in, the fit of text in its box, the joins of lines, a line's
// compound kind and alignment, proofing marks): nothing a person sees of a chart is in them.
bool isHarmlessElement(const QString &name)
{
    static const QSet<QString> s = {"ea", "cs", "sym", "normAutofit", "spAutoFit", "noAutofit", "miter", "round", "bevel", "printSettings", "headerFooter", "pageMargins", "pageSetup"};
    return s.contains(name) || colorElements().contains(name);
}
bool isHarmlessAttribute(const QString &name)
{
    static const QSet<QString> s = {"cmpd", "algn", "kern", "spc", "lang", "dirty", "altLang", "noProof", "smtClean", "err", "rtl", "anchor", "anchorCtr", "spcFirstLastPara", "vertOverflow", "horzOverflow",
                                    "wrap", "lIns", "tIns", "rIns", "bIns", "numCol", "spcCol", "upright", "forceAA", "compatLnSpc", "fromWordArt", "rotWithShape", "Ignorable", "id", "pos", "pitchFamily",
                                    "charset", "panose", "uri"};
    return s.contains(name);
}

// The names of the elements of a part, each with the names of the attributes it has anywhere (those of namespace declarations left out), looking into extension lists and the markup that
// offers a part another form only for what is plainly harmless there.
struct Shape {
    QHash<QString, QSet<QString>> elements;
    QString problem;
};

void collect(const QDomElement &e, Shape *out, int depth)
{
    if (depth > 64) {
        out->problem = QStringLiteral("too deep");
        return;
    }
    const QString name = e.localName();
    if (name == QLatin1String("printSettings")) return;   // (how it is printed: not kept, not a chart's look)
    if (name == QLatin1String("extLst")) {
        // extensions: only those that hold nothing but an identifier of the chart or of a series
        for (QDomElement ext = e.firstChildElement(); !ext.isNull(); ext = ext.nextSiblingElement())
            for (QDomElement c = ext.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                if (c.localName() != QLatin1String("uniqueId")) out->problem = QStringLiteral("extLst (%1)").arg(c.localName());
        return;
    }
    QSet<QString> &attrs = out->elements[name];
    const QDomNamedNodeMap map = e.attributes();
    for (int i = 0; i < map.size(); ++i) {
        const QDomAttr a = map.item(i).toAttr();
        if (a.name().startsWith(QLatin1String("xmlns")) || a.prefix() == QLatin1String("xmlns") || a.prefix() == QLatin1String("mc")) continue;
        attrs.insert(a.localName().isEmpty() ? a.name() : a.localName());
    }
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) collect(c, out, depth + 1);
}

QStringList refsOf(const Chart &c)
{
    QStringList out;
    out << c.title.ref;
    for (const ChartAxis &a : c.axes) out << a.title.ref;
    for (const ChartPlot &p : c.plots)
        for (const ChartSeries &s : p.series) out << s.nameRef << s.categoryRef << s.valueRef << s.xRef;
    return out;
}

} // namespace

bool chartIsFullyModeled(const QByteArray &chartPartXml, QString *why)
{
    auto no = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (chartPartXml.size() > 512 * 1024) return no(QStringLiteral("a part too large to be looked through"));
    QDomDocument doc;
    QString problem;
    if (!parseXml(chartPartXml, &doc, &problem)) return no(problem);
    const Theme theme = defaultTheme();
    Chart a;
    if (!readChart(chartPartXml, theme, ColorMap(), &a, &problem)) return no(problem);
    // what is written of what was read, read again
    const QByteArray written = writeChartPart(a);
    QDomDocument out;
    if (!parseXml(written, &out, &problem)) return no(problem);
    Chart b;
    if (!readChart(written, theme, ColorMap(), &b, &problem)) return no(problem);
    // every element of the part, and every attribute an element has, is in what is written (but what only says how a program set the part up)
    Shape was, now;
    collect(doc.documentElement(), &was, 0);
    collect(out.documentElement(), &now, 0);
    if (!was.problem.isEmpty()) return no(was.problem);
    QStringList names = was.elements.keys();
    names.sort();   // (the first thing missing, in the same order every time)
    for (const QString &name : std::as_const(names)) {
        if (isHarmlessElement(name)) continue;
        if (!now.elements.contains(name)) return no(name);
        QStringList attrs = was.elements.value(name).values();
        attrs.sort();
        for (const QString &attr : std::as_const(attrs))
            if (!isHarmlessAttribute(attr) && !now.elements.value(name).contains(attr)) return no(name + QLatin1Char('@') + attr);
    }
    // a value that is only written as the usual one
    const QDomNodeList over = doc.elementsByTagNameNS(QString::fromLatin1(kCNs), QStringLiteral("showDLblsOverMax"));
    for (int i = 0; i < over.size(); ++i) {
        const QString v = over.at(i).toElement().attribute(QStringLiteral("val"), QStringLiteral("1"));
        if (v != QLatin1String("0") && v != QLatin1String("false")) return no(QStringLiteral("showDLblsOverMax"));
    }
    // and what is read of it is what was read of the part: the same references, the same picture
    if (refsOf(a) != refsOf(b)) return no(QStringLiteral("what the series read"));
    if (chartSvg(a, QSizeF(480, 288)) != chartSvg(b, QSizeF(480, 288))) return no(QStringLiteral("how the chart is drawn"));
    return true;
}

} // namespace jp::office
