#include "sheet/xlschart.h"

#include "office/chart_write.h"
#include "office/theme.h"
#include "sheet/xlsbytes.h"

#include <QColor>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::office::sheet {

namespace {

constexpr qint64 kEmuPerDefaultColumn = 609600, kEmuPerDefaultRow = 190500;   // (64 and 20 pixels: the sizes the parts of cells in a drawing record are taken of)
constexpr int kMaxBlockDepth = 64;
constexpr int kMaxGroups = 64;

struct RawSeries {
    QString name, nameRef, valueRef, categoryRef;
    QVector<double> values;
    QStringList categories;
    bool valuesLiteral = false, categoriesLiteral = false;
    int group = 0;
    bool showValue = false, showPercent = false, showCategory = false, showSeries = false;
    QColor fill, line;
    bool lineNone = false, lineSet = false, smooth = false;
    int marker = -1;
};

struct RawGroup {
    quint16 kind = 0;    // the record that says what is drawn (0x1017 bar, ...), 0 for none yet
    int parent = 0;
    bool varied = false;
    bool is3d = false, bubbles = false, dropLines = false, hiLowLines = false;
    int overlap = 0, gap = 150, grbit = 0, start = 0, donut = 0;
};

struct RawAxis {
    bool present = false, hasRange = false, hasScale = false;
    double lo = 0, hi = 0, major = 0, minor = 0;
    int flags = 0x1F;
    int majorTick = -1, minorTick = -1, labels = -1;
    bool majorGrid = false, minorGrid = false;
    bool between = true, reverse = false;
    QString title;
    bool hasTitle = false;
};

QString shortString(Bytes &b)
{
    const int count = b.u8();
    const int flags = b.u8();
    if (b.bad()) return QString();
    const bool high = flags & 1;
    const qint64 bytes = qint64(count) * (high ? 2 : 1);
    if (bytes > b.left()) return QString();
    const char *p = b.take(bytes);
    if (!p) return QString();
    QString s = high ? QString::fromUtf16(reinterpret_cast<const char16_t *>(p), count) : QString::fromLatin1(p, count);
    return s.left(kChartMaxLabel);
}

QColor colorOf(quint32 rgb) { return QColor(int(rgb & 0xFF), int((rgb >> 8) & 0xFF), int((rgb >> 16) & 0xFF)); }

// The values or words of an array constant as a formula is written ("{1,2;3,4}"): at most kMaxXlsChartPoints.
QStringList arrayItems(const QString &text)
{
    QString inner = text.mid(1);
    if (inner.endsWith(QLatin1Char('}'))) inner.chop(1);
    QStringList out;
    QString cur;
    bool quoted = false;
    for (int i = 0; i < inner.size() && out.size() < kMaxXlsChartPoints; ++i) {
        const QChar ch = inner[i];
        if (ch == QLatin1Char('"')) {
            if (quoted && i + 1 < inner.size() && inner[i + 1] == QLatin1Char('"')) {
                cur += ch;
                ++i;
            } else {
                quoted = !quoted;
            }
        } else if (!quoted && (ch == QLatin1Char(',') || ch == QLatin1Char(';'))) {
            out << cur;
            cur.clear();
        } else {
            cur += ch;
        }
    }
    if (out.size() < kMaxXlsChartPoints && (!cur.isEmpty() || !out.isEmpty())) out << cur;
    return out;
}

ChartMarker::Symbol markerOf(int imk)
{
    switch (imk) {
    case 1: return ChartMarker::Square;
    case 2: return ChartMarker::Diamond;
    case 3: return ChartMarker::Triangle;
    case 4: return ChartMarker::X;
    case 5: return ChartMarker::Star;
    case 6: return ChartMarker::Dot;
    case 7: return ChartMarker::Dash;
    case 8: return ChartMarker::Circle;
    case 9: return ChartMarker::Plus;
    default: return ChartMarker::None;
    }
}

ChartAxis makeAxis(ChartAxis::Kind kind, qint64 id, qint64 cross, const QString &position, const RawAxis &raw)
{
    ChartAxis a;
    a.kind = kind;
    a.id = id;
    a.crossId = cross;
    a.position = position;
    if (raw.hasTitle) {
        a.title.present = true;
        a.title.text = raw.title;
    }
    a.majorGridlines = raw.majorGrid;
    a.minorGridlines = raw.minorGrid;
    switch (raw.labels) {
    case 0: a.labelPosition = QStringLiteral("none"); break;
    case 1: a.labelPosition = QStringLiteral("low"); break;
    case 2: a.labelPosition = QStringLiteral("high"); break;
    default: break;
    }
    auto tick = [](int v, const QString &usual) {
        switch (v) {
        case 0: return QStringLiteral("none");
        case 1: return QStringLiteral("in");
        case 2: return QStringLiteral("out");
        case 3: return QStringLiteral("cross");
        default: return usual;
        }
    };
    a.majorTick = tick(raw.majorTick, a.majorTick);
    a.minorTick = tick(raw.minorTick, a.minorTick);
    if (raw.hasRange) {
        a.autoMin = raw.flags & 1;
        a.autoMax = raw.flags & 2;
        if (!a.autoMin && std::isfinite(raw.lo)) a.min = raw.lo; else a.autoMin = true;
        if (!a.autoMax && std::isfinite(raw.hi)) a.max = raw.hi; else a.autoMax = true;
        if (!(raw.flags & 4) && std::isfinite(raw.major) && raw.major > 0) a.majorUnit = raw.major;
        if (!(raw.flags & 8) && std::isfinite(raw.minor) && raw.minor > 0) a.minorUnit = raw.minor;
        if (raw.flags & 0x20) a.logBase = 10;
        if (raw.flags & 0x40) a.reversed = true;
        if (raw.flags & 0x80) a.crosses = QStringLiteral("max");
    }
    if (raw.reverse) a.reversed = true;
    return a;
}

} // namespace

bool readXlsAnchor(const QByteArray &data, qsizetype from, qsizetype to, XlsAnchor *out)
{
    // The atoms of the drawing format: a header of 8 bytes (version and instance, kind, length); a container's atoms follow its header at once, any other is skipped by its length.
    Bytes b(data, from, to);
    bool found = false;
    qint64 steps = (to - from) / 8 + 1;
    while (b.left() >= 8 && steps-- > 0) {
        const quint16 verInst = b.u16();
        const quint16 type = b.u16();
        const quint32 len = b.u32();
        if ((verInst & 0xF) == 0xF) continue;
        if (type == 0xF010 && len >= 18 && b.left() >= 18) {
            b.u16();
            XlsAnchor a;
            const quint32 col1 = b.u16(), dx1 = b.u16(), row1 = b.u16(), dy1 = b.u16(), col2 = b.u16(), dx2 = b.u16(), row2 = b.u16(), dy2 = b.u16();
            a.col1 = col1;
            a.row1 = row1;
            a.col2 = col2;
            a.row2 = row2;
            a.colOff1 = qint64(std::min<quint32>(dx1, 1023)) * kEmuPerDefaultColumn / 1024;
            a.rowOff1 = qint64(std::min<quint32>(dy1, 255)) * kEmuPerDefaultRow / 256;
            a.colOff2 = qint64(std::min<quint32>(dx2, 1023)) * kEmuPerDefaultColumn / 1024;
            a.rowOff2 = qint64(std::min<quint32>(dy2, 255)) * kEmuPerDefaultRow / 256;
            *out = a;
            found = true;
            b.skip(len - 18);
            continue;
        }
        if (len > quint32(b.left())) break;
        b.skip(len);
    }
    return found;
}

XlsChartRead readXlsChart(const QByteArray &d, qsizetype from, qsizetype to, const XlsContext &cx, const QString &ownSheet)
{
    XlsChartRead res;
    QVector<RawSeries> series;
    QVector<RawGroup> groups;
    RawAxis axes[2][3];
    QVector<quint16> stack;
    int overflow = 0;
    quint16 prev = 0;
    int curSeries = -1, curGroup = -1, curAxis = -1, curParent = 0;
    bool parentOpen = false, inFormat = false, inText = false, textDefault = false, textHas = false, seriesTooMany = false;
    int textLink = -1, lastAi = -1;
    QString textString;
    bool legendShown = false;
    QString legendPosition = QStringLiteral("r");
    bool titleSet = false, titleAuto = false;
    QString titleText;

    auto reference = [&](const QString &text) {
        if (text.contains(QLatin1String("#REF"))) return QString();
        const int colon = text.lastIndexOf(QLatin1Char(':'));
        if (colon > 0 && text.left(colon).endsWith(text.mid(colon + 1)) && text.left(colon).size() > text.size() - colon - 1) {
            // (a single cell is stored as an area of one cell)
            const QString cell = text.left(colon);
            const QString tail = cell.mid(cell.size() - (text.size() - colon - 1) - 1);
            if (tail.startsWith(QLatin1Char('!')) || tail.startsWith(QLatin1Char(':'))) return cell.contains(QLatin1Char('!')) ? cell : xlsSheetPrefix(ownSheet) + QLatin1Char('!') + cell;
        }
        return text.contains(QLatin1Char('!')) ? text : xlsSheetPrefix(ownSheet) + QLatin1Char('!') + text;
    };
    auto finishText = [&]() {
        if (textDefault) return;
        const int p = parentOpen ? curParent : 0;
        switch (textLink) {
        case 1:
            titleSet = true;
            titleAuto = !textHas;
            titleText = textString;
            break;
        case 2:
            if (textHas) {
                axes[p][1].hasTitle = true;
                axes[p][1].title = textString;
            }
            break;
        case 3:
            if (textHas) {
                axes[p][0].hasTitle = true;
                axes[p][0].title = textString;
            }
            break;
        default: break;
        }
    };

    qsizetype pos = from;
    const qsizetype stop = std::min<qsizetype>(to, d.size());
    qint64 guard = qint64(stop - from) / 4 + 1;
    while (pos >= 0 && pos + 4 <= stop && guard-- > 0) {
        Bytes head(d, pos, pos + 4);
        const quint16 id = head.u16();
        const qsizetype len = head.u16();
        const qsizetype at = pos + 4;
        const qsizetype last = std::min<qsizetype>(at + len, stop);
        pos = at + len;
        Bytes b(d, at, last);
        if (id == 0x1033) {
            if (stack.size() < kMaxBlockDepth) stack.append(prev);
            else ++overflow;
            continue;
        }
        if (id == 0x1034) {
            if (overflow > 0) {
                --overflow;
            } else if (!stack.isEmpty()) {
                const quint16 kind = stack.takeLast();
                switch (kind) {
                case 0x1003: curSeries = -1; break;
                case 0x1006: inFormat = false; break;
                case 0x1025:
                    if (inText) finishText();
                    inText = false;
                    break;
                case 0x1041: parentOpen = false; break;
                case 0x101D: curAxis = -1; break;
                case 0x1014: curGroup = -1; break;
                default: break;
                }
            }
            continue;
        }
        if (id == 0x003C || id == 0x0809 || id == 0x000A) continue;
        const quint16 before = prev;
        prev = id;
        switch (id) {
        case 0x1003: {   // Series
            curSeries = -1;
            if (series.size() >= kMaxXlsChartSeries) {
                seriesTooMany = true;
                break;
            }
            series.append(RawSeries());
            curSeries = int(series.size()) - 1;
            lastAi = -1;
            break;
        }
        case 0x1051: {   // the source of a text or of numbers: a literal or cells
            const int what = b.u8();
            const int kind = b.u8();
            b.u16();
            b.u16();
            const int cce = b.u16();
            if (b.bad() || cce > b.left()) break;
            lastAi = what;
            if (kind != 2 || cce == 0) break;
            const QByteArray rgce(b.take(cce), cce);
            const qsizetype extraLength = b.left();
            const QByteArray extra = extraLength > 0 ? QByteArray(b.take(extraLength), extraLength) : QByteArray();
            XlsFormulaOptions opts;
            opts.maxChars = 4000;
            const XlsFormulaResult text = xlsFormulaText(cx, rgce, extra, opts);
            if (!text.ok() || text.text.isEmpty()) break;
            if (inText) {
                break;
            }
            if (curSeries < 0) break;
            RawSeries &s = series[curSeries];
            const bool literal = text.text.startsWith(QLatin1Char('{'));
            if (what == 0) {
                if (!literal) s.nameRef = reference(text.text);
            } else if (what == 1) {
                if (literal) {
                    s.valuesLiteral = true;
                    for (const QString &item : arrayItems(text.text)) {
                        bool ok = false;
                        const double v = item.trimmed().toDouble(&ok);
                        s.values << (ok ? v : std::numeric_limits<double>::quiet_NaN());
                    }
                } else {
                    s.valueRef = reference(text.text);
                }
            } else if (what == 2) {
                if (literal) {
                    s.categoriesLiteral = true;
                    s.categories = arrayItems(text.text);
                } else {
                    s.categoryRef = reference(text.text);
                }
            }
            break;
        }
        case 0x100D: {   // SeriesText: the words of a name or a title
            b.u16();
            const QString s = shortString(b);
            if (b.bad()) break;
            if (inText) {
                textString = s;
                textHas = true;
            } else if (curSeries >= 0 && lastAi == 0) {
                series[curSeries].name = s;
            }
            break;
        }
        case 0x1025:   // Text
            inText = true;
            textDefault = before == 0x1024;
            textLink = -1;
            textHas = false;
            textString.clear();
            break;
        case 0x1027:   // ObjectLink: what a text is the title of
            if (inText) textLink = b.u16();
            break;
        case 0x1015: {   // Legend
            b.skip(16);
            const int type = b.u8();
            if (b.bad()) break;
            legendShown = true;
            legendPosition = type == 0 ? QStringLiteral("b") : type == 1 ? QStringLiteral("tr") : type == 2 ? QStringLiteral("t") : type == 4 ? QStringLiteral("l") : QStringLiteral("r");
            break;
        }
        case 0x1041: {   // AxisParent
            const int iax = b.u16();
            if (b.bad()) break;
            curParent = iax == 1 ? 1 : 0;
            parentOpen = true;
            break;
        }
        case 0x101D: {   // Axis
            const int type = b.u16();
            if (b.bad() || type > 2) break;
            curAxis = type;
            axes[curParent][type].present = true;
            break;
        }
        case 0x101F:   // ValueRange
            if (curAxis >= 0) {
                RawAxis &a = axes[curParent][curAxis];
                a.lo = b.f64();
                a.hi = b.f64();
                a.major = b.f64();
                a.minor = b.f64();
                b.f64();
                const int flags = b.u16();
                if (!b.bad()) {
                    a.flags = flags;
                    a.hasRange = true;
                }
            }
            break;
        case 0x1020:   // CatSerRange
            if (curAxis >= 0) {
                b.skip(6);
                const int flags = b.u16();
                if (!b.bad()) {
                    axes[curParent][curAxis].between = flags & 1;
                    axes[curParent][curAxis].reverse = flags & 4;
                }
            }
            break;
        case 0x101E:   // Tick
            if (curAxis >= 0) {
                const int major = b.u8(), minor = b.u8(), labels = b.u8();
                if (!b.bad()) {
                    axes[curParent][curAxis].majorTick = major;
                    axes[curParent][curAxis].minorTick = minor;
                    axes[curParent][curAxis].labels = labels;
                }
            }
            break;
        case 0x1021: {   // AxisLineFormat
            const int what = b.u16();
            if (curAxis >= 0 && !b.bad()) {
                if (what == 1) axes[curParent][curAxis].majorGrid = true;
                else if (what == 2) axes[curParent][curAxis].minorGrid = true;
            }
            break;
        }
        case 0x1014: {   // ChartFormat: a group of series drawn the same way
            b.skip(16);
            const int flags = b.u16();
            curGroup = -1;
            if (b.bad()) break;
            if (groups.size() >= kMaxGroups) {
                res.lost = true;
                break;
            }
            RawGroup g;
            g.parent = curParent;
            g.varied = flags & 1;
            groups.append(g);
            curGroup = int(groups.size()) - 1;
            break;
        }
        case 0x1017:   // Bar
        case 0x1018:   // Line
        case 0x1019:   // Pie
        case 0x101A:   // Area
        case 0x101B:   // Scatter
        case 0x103E:   // Radar
        case 0x103F:   // Surface
        case 0x1040:   // filled Radar
        case 0x1061:   // bar or pie of pie
            if (curGroup >= 0 && groups[curGroup].kind == 0) {
                RawGroup &g = groups[curGroup];
                g.kind = id;
                if (id == 0x1017) {
                    g.overlap = qint16(b.u16());
                    g.gap = b.u16();
                    g.grbit = b.u16();
                } else if (id == 0x1019) {
                    g.start = b.u16();
                    g.donut = b.u16();
                } else if (id == 0x101B) {
                    b.skip(4);
                    g.bubbles = b.u16() & 1;
                } else {
                    g.grbit = b.u16();
                }
            }
            break;
        case 0x103A:   // Chart3d: the model has no depth
            if (curGroup >= 0) groups[curGroup].is3d = true;
            break;
        case 0x101C: {   // ChartLine: the lines a chart group draws between its series' points
            const int what = b.u16();
            if (curGroup >= 0 && !b.bad()) {
                if (what == 0) groups[curGroup].dropLines = true;
                else if (what == 1) groups[curGroup].hiLowLines = true;
            }
            break;
        }
        case 0x1045: {   // SerToCrt: the group a series is in
            const int g = b.u16();
            if (curSeries >= 0 && !b.bad()) series[curSeries].group = g;
            break;
        }
        case 0x1006: {   // DataFormat: the look of a series, or of a point of it
            const int point = b.u16();
            inFormat = curSeries >= 0 && !b.bad() && point == 0xFFFF;
            break;
        }
        case 0x100A:   // AreaFormat
            if (inFormat) {
                const quint32 fore = b.u32();
                b.u32();
                const int fills = b.u16();
                const int flags = b.u16();
                if (!b.bad() && fills == 1 && !(flags & 1)) series[curSeries].fill = colorOf(fore);
            }
            break;
        case 0x1007:   // LineFormat
            if (inFormat) {
                const quint32 rgb = b.u32();
                const int style = b.u16();
                b.u16();
                const int flags = b.u16();
                if (!b.bad()) {
                    RawSeries &s = series[curSeries];
                    s.lineSet = true;
                    s.lineNone = style == 5;
                    if (!(flags & 1) && style != 5) s.line = colorOf(rgb);
                }
            }
            break;
        case 0x1009:   // MarkerFormat
            if (inFormat) {
                b.skip(8);
                const int imk = b.u16();
                if (!b.bad()) series[curSeries].marker = imk;
            }
            break;
        case 0x100C:   // AttachedLabel: what the labels of a series show
            if (inFormat) {
                const int flags = b.u16();
                if (!b.bad()) {
                    RawSeries &s = series[curSeries];
                    s.showValue = flags & 1;
                    s.showPercent = flags & 6;
                    s.showCategory = flags & 0x14;
                    s.showSeries = flags & 0x40;
                }
            }
            break;
        case 0x105D:   // SerFmt
            if (inFormat) {
                const int flags = b.u16();
                if (!b.bad()) series[curSeries].smooth = flags & 1;
            }
            break;
        default: break;
        }
    }

    // ---- the model
    Chart c = blankChart(defaultTheme());
    if (seriesTooMany) res.lost = true;
    bool any3d = false;
    QVector<int> planOf(groups.size(), -1);
    for (int gi = 0; gi < groups.size(); ++gi) {
        const RawGroup &g = groups[gi];
        const bool known = g.kind == 0x1017 || g.kind == 0x1018 || g.kind == 0x1019 || g.kind == 0x101A || (g.kind == 0x101B && !g.bubbles);
        if (!known) {
            res.lost = true;
            continue;
        }
        if (g.is3d) any3d = true;
        ChartPlot p;
        switch (g.kind) {
        case 0x1017:
            p.type = ChartType::Bar;
            p.horizontal = g.grbit & 1;
            p.grouping = (g.grbit & 4) ? ChartGrouping::PercentStacked : (g.grbit & 2) ? ChartGrouping::Stacked : ChartGrouping::Clustered;
            p.overlap = p.grouping == ChartGrouping::Clustered ? std::clamp(g.overlap, -100, 100) : 100;
            p.gapWidth = std::clamp(g.gap, 0, 500);
            break;
        case 0x1018:
        case 0x101A:
            p.type = g.kind == 0x1018 ? ChartType::Line : ChartType::Area;
            p.grouping = (g.grbit & 2) ? ChartGrouping::PercentStacked : (g.grbit & 1) ? ChartGrouping::Stacked : ChartGrouping::Standard;
            p.dropLines = g.dropLines;
            p.hiLowLines = g.hiLowLines;
            break;
        case 0x1019:
            p.type = g.donut > 0 ? ChartType::Doughnut : ChartType::Pie;
            p.grouping = ChartGrouping::Standard;
            p.firstSliceAngle = std::clamp(g.start, 0, 360);
            if (g.donut > 0) p.holeSize = std::clamp(g.donut, 10, 90);
            break;
        default:
            p.type = ChartType::Scatter;
            p.grouping = ChartGrouping::Standard;
            break;
        }
        p.varyColors = g.varied;
        planOf[gi] = int(c.plots.size());
        c.plots.append(p);
    }
    // the series, each in its group
    QVector<bool> markerSeen(c.plots.size(), false), markerOn(c.plots.size(), false), lineOn(c.plots.size(), false), smoothOn(c.plots.size(), false);
    for (const RawSeries &r : series) {
        const int gi = r.group >= 0 && r.group < groups.size() ? r.group : 0;
        const int pi = gi < planOf.size() ? planOf[gi] : -1;
        if (pi < 0) {
            res.lost = true;
            continue;
        }
        ChartPlot &p = c.plots[pi];
        ChartSeries s;
        s.name = r.name.left(kChartMaxLabel);
        s.nameRef = r.nameRef;
        s.valueRef = r.valueRef;
        s.values = r.values;
        if (p.type == ChartType::Scatter) {
            s.xRef = r.categoryRef;
            for (const QString &t : r.categories) {
                bool ok = false;
                const double v = t.trimmed().toDouble(&ok);
                s.xValues << (ok ? v : std::numeric_limits<double>::quiet_NaN());
            }
        } else {
            s.categoryRef = r.categoryRef;
            for (const QString &t : r.categories) s.categories << t.left(kChartMaxLabel);
        }
        const bool lineLike = p.type == ChartType::Line || p.type == ChartType::Scatter;
        const QColor color = lineLike ? r.line : r.fill;
        if (color.isValid() && p.type != ChartType::Pie && p.type != ChartType::Doughnut) {
            s.color = color;
            s.colorFromFile = true;
        }
        s.smooth = r.smooth;
        if (r.marker >= 0) {
            s.marker.symbol = markerOf(r.marker);
            markerSeen[pi] = true;
            if (r.marker != 0) markerOn[pi] = true;
        }
        if (r.lineSet && !r.lineNone) lineOn[pi] = true;
        if (r.smooth) smoothOn[pi] = true;
        s.labels.value = r.showValue;
        s.labels.percent = r.showPercent;
        s.labels.category = r.showCategory;
        s.labels.series = r.showSeries;
        p.series.append(s);
    }
    for (int pi = 0; pi < c.plots.size(); ++pi) {
        ChartPlot &p = c.plots[pi];
        if (p.type == ChartType::Line) p.markers = !markerSeen[pi] || markerOn[pi];
        if (p.type == ChartType::Scatter) {
            p.lines = lineOn[pi];
            p.smooth = smoothOn[pi];
        }
    }
    // plots with no series have nothing to draw
    {
        QVector<ChartPlot> kept;
        QVector<int> parentOf;
        for (int gi = 0; gi < groups.size(); ++gi)
            if (planOf[gi] >= 0 && !c.plots[planOf[gi]].series.isEmpty()) {
                kept.append(c.plots[planOf[gi]]);
                parentOf.append(groups[gi].parent);
            }
        c.plots = kept;
        // the axes: a pair for each side that has a plot that is drawn against axes
        bool sideUsed[2] = {false, false}, horizontal = false, scatterOnly[2] = {true, true};
        for (int i = 0; i < c.plots.size(); ++i) {
            const bool round = c.plots[i].type == ChartType::Pie || c.plots[i].type == ChartType::Doughnut;
            const int side = parentOf[i];
            if (round) continue;
            if (!sideUsed[side] && side == 0) horizontal = c.plots[i].horizontal;
            sideUsed[side] = true;
            if (c.plots[i].type != ChartType::Scatter) scatterOnly[side] = false;
            c.plots[i].axisIds[0] = side ? 1003 : 1001;
            c.plots[i].axisIds[1] = side ? 1004 : 1002;
        }
        for (int side = 0; side < 2; ++side) {
            if (!sideUsed[side]) continue;
            const qint64 catId = side ? 1003 : 1001, valId = side ? 1004 : 1002;
            ChartAxis cat = makeAxis(scatterOnly[side] ? ChartAxis::Value : ChartAxis::Category, catId, valId, horizontal ? QStringLiteral("l") : QStringLiteral("b"), axes[side][0]);
            ChartAxis val = makeAxis(ChartAxis::Value, valId, catId, horizontal ? QStringLiteral("b") : QStringLiteral("l"), axes[side][1]);
            val.between = axes[side][0].between;
            if (scatterOnly[side]) cat.between = false;
            if (side) {
                cat.deleted = true;
                cat.position = horizontal ? QStringLiteral("r") : QStringLiteral("t");
                val.position = horizontal ? QStringLiteral("t") : QStringLiteral("r");
                val.crosses = QStringLiteral("max");
            }
            c.axes << cat << val;
        }
    }
    if (titleSet) {
        c.title.present = true;
        c.title.automatic = titleAuto;
        c.title.text = titleAuto ? QString() : titleText;
    }
    c.legend.shown = legendShown;
    c.legend.position = legendPosition;
    if (any3d) res.lost = true;
    res.chart = c;
    res.read = !c.plots.isEmpty();
    return res;
}

} // namespace jp::office::sheet
