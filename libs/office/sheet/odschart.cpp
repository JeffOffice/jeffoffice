// The charts of an OpenDocument spreadsheet against the shared chart model (see odschart.h). Writing makes the chart sub-document (`chart:chart` with its automatic styles, its axes, its series with the
// cells they read, and a table of the numbers they had); reading takes the same elements, and the chart-family styles by name, into the model. What the model has and the document has no element for
// (trendlines, error bars, a secondary axis) is named by the writer; what the model does not draw (stock and radar charts) is named by the reader.

#include "sheet/odschart.h"

#include "office/chart_write.h"
#include "office/xmlstream.h"
#include "sheet/cellref.h"
#include "sheet/odsformat.h"
#include "sheet/odsformula.h"
#include "sheet/odsdraw_p.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet::odschart {

using jp::office::Chart;
using jp::office::ChartAxis;
using jp::office::ChartFont;
using jp::office::ChartLine;
using jp::office::ChartPlot;
using jp::office::ChartSeries;
using jp::office::ChartType;

namespace {

constexpr double kEmuPerPoint = 12700.0;
const char kLocalTable[] = "local-table";

QString decimal(double v) { return QString::number(v, 'g', 12); }
QString hex(const QColor &c) { return QStringLiteral("#%1").arg(c.rgb() & 0xFFFFFF, 6, 16, QLatin1Char('0')); }
QString lengthPt(double pt) { return odsdraw::lengthText(qint64(std::llround(pt * kEmuPerPoint))); }

// ---- addresses

QString quoteSheet(const QString &name)
{
    static const QRegularExpression plain(QStringLiteral("^[A-Za-z_][A-Za-z0-9_.]*$"));
    if (plain.match(name).hasMatch() && !name.contains(QLatin1Char('.'))) return name;
    QString q = name;
    q.replace(QLatin1Char('\''), QStringLiteral("''"));
    return QLatin1Char('\'') + q + QLatin1Char('\'');
}

// "Sheet1!$B$2:$B$4" or "'My Data'!B2" as the standard writes a range ("Sheet1.B2:Sheet1.B4"); empty if it is not one range of one sheet.
QString odfRange(const QString &ref)
{
    const QString r = ref.trimmed();
    if (r.isEmpty() || r.contains(QLatin1Char('(')) || r.contains(QLatin1Char(',')) || r.contains(QLatin1Char('['))) return QString();
    const int bang = int(r.lastIndexOf(QLatin1Char('!')));
    if (bang < 0) return QString();
    QString sheet = r.left(bang), area = r.mid(bang + 1);
    if (sheet.startsWith(QLatin1Char('\'')) && sheet.endsWith(QLatin1Char('\'')) && sheet.size() >= 2) sheet = sheet.mid(1, sheet.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
    area.remove(QLatin1Char('$'));
    static const QRegularExpression cell(QStringLiteral("^[A-Za-z]{1,3}[0-9]+$"));
    const QStringList ends = area.split(QLatin1Char(':'));
    if (ends.isEmpty() || ends.size() > 2) return QString();
    for (const QString &e : ends)
        if (!cell.match(e).hasMatch()) return QString();
    const QString q = quoteSheet(sheet);
    return ends.size() == 1 ? q + QLatin1Char('.') + ends[0].toUpper() : q + QLatin1Char('.') + ends[0].toUpper() + QLatin1Char(':') + q + QLatin1Char('.') + ends[1].toUpper();
}

// The model's range for the standard's ("Sheet1.B2:Sheet1.B4", "$'My Data'.$B$2:.$B$4", or "Sheet1.B2"), every cell absolute, the sheet by the name it has in the workbook; empty if it is none.
QString xlsxRange(const QString &address, const QHash<QString, QString> *renames)
{
    const QString a = address.simplified().section(QLatin1Char(' '), 0, 0).isEmpty() ? QString() : address.trimmed();
    if (a.isEmpty() || a.startsWith(QLatin1String(kLocalTable))) return QString();
    // each end: an optional sheet (quoted or not, with an optional $), a dot, and a cell
    static const QRegularExpression end(QStringLiteral("^\\$?(?:'((?:[^']|'')+)'|([^.:'\\s]+))?\\.\\$?([A-Za-z]{1,3})\\$?([0-9]+)$"));
    // (split at a colon outside quotes: a sheet's name may have one)
    QStringList ends;
    {
        bool quoted = false;
        QString part;
        for (const QChar c : a) {
            if (c == QLatin1Char('\'')) quoted = !quoted;
            if (c == QLatin1Char(':') && !quoted) {
                ends << part;
                part.clear();
            } else {
                part += c;
            }
        }
        ends << part;
    }
    if (ends.isEmpty() || ends.size() > 2) return QString();
    QString sheet;
    QStringList cells;
    for (int i = 0; i < ends.size(); ++i) {
        const QRegularExpressionMatch m = end.match(ends[i]);
        if (!m.hasMatch()) return QString();
        QString name = m.captured(1).isEmpty() ? m.captured(2) : m.captured(1).replace(QStringLiteral("''"), QStringLiteral("'"));
        if (i == 0) sheet = name;
        else if (!name.isEmpty() && name != sheet) return QString();
        cells << QStringLiteral("$%1$%2").arg(m.captured(3).toUpper(), m.captured(4));
    }
    if (sheet.isEmpty()) return QString();
    if (renames) sheet = renames->value(sheet, sheet);
    return quoteSheet(sheet) + QLatin1Char('!') + cells.join(QLatin1Char(':'));
}

// "local-table.B2:local-table.B4" as the column (from 0) and the rows (from 0, inclusive); false if it is not an address in the chart's own table.
bool localRange(const QString &address, int *col, int *row1, int *row2)
{
    const QString a = address.simplified();
    if (!a.startsWith(QLatin1String(kLocalTable))) return false;
    static const QRegularExpression re(QStringLiteral("^local-table\\.\\$?([A-Za-z]{1,3})\\$?([0-9]+)(?::(?:local-table)?\\.?\\$?([A-Za-z]{1,3})\\$?([0-9]+))?$"));
    const QRegularExpressionMatch m = re.match(a);
    if (!m.hasMatch()) return false;
    auto column = [](const QString &letters) {
        int c = 0;
        for (const QChar ch : letters.toUpper()) c = c * 26 + (ch.unicode() - 'A' + 1);
        return c - 1;
    };
    *col = column(m.captured(1));
    *row1 = m.captured(2).toInt() - 1;
    *row2 = m.captured(4).isEmpty() ? *row1 : m.captured(4).toInt() - 1;
    return *col >= 0 && *row1 >= 0 && *row2 >= *row1;
}

QString columnLetters(int col)
{
    QString s;
    for (int c = col + 1; c > 0; c = (c - 1) / 26) s.prepend(QChar('A' + (c - 1) % 26));
    return s;
}

// ================================================================== writing

struct Sty {
    QString chart, graphic, text, data;
};

class Writer {
public:
    Writer(const Chart &chart, qint64 w, qint64 h, QStringList *notes) : m_c(chart), m_w(w), m_h(h), m_notes(notes) {}

    QByteArray run()
    {
        // the plots and their series, within the limits
        struct Entry {
            const ChartPlot *plot;
            const ChartSeries *series;
        };
        std::vector<Entry> all;
        bool cutSeries = false, cutPoints = false;
        qint64 total = 0;
        for (const ChartPlot &p : m_c.plots)
            for (const ChartSeries &s : p.series) {
                if (int(all.size()) >= kMaxSeries) {
                    cutSeries = true;
                    continue;
                }
                const qint64 n = s.values.size();
                if (n > kMaxPoints || total + std::min<qint64>(n, kMaxPoints) > kMaxPointsInChart) cutPoints = true;
                total += std::min<qint64>(n, kMaxPoints);
                all.push_back({&p, &s});
            }
        if (cutSeries) note(QCoreApplication::translate("Sheet", "series past the %1 a chart may have").arg(kMaxSeries));
        if (cutPoints) note(QCoreApplication::translate("Sheet", "points past the %1 a series may have").arg(kMaxPoints));
        const bool scatter = m_c.type() == ChartType::Scatter;
        noteWhatIsLost();

        // the table of numbers: the categories, then each series (a scatter's x values before its y values)
        const int rows = int(std::min<qint64>(kMaxPoints, longest(all)));
        QVector<QStringList> text;     // by column: the cells as text for the header and the categories
        QVector<QVector<double>> numbers;
        QStringList seriesXml;
        QString categoriesAddress;
        int col = 1;   // (column 0 holds the categories)
        QStringList categories;
        for (const Entry &e : all)
            if (categories.isEmpty() && !e.series->categories.isEmpty()) categories = e.series->categories.mid(0, rows);
        QVector<QPair<QString, QVector<double>>> columns;   // header and numbers, from column 1
        QString firstCategoryRef;
        for (const Entry &e : all)
            if (firstCategoryRef.isEmpty()) firstCategoryRef = odfRange(e.series->categoryRef);
        for (const Entry &e : all) {
            const ChartSeries &s = *e.series;
            QString domain;
            if (scatter) {
                QVector<double> xs = s.xValues.mid(0, rows);
                const QString ref = odfRange(s.xRef);
                const QString address = !ref.isEmpty() ? ref : localAddress(col, xs.size());
                columns.append({QString(), xs});
                domain = QStringLiteral("<chart:domain table:cell-range-address=\"%1\"/>").arg(odsdraw::xmlText(address));
                ++col;
            }
            const QVector<double> ys = s.values.mid(0, rows);
            columns.append({s.name, ys});
            const QString valueRef = odfRange(s.valueRef), nameRef = odfRange(s.nameRef);
            const QString values = !valueRef.isEmpty() ? valueRef : localAddress(col, ys.size());
            QString x = QStringLiteral("<chart:series chart:values-cell-range-address=\"%1\"").arg(odsdraw::xmlText(values));
            if (!nameRef.isEmpty() && !nameRef.contains(QLatin1Char(':'))) x += QStringLiteral(" chart:label-cell-address=\"%1\"").arg(odsdraw::xmlText(nameRef));
            else if (!s.name.isEmpty()) x += QStringLiteral(" chart:label-cell-address=\"%1\"").arg(odsdraw::xmlText(localHeader(col)));
            x += QStringLiteral(" chart:class=\"%1\" chart:style-name=\"%2\">").arg(classOf(e.plot->type), styleName(seriesStyle(s, *e.plot))) + domain;
            x += pointsOf(s);
            x += QStringLiteral("</chart:series>");
            seriesXml << x;
            ++col;
        }
        // the axes
        const ChartAxis *xAxis = nullptr, *yAxis = nullptr;
        for (const ChartAxis &a : m_c.axes) {
            if (scatter) {
                if (!xAxis) xAxis = &a;
                else if (!yAxis) yAxis = &a;
            } else if (a.kind == ChartAxis::Value) {
                if (!yAxis) yAxis = &a;
            } else if (!xAxis) {
                xAxis = &a;
            }
        }
        if (m_c.axes.size() > 2) note(QCoreApplication::translate("Sheet", "secondary axes of charts (their series are drawn on the first axes)"));
        QString axes;
        const bool noAxes = m_c.type() == ChartType::Pie || m_c.type() == ChartType::Doughnut;
        if (noAxes) {
            // (a pie has no axes to draw; the cells of its categories are named in the first one's place, as other programs write them)
            const QString cats = !firstCategoryRef.isEmpty() ? firstCategoryRef : (categories.isEmpty() ? QString() : localAddress(0, categories.size()));
            if (!cats.isEmpty()) axes += QStringLiteral("<chart:axis chart:dimension=\"x\" chart:name=\"primary-x\"><chart:categories table:cell-range-address=\"%1\"/></chart:axis>").arg(odsdraw::xmlText(cats));
        } else {
            if (xAxis) axes += axisXml(*xAxis, QStringLiteral("x"), QStringLiteral("primary-x"), scatter ? QString() : (!firstCategoryRef.isEmpty() ? firstCategoryRef : (categories.isEmpty() ? QString() : localAddress(0, categories.size()))));
            if (yAxis) axes += axisXml(*yAxis, QStringLiteral("y"), QStringLiteral("primary-y"), QString());
        }
        // the parts of the chart
        QString body;
        body += titleXml(m_c.title, "chart:title");
        body += legendXml();
        QString plot = QStringLiteral("<chart:plot-area chart:style-name=\"%1\"").arg(styleName(plotAreaStyle()));
        if (m_c.plotManual) plot += QStringLiteral(" svg:x=\"%1\" svg:y=\"%2\" svg:width=\"%3\" svg:height=\"%4\"").arg(lengthEmu(m_c.plotX * double(m_w)), lengthEmu(m_c.plotY * double(m_h)), lengthEmu(m_c.plotW * double(m_w)), lengthEmu(m_c.plotH * double(m_h)));
        plot += QStringLiteral(" chart:data-source-has-labels=\"both\">") + axes + seriesXml.join(QString());
        plot += QStringLiteral("<chart:wall chart:style-name=\"%1\"/>").arg(styleName(wallStyle())) + QStringLiteral("</chart:plot-area>");
        body += plot;
        body += tableXml(categories, columns, rows);
        const QString chartStyle = styleName(chartAreaStyle());
        QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
                                      " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\""
                                      " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\""
                                      " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" office:version=\"1.2\">");
        head += QStringLiteral("<office:automatic-styles>") + m_data + m_styles + QStringLiteral("</office:automatic-styles><office:body><office:chart>");
        head += QStringLiteral("<chart:chart svg:width=\"%1\" svg:height=\"%2\" chart:class=\"%3\" chart:style-name=\"%4\">").arg(odsdraw::lengthText(m_w), odsdraw::lengthText(m_h), classOf(m_c.type()), chartStyle);
        return (head + body + QStringLiteral("</chart:chart></office:chart></office:body></office:document-content>")).toUtf8();
    }

private:
    const Chart &m_c;
    qint64 m_w, m_h;
    QStringList *m_notes;
    QString m_styles, m_data;
    QHash<QString, QString> m_named;
    QSet<QString> m_told;
    int m_count = 0, m_dataCount = 0;

    void note(const QString &what)
    {
        if (m_notes && !m_told.contains(what)) {
            m_told.insert(what);
            *m_notes << what;
        }
    }

    void noteWhatIsLost()
    {
        for (const ChartPlot &p : m_c.plots) {
            if (p.dropLines || p.hiLowLines || p.upDownBars) note(QCoreApplication::translate("Sheet", "drop lines, high-low lines, and up-down bars of charts"));
            for (const ChartSeries &s : p.series) {
                if (!s.trendlines.isEmpty()) note(QCoreApplication::translate("Sheet", "trendlines of charts"));
                if (!s.errorBars.isEmpty()) note(QCoreApplication::translate("Sheet", "error bars of charts"));
                if (s.kept.any()) note(QCoreApplication::translate("Sheet", "gradient and pattern fills of charts (they are written as one color)"));
                if (!s.labels.kept.isEmpty()) note(QCoreApplication::translate("Sheet", "the labels of single points of charts"));
                if (s.labels.series) note(QCoreApplication::translate("Sheet", "series names in the labels of charts"));
                if (s.labels.cells) note(QCoreApplication::translate("Sheet", "labels read from cells in charts"));
            }
        }
        if (!m_c.pivotName.isEmpty()) note(QCoreApplication::translate("Sheet", "the pivot table a chart follows (the chart keeps its numbers)"));
        if (m_c.plots.size() > 1) {
            bool same = true;
            for (const ChartPlot &p : m_c.plots)
                if (p.axisIds[0] != m_c.plots[0].axisIds[0] || p.axisIds[1] != m_c.plots[0].axisIds[1]) same = false;
            if (!same) note(QCoreApplication::translate("Sheet", "secondary axes of charts (their series are drawn on the first axes)"));
        }
    }

    template <typename T>
    static qint64 longest(const T &all)
    {
        qint64 n = 0;
        for (const auto &e : all) n = std::max<qint64>({n, e.series->values.size(), e.series->categories.size(), e.series->xValues.size()});
        return n;
    }

    // ---- styles
    QString styleName(const Sty &s)
    {
        const QString key = s.chart + QLatin1Char('|') + s.graphic + QLatin1Char('|') + s.text + QLatin1Char('|') + s.data;
        const auto it = m_named.constFind(key);
        if (it != m_named.cend()) return it.value();
        const QString name = QStringLiteral("ch%1").arg(++m_count);
        m_named.insert(key, name);
        QString x = QStringLiteral("<style:style style:name=\"%1\" style:family=\"chart\"").arg(name);
        if (!s.data.isEmpty()) x += QStringLiteral(" style:data-style-name=\"%1\"").arg(s.data);
        x += QLatin1Char('>');
        if (!s.chart.isEmpty()) x += QStringLiteral("<style:chart-properties%1/>").arg(s.chart);
        if (!s.graphic.isEmpty()) x += QStringLiteral("<style:graphic-properties%1/>").arg(s.graphic);
        if (!s.text.isEmpty()) x += QStringLiteral("<style:text-properties%1/>").arg(s.text);
        m_styles += x + QStringLiteral("</style:style>");
        return name;
    }

    QString dataStyle(const QString &code)
    {
        if (code.isEmpty() || code == QLatin1String("General")) return QString();
        OdfDataStyles d;
        if (m_dataCount > 200 || !odfDataStyles(code, QStringLiteral("NC%1").arg(m_dataCount + 1), &d)) return QString();
        ++m_dataCount;
        m_data += d.xml;
        return d.mainName;
    }

    static QString fillOf(const QColor &c, bool none)
    {
        if (none || !c.isValid()) return none ? QStringLiteral(" draw:fill=\"none\"") : QString();
        return QStringLiteral(" draw:fill=\"solid\" draw:fill-color=\"%1\"").arg(hex(c));
    }

    static QString lineOf(const ChartLine &l)
    {
        if (l.none) return QStringLiteral(" draw:stroke=\"none\"");
        if (!l.set) return QString();
        QString x = QStringLiteral(" draw:stroke=\"%1\"").arg(l.dash.isEmpty() || l.dash == QLatin1String("solid") ? QStringLiteral("solid") : QStringLiteral("dash"));
        if (l.width > 0) x += QStringLiteral(" svg:stroke-width=\"%1\"").arg(lengthPt(l.width));
        if (l.color.isValid()) x += QStringLiteral(" svg:stroke-color=\"%1\"").arg(hex(l.color));
        return x;
    }

    static QString textOf(const ChartFont &f)
    {
        QString x;
        if (f.size > 0) x += QStringLiteral(" fo:font-size=\"%1pt\"").arg(decimal(f.size));
        if (f.bold >= 0) x += QStringLiteral(" fo:font-weight=\"%1\"").arg(f.bold ? QStringLiteral("bold") : QStringLiteral("normal"));
        if (f.italic >= 0) x += QStringLiteral(" fo:font-style=\"%1\"").arg(f.italic ? QStringLiteral("italic") : QStringLiteral("normal"));
        if (f.color.isValid()) x += QStringLiteral(" fo:color=\"%1\"").arg(hex(f.color));
        if (!f.family.isEmpty()) x += QStringLiteral(" fo:font-family=\"%1\"").arg(odsdraw::xmlText(f.family));
        return x;
    }

    static QString classOf(ChartType t)
    {
        switch (t) {
        case ChartType::Line: return QStringLiteral("chart:line");
        case ChartType::Pie: return QStringLiteral("chart:circle");
        case ChartType::Doughnut: return QStringLiteral("chart:ring");
        case ChartType::Area: return QStringLiteral("chart:area");
        case ChartType::Scatter: return QStringLiteral("chart:scatter");
        default: return QStringLiteral("chart:bar");
        }
    }

    Sty chartAreaStyle() const
    {
        Sty s;
        s.graphic = fillOf(m_c.background, m_c.backgroundNone) + lineOf(m_c.border);
        s.text = textOf(m_c.font);
        return s;
    }
    Sty wallStyle() const
    {
        Sty s;
        s.graphic = fillOf(m_c.plotFill, false) + lineOf(m_c.plotBorder);
        return s;
    }
    Sty plotAreaStyle()
    {
        Sty s;
        if (m_c.plots.isEmpty()) return s;
        const ChartPlot &p = m_c.plots[0];
        if (p.horizontal) s.chart += QStringLiteral(" chart:vertical=\"true\"");
        if (p.grouping == jp::office::ChartGrouping::Stacked) s.chart += QStringLiteral(" chart:stacked=\"true\"");
        if (p.grouping == jp::office::ChartGrouping::PercentStacked) s.chart += QStringLiteral(" chart:percentage=\"true\"");
        if (p.type == ChartType::Bar) s.chart += QStringLiteral(" chart:gap-width=\"%1\" chart:overlap=\"%2\"").arg(p.gapWidth).arg(p.overlap);
        if (p.type == ChartType::Pie || p.type == ChartType::Doughnut) s.chart += QStringLiteral(" chart:angle-offset=\"%1\"").arg(p.firstSliceAngle);
        if (p.type == ChartType::Line || p.type == ChartType::Scatter) {
            s.chart += QStringLiteral(" chart:symbol-type=\"%1\"").arg(p.markers ? QStringLiteral("automatic") : QStringLiteral("none"));
            if (p.type == ChartType::Scatter) s.chart += QStringLiteral(" chart:lines=\"%1\"").arg(p.lines ? QStringLiteral("true") : QStringLiteral("false"));
        }
        s.chart += QStringLiteral(" chart:include-hidden-cells=\"%1\"").arg(m_c.plotVisibleOnly ? QStringLiteral("false") : QStringLiteral("true"));
        s.chart += QStringLiteral(" chart:treat-empty-cells=\"%1\"").arg(m_c.blanksAs == QLatin1String("zero") ? QStringLiteral("use-zero") : m_c.blanksAs == QLatin1String("span") ? QStringLiteral("continue") : QStringLiteral("leave-gap"));
        return s;
    }

    static QString markerName(ChartMarker::Symbol s)
    {
        switch (s) {
        case ChartMarker::Circle: case ChartMarker::Dot: return QStringLiteral("circle");
        case ChartMarker::Diamond: return QStringLiteral("diamond");
        case ChartMarker::Square: return QStringLiteral("square");
        case ChartMarker::Triangle: return QStringLiteral("arrow-up");
        case ChartMarker::X: return QStringLiteral("x");
        case ChartMarker::Plus: return QStringLiteral("plus");
        case ChartMarker::Star: return QStringLiteral("star");
        case ChartMarker::Dash: return QStringLiteral("horizontal-bar");
        default: return QString();
        }
    }

    Sty seriesStyle(const ChartSeries &s, const ChartPlot &p)
    {
        Sty st;
        const bool lineLike = p.type == ChartType::Line || p.type == ChartType::Scatter;
        if (lineLike) {
            ChartLine l = s.outline;
            if (!l.color.isValid()) l.color = s.color;
            l.set = !l.none;
            st.graphic = lineOf(l) + (s.color.isValid() ? QStringLiteral(" draw:fill-color=\"%1\"").arg(hex(s.color)) : QString());
            if (p.type == ChartType::Scatter && !p.lines) st.graphic = QStringLiteral(" draw:stroke=\"none\"") + (s.color.isValid() ? QStringLiteral(" draw:fill-color=\"%1\"").arg(hex(s.color)) : QString());
            const bool markers = p.markers && s.marker.symbol != ChartMarker::None;
            st.chart += QStringLiteral(" chart:symbol-type=\"%1\"").arg(!markers ? QStringLiteral("none") : markerName(s.marker.symbol).isEmpty() ? QStringLiteral("automatic") : QStringLiteral("named-symbol"));
            if (markers && !markerName(s.marker.symbol).isEmpty()) st.chart += QStringLiteral(" chart:symbol-name=\"%1\"").arg(markerName(s.marker.symbol));
            if (s.smooth) st.chart += QStringLiteral(" chart:interpolation=\"cubic-spline\"");
        } else {
            st.graphic = fillOf(s.color, s.noFill && !s.color.isValid()) + lineOf(s.outline);
            if (s.noFill) st.graphic = QStringLiteral(" draw:fill=\"none\"") + lineOf(s.outline);
        }
        if (s.explosion > 0) st.chart += QStringLiteral(" chart:pie-offset=\"%1\"").arg(int(std::lround(s.explosion)));
        const ChartDataLabels &d = s.labels;
        if (d.value && d.percent) st.chart += QStringLiteral(" chart:data-label-number=\"value-and-percentage\"");
        else if (d.value) st.chart += QStringLiteral(" chart:data-label-number=\"value\"");
        else if (d.percent) st.chart += QStringLiteral(" chart:data-label-number=\"percentage\"");
        if (d.category) st.chart += QStringLiteral(" chart:data-label-text=\"true\"");
        if (d.legendKey) st.chart += QStringLiteral(" chart:data-label-symbol=\"true\"");
        if (d.shown() && !d.position.isEmpty()) {
            static const QHash<QString, QString> positions = {{"outEnd", "outside"}, {"inEnd", "inside"}, {"ctr", "center"}, {"inBase", "near-origin"}, {"t", "top"}, {"b", "bottom"}, {"l", "left"}, {"r", "right"}, {"bestFit", "avoid-overlap"}};
            if (positions.contains(d.position)) st.chart += QStringLiteral(" chart:label-position=\"%1\"").arg(positions.value(d.position));
        }
        if (d.shown()) {
            st.text = textOf(d.font);
            if (!d.format.isEmpty()) {
                st.data = dataStyle(d.format);
                if (!st.data.isEmpty()) st.chart += QStringLiteral(" chart:link-data-style-to-source=\"false\"");
            }
        }
        return st;
    }

    // The slices or bars that have a color of their own: one data-point for each, the plain ones between them as a run.
    QString pointsOf(const ChartSeries &s)
    {
        if (s.pointColors.isEmpty()) return QString();
        QString x;
        int at = 0;
        for (auto it = s.pointColors.cbegin(); it != s.pointColors.cend(); ++it) {
            if (it.key() < at || it.key() >= kMaxPoints) continue;
            if (it.key() > at) x += QStringLiteral("<chart:data-point chart:repeated=\"%1\"/>").arg(it.key() - at);
            Sty p;
            p.graphic = fillOf(it.value(), false);
            x += QStringLiteral("<chart:data-point chart:style-name=\"%1\"/>").arg(styleName(p));
            at = it.key() + 1;
        }
        return x;
    }

    QString titleXml(const ChartTitle &t, const char *element) const
    {
        if (!t.present || (t.text.isEmpty())) return QString();
        QString paragraphs;
        for (const QString &line : t.text.split(QLatin1Char('\n'))) paragraphs += QStringLiteral("<text:p>%1</text:p>").arg(odsdraw::xmlText(line.left(kChartMaxLabel * 4)));
        QString position;
        if (t.manual) position = QStringLiteral(" svg:x=\"%1\" svg:y=\"%2\"").arg(lengthEmu(t.x * double(m_w)), lengthEmu(t.y * double(m_h)));
        Sty s;
        s.text = textOf(t.font);
        return QStringLiteral("<%1%2 chart:style-name=\"%3\">%4</%1>").arg(QLatin1String(element), position, const_cast<Writer *>(this)->styleName(s), paragraphs);
    }

    QString lengthEmu(double emu) const { return odsdraw::lengthText(qint64(std::llround(emu))); }

    QString legendXml()
    {
        const ChartLegend &l = m_c.legend;
        if (!l.shown) return QString();
        static const QHash<QString, QString> positions = {{"r", "end"}, {"l", "start"}, {"t", "top"}, {"b", "bottom"}, {"tr", "end"}};
        Sty s;
        s.graphic = fillOf(l.fill, false) + lineOf(l.border);
        s.text = textOf(l.font);
        QString x = QStringLiteral("<chart:legend chart:legend-position=\"%1\" style:legend-expansion=\"high\" chart:style-name=\"%2\"").arg(positions.value(l.position, QStringLiteral("end")), styleName(s));
        if (l.manual && l.edge) x += QStringLiteral(" svg:x=\"%1\" svg:y=\"%2\"").arg(lengthEmu(l.x * double(m_w)), lengthEmu(l.y * double(m_h)));
        return x + QStringLiteral("/>");
    }

    QString axisXml(const ChartAxis &a, const QString &dimension, const QString &name, const QString &categories)
    {
        Sty s;
        if (a.logBase > 0) s.chart += QStringLiteral(" chart:logarithmic=\"true\"");
        if (!a.autoMin) s.chart += QStringLiteral(" chart:minimum=\"%1\"").arg(decimal(a.min));
        if (!a.autoMax) s.chart += QStringLiteral(" chart:maximum=\"%1\"").arg(decimal(a.max));
        if (a.majorUnit > 0) s.chart += QStringLiteral(" chart:interval-major=\"%1\"").arg(decimal(a.majorUnit));
        if (a.minorUnit > 0 && a.majorUnit > 0) s.chart += QStringLiteral(" chart:interval-minor-divisor=\"%1\"").arg(std::max(1, int(std::lround(a.majorUnit / a.minorUnit))));
        if (a.reversed) s.chart += QStringLiteral(" chart:reverse-direction=\"true\"");
        if (a.deleted) s.chart += QStringLiteral(" chart:visible=\"false\"");
        if (a.labelPosition == QLatin1String("none")) s.chart += QStringLiteral(" chart:display-label=\"false\"");
        s.chart += QStringLiteral(" chart:tick-marks-major-inner=\"%1\" chart:tick-marks-major-outer=\"%2\"")
                       .arg(a.majorTick == QLatin1String("in") || a.majorTick == QLatin1String("cross") ? QStringLiteral("true") : QStringLiteral("false"))
                       .arg(a.majorTick == QLatin1String("out") || a.majorTick == QLatin1String("cross") ? QStringLiteral("true") : QStringLiteral("false"));
        if (!a.formatLinked && !a.format.isEmpty() && a.format != QLatin1String("General")) {
            s.data = dataStyle(a.format);
            if (!s.data.isEmpty()) s.chart += QStringLiteral(" chart:link-data-style-to-source=\"false\"");
        }
        s.graphic = lineOf(a.line);
        s.text = textOf(a.font);
        QString x = QStringLiteral("<chart:axis chart:dimension=\"%1\" chart:name=\"%2\" chart:style-name=\"%3\">").arg(dimension, name, styleName(s));
        x += titleXml(a.title, "chart:title");
        if (!categories.isEmpty()) x += QStringLiteral("<chart:categories table:cell-range-address=\"%1\"/>").arg(odsdraw::xmlText(categories));
        if (a.majorGridlines) {
            Sty g;
            g.graphic = lineOf(a.gridline);
            x += QStringLiteral("<chart:grid chart:class=\"major\" chart:style-name=\"%1\"/>").arg(styleName(g));
        }
        if (a.minorGridlines) x += QStringLiteral("<chart:grid chart:class=\"minor\"/>");
        return x + QStringLiteral("</chart:axis>");
    }

    // ---- the table of the numbers
    static QString localAddress(int col, int count) { return QStringLiteral("local-table.%1%2:local-table.%1%3").arg(columnLetters(col)).arg(2).arg(1 + std::max(1, count)); }
    static QString localHeader(int col) { return QStringLiteral("local-table.%1%2").arg(columnLetters(col)).arg(1); }

    QString tableXml(const QStringList &categories, const QVector<QPair<QString, QVector<double>>> &columns, int rows) const
    {
        QString x = QStringLiteral("<table:table table:name=\"local-table\"><table:table-header-columns><table:table-column/></table:table-header-columns><table:table-columns>");
        x += QStringLiteral("<table:table-column table:number-columns-repeated=\"%1\"/>").arg(std::max<qsizetype>(1, columns.size()));
        x += QStringLiteral("</table:table-columns><table:table-header-rows><table:table-row><table:table-cell/>");
        for (const auto &c : columns) x += c.first.isEmpty() ? QStringLiteral("<table:table-cell/>") : QStringLiteral("<table:table-cell office:value-type=\"string\"><text:p>%1</text:p></table:table-cell>").arg(odsdraw::xmlText(c.first));
        x += QStringLiteral("</table:table-row></table:table-header-rows><table:table-rows>");
        for (int r = 0; r < rows; ++r) {
            x += QStringLiteral("<table:table-row>");
            x += r < categories.size() && !categories[r].isEmpty() ? QStringLiteral("<table:table-cell office:value-type=\"string\"><text:p>%1</text:p></table:table-cell>").arg(odsdraw::xmlText(categories[r])) : QStringLiteral("<table:table-cell/>");
            for (const auto &c : columns) {
                if (r < c.second.size() && std::isfinite(c.second[r])) x += QStringLiteral("<table:table-cell office:value-type=\"float\" office:value=\"%1\"/>").arg(QString::number(c.second[r], 'g', 17));
                else x += QStringLiteral("<table:table-cell/>");
            }
            x += QStringLiteral("</table:table-row>");
        }
        return x + QStringLiteral("</table:table-rows></table:table>");
    }
};


// ================================================================== reading

using odsdraw::Node;
using odsdraw::NodePtr;
using Props = ods::Props;

class Reader {
public:
    Reader(const ods::StyleStore &store, const QHash<QString, QString> *renames, QStringList *notes) : m_store(store), m_renames(renames), m_notes(notes) {}

    bool run(const NodePtr &root, Chart *out, QString *error)
    {
        Chart c = jp::office::blankChart(jp::office::defaultTheme());
        const QString cls = root->attrs.value(QStringLiteral("chart:class"));
        ChartType type = ChartType::Bar;
        QString element;
        if (!typeOf(cls, &type, &element)) {
            if (error) *error = QCoreApplication::translate("jp::office::sheet::Ods", "The chart is of a kind this program does not read.");
            return false;
        }
        qint64 width = 0, height = 0;
        odsdraw::lengthEmu(root->attrs.value(QStringLiteral("svg:width")), &width);
        odsdraw::lengthEmu(root->attrs.value(QStringLiteral("svg:height")), &height);
        // the chart's own look
        {
            const Props p = props(root->attrs.value(QStringLiteral("chart:style-name")));
            const QString fill = p.value(QStringLiteral("graphic:draw:fill"));
            QColor color;
            if (colorOf(p.value(QStringLiteral("graphic:draw:fill-color")), &color) && fill != QLatin1String("none")) c.background = color;
            else if (fill == QLatin1String("none")) c.backgroundNone = true;
            c.border = lineOf(p);
            c.font = fontOf(p);
        }
        const NodePtr plotArea = child(root, "chart:plot-area");
        const Props plotProps = plotArea ? props(plotArea->attrs.value(QStringLiteral("chart:style-name"))) : Props();
        // title and legend
        if (const NodePtr t = child(root, "chart:title")) c.title = titleOf(t, width, height);
        if (child(root, "chart:subtitle") || child(root, "chart:footer")) note(QCoreApplication::translate("Sheet", "subtitles and footers of charts"));
        if (const NodePtr l = child(root, "chart:legend")) {
            c.legend.shown = true;
            const QString pos = l->attrs.value(QStringLiteral("chart:legend-position"));
            c.legend.position = pos == QLatin1String("start") ? QStringLiteral("l") : pos == QLatin1String("top") ? QStringLiteral("t") : pos == QLatin1String("bottom") ? QStringLiteral("b") : QStringLiteral("r");
            const Props lp = props(l->attrs.value(QStringLiteral("chart:style-name")));
            QColor fill;
            if (lp.value(QStringLiteral("graphic:draw:fill")) == QLatin1String("solid") && colorOf(lp.value(QStringLiteral("graphic:draw:fill-color")), &fill)) c.legend.fill = fill;
            c.legend.border = lineOf(lp);
            c.legend.font = fontOf(lp);
            qint64 x = 0, y = 0;
            if (width > 0 && height > 0 && odsdraw::lengthEmu(l->attrs.value(QStringLiteral("svg:x")), &x) && odsdraw::lengthEmu(l->attrs.value(QStringLiteral("svg:y")), &y)) {
                c.legend.manual = true;
                c.legend.edge = true;
                c.legend.x = double(x) / double(width);
                c.legend.y = double(y) / double(height);
            }
        }
        // the plot area
        if (plotArea) {
            if (const NodePtr wall = child(plotArea, "chart:wall")) {
                const Props wp = props(wall->attrs.value(QStringLiteral("chart:style-name")));
                QColor fill;
                if (wp.value(QStringLiteral("graphic:draw:fill")) == QLatin1String("solid") && colorOf(wp.value(QStringLiteral("graphic:draw:fill-color")), &fill)) c.plotFill = fill;
                c.plotBorder = lineOf(wp);
            }
            qint64 x = 0, y = 0, w = 0, h = 0;
            if (width > 0 && height > 0 && odsdraw::lengthEmu(plotArea->attrs.value(QStringLiteral("svg:x")), &x) && odsdraw::lengthEmu(plotArea->attrs.value(QStringLiteral("svg:y")), &y)
                && odsdraw::lengthEmu(plotArea->attrs.value(QStringLiteral("svg:width")), &w) && odsdraw::lengthEmu(plotArea->attrs.value(QStringLiteral("svg:height")), &h) && w > 0 && h > 0) {
                c.plotManual = true;
                c.plotX = double(x) / double(width);
                c.plotY = double(y) / double(height);
                c.plotW = double(w) / double(width);
                c.plotH = double(h) / double(height);
            }
            const QString blanks = plotProps.value(QStringLiteral("chart:chart:treat-empty-cells"));
            c.blanksAs = blanks == QLatin1String("use-zero") ? QStringLiteral("zero") : blanks == QLatin1String("continue") ? QStringLiteral("span") : QStringLiteral("gap");
            c.plotVisibleOnly = plotProps.value(QStringLiteral("chart:chart:include-hidden-cells")) != QLatin1String("true");
        }
        readTable(root);
        // the axes (the first of each dimension; a secondary one is named)
        const NodePtr xNode = axisNode(plotArea, QStringLiteral("x")), yNode = axisNode(plotArea, QStringLiteral("y"));
        QString categoriesAddress;
        if (xNode)
            if (const NodePtr cats = child(xNode, "chart:categories")) categoriesAddress = cats->attrs.value(QStringLiteral("table:cell-range-address"));
        // the series
        const QVector<QColor> palette = c.palette.isEmpty() ? defaultPalette() : c.palette;
        QVector<ChartPlot> plots;
        QHash<int, int> plotOfType;
        int k = 0;
        qint64 total = 0;
        bool cutSeries = false, cutPoints = false;
        if (plotArea)
            for (const NodePtr &n : plotArea->children) {
                if (n->name != QLatin1String("chart:series")) continue;
                if (k >= kMaxSeries) {
                    cutSeries = true;
                    continue;
                }
                ChartType st = type;
                QString el = element;
                const QString sc = n->attrs.value(QStringLiteral("chart:class"));
                if (!sc.isEmpty()) typeOf(sc, &st, &el);
                ChartSeries s = seriesOf(n, k, st, palette, categoriesAddress, &cutPoints, &total);
                const int key = int(st);
                if (!plotOfType.contains(key)) {
                    ChartPlot p;
                    p.type = st;
                    p.element = st == ChartType::Other ? el : QString();
                    p.axisIds[0] = 111;
                    p.axisIds[1] = 222;
                    p.varyColors = st == ChartType::Pie || st == ChartType::Doughnut;
                    plotOfType.insert(key, int(plots.size()));
                    plots.append(p);
                }
                plots[plotOfType[key]].series.append(s);
                ++k;
            }
        if (cutSeries) note(QCoreApplication::translate("Sheet", "series past the %1 a chart may have").arg(kMaxSeries));
        if (cutPoints) note(QCoreApplication::translate("Sheet", "points past the %1 a series may have").arg(kMaxPoints));
        // what the plot area says of the kinds
        for (ChartPlot &p : plots) {
            if (plotProps.value(QStringLiteral("chart:chart:vertical")) == QLatin1String("true") && p.type == ChartType::Bar) p.horizontal = true;
            if (plotProps.value(QStringLiteral("chart:chart:percentage")) == QLatin1String("true")) p.grouping = jp::office::ChartGrouping::PercentStacked;
            else if (plotProps.value(QStringLiteral("chart:chart:stacked")) == QLatin1String("true")) p.grouping = jp::office::ChartGrouping::Stacked;
            else p.grouping = (p.type == ChartType::Line || p.type == ChartType::Area) ? jp::office::ChartGrouping::Standard : jp::office::ChartGrouping::Clustered;
            bool ok = false;
            const int gap = plotProps.value(QStringLiteral("chart:chart:gap-width")).toInt(&ok);
            if (ok) p.gapWidth = std::clamp(gap, 0, 500);
            const int overlap = plotProps.value(QStringLiteral("chart:chart:overlap")).toInt(&ok);
            if (ok) p.overlap = std::clamp(overlap, -100, 100);
            const int angle = plotProps.value(QStringLiteral("chart:chart:angle-offset")).toInt(&ok);
            if (ok) p.firstSliceAngle = ((angle % 360) + 360) % 360;
            p.markers = plotProps.value(QStringLiteral("chart:chart:symbol-type")) != QLatin1String("none");
            if (p.type == ChartType::Scatter) p.lines = plotProps.value(QStringLiteral("chart:chart:lines")) != QLatin1String("false");
        }
        c.plots = plots;
        // the axes of the model
        if (type != ChartType::Pie && type != ChartType::Doughnut) {
            ChartAxis ax;
            ax.id = 111;
            ax.crossId = 222;
            ChartAxis ay;
            ay.id = 222;
            ay.crossId = 111;
            ay.kind = ChartAxis::Value;
            const bool horizontal = !plots.isEmpty() && plots[0].horizontal;
            ax.kind = type == ChartType::Scatter ? ChartAxis::Value : ChartAxis::Category;
            ax.position = horizontal ? QStringLiteral("l") : QStringLiteral("b");
            ay.position = horizontal ? QStringLiteral("b") : QStringLiteral("l");
            fillAxis(&ax, xNode, width, height);
            fillAxis(&ay, yNode, width, height);
            if (!ay.majorGridlines && !yNode) ay.majorGridlines = false;
            c.axes = {ax, ay};
        }
        *out = c;
        return true;
    }

private:
    const ods::StyleStore &m_store;
    const QHash<QString, QString> *m_renames;
    QStringList *m_notes;
    QSet<QString> m_told;
    // the chart's own table: its cells by row and column (the first row is the header when the table has header rows)
    struct Cell {
        bool number = false, text = false;
        double value = 0;
        QString string;
    };
    QVector<QVector<Cell>> m_grid;
    bool m_header = false;

    void note(const QString &what)
    {
        if (m_notes && !m_told.contains(what)) {
            m_told.insert(what);
            *m_notes << what;
        }
    }

    static QVector<QColor> defaultPalette()
    {
        QVector<QColor> v;
        for (const char *c : {"#004586", "#ff420e", "#ffd320", "#579d1c", "#7e0021", "#83caff", "#314004", "#aecf00", "#4b1f6f", "#ff950e", "#c5000b", "#0084d1"}) v << QColor(QLatin1String(c));
        return v;
    }

    Props props(const QString &style) const { return style.isEmpty() ? Props() : m_store.resolve(QStringLiteral("chart"), style); }

    static NodePtr child(const NodePtr &n, const char *name)
    {
        if (!n) return nullptr;
        for (const NodePtr &c : n->children)
            if (c->name == QLatin1String(name)) return c;
        return nullptr;
    }

    static bool colorOf(const QString &text, QColor *color)
    {
        quint32 rgb = 0;
        if (!ods::colorOf(text, &rgb)) return false;
        *color = QColor::fromRgb(rgb);
        return true;
    }

    static bool typeOf(const QString &cls, ChartType *type, QString *element)
    {
        if (cls == QLatin1String("chart:bar")) *type = ChartType::Bar;
        else if (cls == QLatin1String("chart:line")) *type = ChartType::Line;
        else if (cls == QLatin1String("chart:area")) *type = ChartType::Area;
        else if (cls == QLatin1String("chart:circle")) *type = ChartType::Pie;
        else if (cls == QLatin1String("chart:ring")) *type = ChartType::Doughnut;
        else if (cls == QLatin1String("chart:scatter")) *type = ChartType::Scatter;
        else if (cls == QLatin1String("chart:radar") || cls == QLatin1String("chart:filled-radar")) {
            *type = ChartType::Other;
            *element = QStringLiteral("radarChart");
        } else if (cls == QLatin1String("chart:stock") || cls == QLatin1String("chart:bubble") || cls == QLatin1String("chart:gantt")) {
            *type = ChartType::Other;
            *element = cls == QLatin1String("chart:stock") ? QStringLiteral("stockChart") : QStringLiteral("bubbleChart");
        } else if (cls.isEmpty()) {
            *type = ChartType::Bar;
        } else {
            return false;
        }
        return true;
    }

    ChartLine lineOf(const Props &p) const
    {
        ChartLine l;
        const QString stroke = p.value(QStringLiteral("graphic:draw:stroke"));
        if (stroke.isEmpty()) return l;
        l.set = true;
        if (stroke == QLatin1String("none")) {
            l.none = true;
            return l;
        }
        QColor color;
        if (colorOf(p.value(QStringLiteral("graphic:svg:stroke-color")), &color)) l.color = color;
        qint64 w = 0;
        if (odsdraw::lengthEmu(p.value(QStringLiteral("graphic:svg:stroke-width")), &w) && w > 0) l.width = std::min(1584.0, double(w) / kEmuPerPoint);
        if (stroke == QLatin1String("dash")) {
            const QString d = p.value(QStringLiteral("graphic:draw:stroke-dash")).toLower();
            l.dash = d.contains(QLatin1String("dot")) && d.contains(QLatin1String("dash")) ? QStringLiteral("dashDot") : d.contains(QLatin1String("dot")) ? QStringLiteral("sysDot") : QStringLiteral("dash");
        }
        return l;
    }

    ChartFont fontOf(const Props &p) const
    {
        ChartFont f;
        qint64 emu = 0;
        if (odsdraw::lengthEmu(p.value(QStringLiteral("text:fo:font-size")), &emu) && emu > 0) f.size = std::min(409.0, double(emu) / kEmuPerPoint);
        const QString weight = p.value(QStringLiteral("text:fo:font-weight"));
        if (!weight.isEmpty()) f.bold = weight == QLatin1String("bold") || weight.toInt() >= 600 ? 1 : 0;
        const QString italic = p.value(QStringLiteral("text:fo:font-style"));
        if (!italic.isEmpty()) f.italic = italic == QLatin1String("italic") || italic == QLatin1String("oblique") ? 1 : 0;
        QColor color;
        if (colorOf(p.value(QStringLiteral("text:fo:color")), &color)) f.color = color;
        QString family = p.value(QStringLiteral("text:fo:font-family"));
        if (family.isEmpty()) family = p.value(QStringLiteral("text:style:font-name"));
        family.remove(QLatin1Char('\''));
        if (family.size() < 200) f.family = family;
        return f;
    }

    static QString textOfNode(const NodePtr &n, int depth = 0)
    {
        QString s;
        if (!n || depth > 8) return s;
        for (int i = 0; i <= int(n->children.size()); ++i) {
            s += n->texts.value(i);
            if (i < n->children.size()) {
                const NodePtr &c = n->children[i];
                if (c->name == QLatin1String("text:s")) s += QLatin1Char(' ');
                else if (c->name == QLatin1String("text:tab")) s += QLatin1Char('\t');
                else s += textOfNode(c, depth + 1);
            }
        }
        return s;
    }

    ChartTitle titleOf(const NodePtr &t, qint64 width, qint64 height) const
    {
        ChartTitle title;
        QStringList lines;
        for (const NodePtr &p : t->children)
            if (p->name == QLatin1String("text:p")) lines << textOfNode(p).left(kChartMaxLabel * 4);
        title.text = lines.join(QLatin1Char('\n'));
        title.present = !title.text.isEmpty();
        title.font = fontOf(props(t->attrs.value(QStringLiteral("chart:style-name"))));
        qint64 x = 0, y = 0;
        if (width > 0 && height > 0 && odsdraw::lengthEmu(t->attrs.value(QStringLiteral("svg:x")), &x) && odsdraw::lengthEmu(t->attrs.value(QStringLiteral("svg:y")), &y)) {
            title.manual = true;
            title.x = std::clamp(double(x) / double(width), 0.0, 1.0);
            title.y = std::clamp(double(y) / double(height), 0.0, 1.0);
        }
        return title;
    }

    NodePtr axisNode(const NodePtr &plotArea, const QString &dimension)
    {
        if (!plotArea) return nullptr;
        NodePtr found;
        for (const NodePtr &n : plotArea->children) {
            if (n->name != QLatin1String("chart:axis") || n->attrs.value(QStringLiteral("chart:dimension")) != dimension) continue;
            if (n->attrs.value(QStringLiteral("chart:name")).startsWith(QLatin1String("secondary"))) {
                note(QCoreApplication::translate("Sheet", "secondary axes of charts"));
                continue;
            }
            if (!found) found = n;
        }
        return found;
    }

    void fillAxis(ChartAxis *a, const NodePtr &n, qint64 width, qint64 height)
    {
        Q_UNUSED(width);
        Q_UNUSED(height);
        if (!n) {
            a->deleted = false;
            return;
        }
        const QString styleName = n->attrs.value(QStringLiteral("chart:style-name"));
        const Props p = props(styleName);
        if (p.value(QStringLiteral("chart:chart:logarithmic")) == QLatin1String("true")) a->logBase = 10;
        bool ok = false;
        double v = p.value(QStringLiteral("chart:chart:minimum")).toDouble(&ok);
        if (ok && std::isfinite(v)) {
            a->autoMin = false;
            a->min = v;
        }
        v = p.value(QStringLiteral("chart:chart:maximum")).toDouble(&ok);
        if (ok && std::isfinite(v)) {
            a->autoMax = false;
            a->max = v;
        }
        v = p.value(QStringLiteral("chart:chart:interval-major")).toDouble(&ok);
        if (ok && std::isfinite(v) && v > 0) a->majorUnit = v;
        const int divisor = p.value(QStringLiteral("chart:chart:interval-minor-divisor")).toInt(&ok);
        if (ok && divisor > 1 && a->majorUnit > 0) a->minorUnit = a->majorUnit / divisor;
        a->reversed = p.value(QStringLiteral("chart:chart:reverse-direction")) == QLatin1String("true");
        a->deleted = p.value(QStringLiteral("chart:chart:visible")) == QLatin1String("false");
        if (p.value(QStringLiteral("chart:chart:display-label")) == QLatin1String("false")) a->labelPosition = QStringLiteral("none");
        const bool inner = p.value(QStringLiteral("chart:chart:tick-marks-major-inner")) == QLatin1String("true"), outer = p.value(QStringLiteral("chart:chart:tick-marks-major-outer")) != QLatin1String("false");
        a->majorTick = inner && outer ? QStringLiteral("cross") : inner ? QStringLiteral("in") : outer ? QStringLiteral("out") : QStringLiteral("none");
        const QString dataStyle = m_store.dataStyleOf(QStringLiteral("chart"), styleName);
        if (!dataStyle.isEmpty() && p.value(QStringLiteral("chart:chart:link-data-style-to-source")) == QLatin1String("false")) {
            a->format = m_store.dataStyleCode(dataStyle);
            a->formatLinked = false;
        }
        a->line = lineOf(p);
        a->font = fontOf(p);
        if (const NodePtr t = child(n, "chart:title")) a->title = titleOf(t, 0, 0);
        for (const NodePtr &g : n->children) {
            if (g->name != QLatin1String("chart:grid")) continue;
            if (g->attrs.value(QStringLiteral("chart:class")) == QLatin1String("minor")) a->minorGridlines = true;
            else {
                a->majorGridlines = true;
                a->gridline = lineOf(props(g->attrs.value(QStringLiteral("chart:style-name"))));
            }
        }
    }

    // ---- the table
    void collectRows(const NodePtr &n, bool inHeader, int depth, int *headerRows)
    {
        if (depth > 6) return;
        for (const NodePtr &c : n->children) {
            if (c->name == QLatin1String("table:table-header-rows")) {
                collectRows(c, true, depth + 1, headerRows);
            } else if (c->name == QLatin1String("table:table-rows") || c->name == QLatin1String("table:table-row-group")) {
                collectRows(c, inHeader, depth + 1, headerRows);
            } else if (c->name == QLatin1String("table:table-row")) {
                bool ok = false;
                int repeat = c->attrs.value(QStringLiteral("table:number-rows-repeated")).toInt(&ok);
                if (!ok || repeat < 1) repeat = 1;
                QVector<Cell> row;
                for (const NodePtr &cell : c->children) {
                    if (cell->name != QLatin1String("table:table-cell") && cell->name != QLatin1String("table:covered-table-cell")) continue;
                    int cols = cell->attrs.value(QStringLiteral("table:number-columns-repeated")).toInt(&ok);
                    if (!ok || cols < 1) cols = 1;
                    Cell v;
                    const QString type = cell->attrs.value(QStringLiteral("office:value-type"));
                    if (type == QLatin1String("float") || type == QLatin1String("percentage") || type == QLatin1String("currency")) {
                        double d = cell->attrs.value(QStringLiteral("office:value")).toDouble(&ok);
                        if (ok && std::isfinite(d)) {
                            v.number = true;
                            v.value = d;
                        }
                    }
                    if (type == QLatin1String("string") || v.number) {
                        v.text = true;
                        v.string = textOfNode(cell).left(kChartMaxLabel * 4);
                    }
                    for (int i = 0; i < std::min(cols, 8) && row.size() < 2 * kMaxSeries + 4; ++i) row.append(v);
                }
                if (m_grid.size() > kMaxPoints + 1) {
                    note(QCoreApplication::translate("Sheet", "points past the %1 a series may have").arg(kMaxPoints));
                    return;
                }
                for (int i = 0; i < std::min(repeat, 4) && m_grid.size() <= kMaxPoints + 1; ++i) m_grid.append(row);
                if (inHeader) *headerRows += 1;
            }
        }
    }

    void readTable(const NodePtr &root)
    {
        const NodePtr table = child(root, "table:table");
        if (!table) return;
        int header = 0;
        collectRows(table, false, 0, &header);
        m_header = header > 0;
    }

    int bodyStart() const { return m_header ? 1 : 0; }
    Cell cellAt(int row, int col) const { return row >= 0 && row < m_grid.size() && col >= 0 && col < m_grid[row].size() ? m_grid[row][col] : Cell(); }

    // The numbers of a column of the table, between two rows (from 0).
    QVector<double> numbersOf(int col, int row1, int row2, bool *cut) const
    {
        QVector<double> v;
        for (int r = row1; r <= row2 && r < m_grid.size(); ++r) {
            if (int(v.size()) >= kMaxPoints) {
                if (cut) *cut = true;
                break;
            }
            const Cell c = cellAt(r, col);
            v.append(c.number ? c.value : std::numeric_limits<double>::quiet_NaN());
        }
        return v;
    }
    QStringList textsOf(int col, int row1, int row2) const
    {
        QStringList v;
        for (int r = row1; r <= row2 && r < m_grid.size() && int(v.size()) < kMaxPoints; ++r) v << cellAt(r, col).string;
        return v;
    }

    ChartSeries seriesOf(const NodePtr &n, int k, ChartType type, const QVector<QColor> &palette, const QString &categoriesAddress, bool *cutPoints, qint64 *total)
    {
        ChartSeries s;
        const Props p = props(n->attrs.value(QStringLiteral("chart:style-name")));
        const QString valuesAddress = n->attrs.value(QStringLiteral("chart:values-cell-range-address")), labelAddress = n->attrs.value(QStringLiteral("chart:label-cell-address"));
        int col = 0, r1 = 0, r2 = 0;
        // the numbers
        if (localRange(valuesAddress, &col, &r1, &r2)) {
            s.values = numbersOf(col, r1, r2, cutPoints);
        } else {
            s.valueRef = xlsxRange(valuesAddress, m_renames);
            col = type == ChartType::Scatter ? 2 * k + 2 : k + 1;
            s.values = numbersOf(col, bodyStart(), m_grid.size() - 1, cutPoints);
        }
        if (*total + s.values.size() > kMaxPointsInChart) {
            s.values.clear();
            *cutPoints = true;
        }
        *total += s.values.size();
        const int valuesCol = col;
        // the name
        if (localRange(labelAddress, &col, &r1, &r2)) {
            s.name = cellAt(r1, col).string;
        } else {
            s.nameRef = xlsxRange(labelAddress, m_renames);
            s.name = cellAt(0, valuesCol).string;
            if (!m_header) s.name.clear();
        }
        // the categories
        int ccol = 0, c1 = 0, c2 = 0;
        if (localRange(categoriesAddress, &ccol, &c1, &c2)) {
            s.categories = textsOf(ccol, c1, c2);
        } else {
            s.categoryRef = xlsxRange(categoriesAddress, m_renames);
            if (!m_grid.isEmpty()) s.categories = textsOf(0, bodyStart(), m_grid.size() - 1);
        }
        if (s.categories.size() > s.values.size() && !s.values.isEmpty()) s.categories = s.categories.mid(0, s.values.size());
        // a scatter's x values
        if (type == ChartType::Scatter)
            for (const NodePtr &d : n->children)
                if (d->name == QLatin1String("chart:domain")) {
                    const QString address = d->attrs.value(QStringLiteral("table:cell-range-address"));
                    int xcol = 0, x1 = 0, x2 = 0;
                    if (localRange(address, &xcol, &x1, &x2)) s.xValues = numbersOf(xcol, x1, x2, cutPoints);
                    else {
                        s.xRef = xlsxRange(address, m_renames);
                        if (!m_grid.isEmpty()) s.xValues = numbersOf(2 * k + 1, bodyStart(), m_grid.size() - 1, cutPoints);
                    }
                    break;
                }
        // how it looks
        const bool lineLike = type == ChartType::Line || type == ChartType::Scatter;
        s.color = palette.isEmpty() ? QColor(Qt::gray) : palette[k % palette.size()];
        QColor color;
        const QString fill = p.value(QStringLiteral("graphic:draw:fill"));
        ChartLine line = lineOf(p);
        if (lineLike) {
            if (line.color.isValid()) {
                s.color = line.color;
                s.colorFromFile = true;
            } else if (colorOf(p.value(QStringLiteral("graphic:draw:fill-color")), &color)) {
                s.color = color;
                s.colorFromFile = true;
            }
            line.color = QColor();
            s.outline = line;
            s.outline.color = s.color;
        } else {
            if (fill == QLatin1String("none")) s.noFill = true;
            else if (colorOf(p.value(QStringLiteral("graphic:draw:fill-color")), &color)) {
                s.color = color;
                s.colorFromFile = true;
            }
            s.outline = line;
        }
        const QString symbol = p.value(QStringLiteral("chart:chart:symbol-type"));
        if (symbol == QLatin1String("none")) s.marker.symbol = ChartMarker::None;
        else if (symbol == QLatin1String("named-symbol")) {
            static const QHash<QString, ChartMarker::Symbol> names = {{"circle", ChartMarker::Circle}, {"diamond", ChartMarker::Diamond}, {"square", ChartMarker::Square}, {"arrow-up", ChartMarker::Triangle},
                                                                      {"x", ChartMarker::X}, {"plus", ChartMarker::Plus}, {"star", ChartMarker::Star}, {"horizontal-bar", ChartMarker::Dash}};
            s.marker.symbol = names.value(p.value(QStringLiteral("chart:chart:symbol-name")), ChartMarker::Auto);
        }
        s.smooth = p.value(QStringLiteral("chart:chart:interpolation")).startsWith(QLatin1String("cubic"));
        bool ok = false;
        const int explosion = p.value(QStringLiteral("chart:chart:pie-offset")).toInt(&ok);
        if (ok) s.explosion = std::clamp(explosion, 0, 400);
        const QString number = p.value(QStringLiteral("chart:chart:data-label-number"));
        s.labels.value = number == QLatin1String("value") || number == QLatin1String("value-and-percentage");
        s.labels.percent = number == QLatin1String("percentage") || number == QLatin1String("value-and-percentage");
        s.labels.category = p.value(QStringLiteral("chart:chart:data-label-text")) == QLatin1String("true");
        s.labels.legendKey = p.value(QStringLiteral("chart:chart:data-label-symbol")) == QLatin1String("true");
        if (s.labels.shown()) {
            static const QHash<QString, QString> positions = {{"outside", "outEnd"}, {"inside", "inEnd"}, {"center", "ctr"}, {"near-origin", "inBase"}, {"top", "t"}, {"bottom", "b"}, {"left", "l"}, {"right", "r"},
                                                              {"avoid-overlap", "bestFit"}};
            s.labels.position = positions.value(p.value(QStringLiteral("chart:chart:label-position")));
            s.labels.font = fontOf(p);
            const QString ds = m_store.dataStyleOf(QStringLiteral("chart"), n->attrs.value(QStringLiteral("chart:style-name")));
            if (!ds.isEmpty() && p.value(QStringLiteral("chart:chart:link-data-style-to-source")) == QLatin1String("false")) s.labels.format = m_store.dataStyleCode(ds);
        }
        // the points that have a color of their own
        int at = 0;
        for (const NodePtr &d : n->children) {
            if (d->name != QLatin1String("chart:data-point")) continue;
            int repeat = d->attrs.value(QStringLiteral("chart:repeated")).toInt(&ok);
            if (!ok || repeat < 1) repeat = 1;
            const QString sn = d->attrs.value(QStringLiteral("chart:style-name"));
            if (!sn.isEmpty()) {
                const Props pp = props(sn);
                QColor pc;
                if (colorOf(pp.value(QStringLiteral("graphic:draw:fill-color")), &pc))
                    for (int i = 0; i < std::min(repeat, 64) && at + i < kMaxPoints; ++i) s.pointColors.insert(at + i, pc);
            }
            at += repeat;
            if (at >= kMaxPoints) break;
        }
        return s;
    }
};

} // namespace

QByteArray chartToOdf(const jp::office::Chart &chart, qint64 widthEmu, qint64 heightEmu, QStringList *notes)
{
    Writer w(chart, std::max<qint64>(widthEmu, 1), std::max<qint64>(heightEmu, 1), notes);
    return w.run();
}

bool chartFromOdf(const QByteArray &content, const ods::StyleStore &store, const QHash<QString, QString> *renames, jp::office::Chart *out, QString *error, QStringList *notes)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    if (content.size() > kMaxPartBytes) return fail(QCoreApplication::translate("jp::office::sheet::Ods", "The chart is larger than this program reads."));
    QBuffer buffer;
    buffer.setData(content);
    buffer.open(QIODevice::ReadOnly);
    jp::office::XmlGuardDevice guard(&buffer, qint64(64) << 20);
    guard.open(QIODevice::ReadOnly);
    QXmlStreamReader xml(&guard);
    odsdraw::Counters counters;
    odsdraw::NodePtr root;
    int depth = 0;
    while (!xml.atEnd()) {
        const QXmlStreamReader::TokenType t = xml.readNext();
        if (t == QXmlStreamReader::StartElement) {
            if (xml.namespaceUri() == QLatin1String("urn:oasis:names:tc:opendocument:xmlns:chart:1.0") && xml.name() == QLatin1String("chart")) {
                if (!odsdraw::readNode(xml, 100, &counters, &root)) return fail(QCoreApplication::translate("jp::office::sheet::Ods", "The chart is damaged."));
                break;
            }
            if (++depth > 20) return fail(QCoreApplication::translate("jp::office::sheet::Ods", "The chart is damaged."));
        } else if (t == QXmlStreamReader::EndElement) {
            --depth;
        } else if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference || t == QXmlStreamReader::Invalid) {
            return fail(QCoreApplication::translate("jp::office::sheet::Ods", "The chart is damaged."));
        }
    }
    if (!root) return fail(xml.hasError() || guard.refused() ? QCoreApplication::translate("jp::office::sheet::Ods", "The chart is damaged.") : QCoreApplication::translate("jp::office::sheet::Ods", "The file holds no chart there."));
    if (counters.over && notes) *notes << QCoreApplication::translate("jp::office::sheet::Ods", "parts of a chart past the limits on drawings");
    Reader reader(store, renames, notes);
    return reader.run(root, out, error);
}

} // namespace jp::office::sheet::odschart
