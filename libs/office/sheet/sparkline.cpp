#include "sheet/sparkline.h"

#include "sheet/xlsxbase.h"

#include <QXmlStreamAttributes>

#include <cmath>

namespace jp::office::sheet {

namespace {

const char *kX14Ns = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";
const char *kXmNs = "http://schemas.microsoft.com/office/excel/2006/main";

bool truth(QStringView v) { return v == QLatin1String("1") || v.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0; }

double number(QStringView v, double fallback)
{
    bool ok = false;
    const double d = v.trimmed().toDouble(&ok);
    return ok && std::isfinite(d) ? d : fallback;
}

ColorSpec colorFrom(const QXmlStreamReader &r)
{
    ColorSpec c;
    double tint = 0;
    for (const QXmlStreamAttribute &a : r.attributes()) {
        if (!a.namespaceUri().isEmpty()) continue;
        bool ok = false;
        const QStringView name = a.name();
        if (name == QLatin1String("rgb")) {
            const quint32 v = a.value().trimmed().toUInt(&ok, 16);
            if (ok) {
                c.kind = ColorSpec::Kind::Rgb;
                c.rgb = v & 0xFFFFFF;
            }
        } else if (name == QLatin1String("theme")) {
            const int v = a.value().toInt(&ok);
            if (ok && v >= 0 && v < 12) {
                c.kind = ColorSpec::Kind::Theme;
                c.index = v;
            }
        } else if (name == QLatin1String("indexed")) {
            const int v = a.value().toInt(&ok);
            if (ok && v >= 0) {
                c.kind = ColorSpec::Kind::Indexed;
                c.index = v;
            }
        } else if (name == QLatin1String("auto")) {
            if (truth(a.value())) c.kind = ColorSpec::Kind::Auto;
        } else if (name == QLatin1String("tint")) {
            tint = std::clamp(number(a.value(), 0), -1.0, 1.0);
        }
    }
    if (c.kind == ColorSpec::Kind::Theme) c.tint = tint;
    return c;
}

QByteArray escaped(const QString &s)
{
    QByteArray out;
    for (const QChar ch : s) {
        if (ch == QLatin1Char('&')) out += "&amp;";
        else if (ch == QLatin1Char('<')) out += "&lt;";
        else if (ch == QLatin1Char('>')) out += "&gt;";
        else if (ch == QLatin1Char('"')) out += "&quot;";
        else out += QString(ch).toUtf8();
    }
    return out;
}

QByteArray num(double v) { return QByteArray::number(v, 'g', 15); }

const char *typeName(SparklineGroup::Type t) { return t == SparklineGroup::Type::Column ? "column" : t == SparklineGroup::Type::WinLoss ? "stacked" : "line"; }
const char *axisName(SparklineGroup::Axis a) { return a == SparklineGroup::Axis::Group ? "group" : a == SparklineGroup::Axis::Custom ? "custom" : "individual"; }

} // namespace

bool SparklineGroup::operator==(const SparklineGroup &o) const
{
    return type == o.type && empty == o.empty && lineWeight == o.lineWeight && markers == o.markers && high == o.high && low == o.low && first == o.first && last == o.last && negative == o.negative &&
           showAxis == o.showAxis && hidden == o.hidden && dateAxis == o.dateAxis && rightToLeft == o.rightToLeft && minAxis == o.minAxis && maxAxis == o.maxAxis && manualMin == o.manualMin &&
           manualMax == o.manualMax && series == o.series && negativeColor == o.negativeColor && axisColor == o.axisColor && markerColor == o.markerColor && firstColor == o.firstColor &&
           lastColor == o.lastColor && highColor == o.highColor && lowColor == o.lowColor && dateRange == o.dateRange && otherAttributes == o.otherAttributes && sparklines == o.sparklines;
}

ColorSpec defaultSparklineColor(const char *which)
{
    const QByteArray w(which);
    if (w == "negative") return ColorSpec::fromRgb(0xD00000);
    if (w == "axis") return ColorSpec::fromRgb(0x000000);
    if (w == "markers") return ColorSpec::fromRgb(0xD00000);
    if (w == "first") return ColorSpec::fromRgb(0xD00000);
    if (w == "last") return ColorSpec::fromRgb(0xD00000);
    if (w == "high") return ColorSpec::fromRgb(0xD00000);
    if (w == "low") return ColorSpec::fromRgb(0xD00000);
    return ColorSpec::fromRgb(0x376092);
}

void SparklineParser::start(const QXmlStreamReader &r, int rel)
{
    const QStringView name = r.name();
    if (rel == 1 && name == QLatin1String("sparklineGroup")) {
        m_inGroup = groups.size() < kMaxSparklineGroups;
        m_group = SparklineGroup();
        if (!m_inGroup) return;
        for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations())
            if (!d.prefix().isEmpty()) m_group.otherAttributes += " xmlns:" + d.prefix().toUtf8() + "=\"" + escaped(d.namespaceUri().toString()) + "\"";
        for (const QXmlStreamAttribute &a : r.attributes()) {
            const QStringView n = a.name(), v = a.value();
            if (!a.namespaceUri().isEmpty()) {
                // an attribute of another namespace (a revision id) is kept as it is
                m_group.otherAttributes += " " + a.qualifiedName().toUtf8() + "=\"" + escaped(v.toString()) + "\"";
            } else if (n == QLatin1String("type")) {
                m_group.type = v == QLatin1String("column") ? SparklineGroup::Type::Column : v == QLatin1String("stacked") ? SparklineGroup::Type::WinLoss : SparklineGroup::Type::Line;
            } else if (n == QLatin1String("displayEmptyCellsAs")) {
                m_group.empty = v == QLatin1String("gap") ? SparklineGroup::Empty::Gap : v == QLatin1String("span") ? SparklineGroup::Empty::Span : SparklineGroup::Empty::Zero;
            } else if (n == QLatin1String("lineWeight")) {
                m_group.lineWeight = std::clamp(number(v, 0.75), 0.0, 100.0);
            } else if (n == QLatin1String("markers")) {
                m_group.markers = truth(v);
            } else if (n == QLatin1String("high")) {
                m_group.high = truth(v);
            } else if (n == QLatin1String("low")) {
                m_group.low = truth(v);
            } else if (n == QLatin1String("first")) {
                m_group.first = truth(v);
            } else if (n == QLatin1String("last")) {
                m_group.last = truth(v);
            } else if (n == QLatin1String("negative")) {
                m_group.negative = truth(v);
            } else if (n == QLatin1String("displayXAxis")) {
                m_group.showAxis = truth(v);
            } else if (n == QLatin1String("displayHidden")) {
                m_group.hidden = truth(v);
            } else if (n == QLatin1String("dateAxis")) {
                m_group.dateAxis = truth(v);
            } else if (n == QLatin1String("rightToLeft")) {
                m_group.rightToLeft = truth(v);
            } else if (n == QLatin1String("minAxisType")) {
                m_group.minAxis = v == QLatin1String("group") ? SparklineGroup::Axis::Group : v == QLatin1String("custom") ? SparklineGroup::Axis::Custom : SparklineGroup::Axis::Individual;
            } else if (n == QLatin1String("maxAxisType")) {
                m_group.maxAxis = v == QLatin1String("group") ? SparklineGroup::Axis::Group : v == QLatin1String("custom") ? SparklineGroup::Axis::Custom : SparklineGroup::Axis::Individual;
            } else if (n == QLatin1String("manualMin")) {
                m_group.manualMin = number(v, 0);
            } else if (n == QLatin1String("manualMax")) {
                m_group.manualMax = number(v, 0);
            }
        }
        return;
    }
    if (!m_inGroup) return;
    if (rel == 2) {
        if (name == QLatin1String("colorSeries")) m_group.series = colorFrom(r);
        else if (name == QLatin1String("colorNegative")) m_group.negativeColor = colorFrom(r);
        else if (name == QLatin1String("colorAxis")) m_group.axisColor = colorFrom(r);
        else if (name == QLatin1String("colorMarkers")) m_group.markerColor = colorFrom(r);
        else if (name == QLatin1String("colorFirst")) m_group.firstColor = colorFrom(r);
        else if (name == QLatin1String("colorLast")) m_group.lastColor = colorFrom(r);
        else if (name == QLatin1String("colorHigh")) m_group.highColor = colorFrom(r);
        else if (name == QLatin1String("colorLow")) m_group.lowColor = colorFrom(r);
        else if (name == QLatin1String("f")) {
            into = Into::DateRange;
            m_text.clear();
        }
    } else if (rel == 3 && name == QLatin1String("sparkline")) {
        m_inOne = count < kMaxSparklines;
        m_one = Sparkline();
        m_text.clear();
    } else if (rel == 4 && m_inOne) {
        if (name == QLatin1String("f")) {
            into = Into::Source;
            m_text.clear();
        } else if (name == QLatin1String("sqref")) {
            into = Into::Cell;
            m_text.clear();
        }
    }
}

void SparklineParser::text(QStringView t)
{
    if (into != Into::None && m_text.size() < 4096) m_text += t;
}

void SparklineParser::end(int rel)
{
    if (rel == 2 && into == Into::DateRange) {
        m_group.dateRange = m_text.trimmed();
        into = Into::None;
    } else if (rel == 4 && into != Into::None) {
        if (into == Into::Source) m_one.source = m_text.trimmed();
        else {
            CellRange range;
            if (parseRangeRef(m_text.trimmed(), &range)) {
                m_one.row = range.row1;
                m_one.col = range.col1;
            } else {
                m_inOne = false;
            }
        }
        into = Into::None;
    } else if (rel == 3 && m_inOne) {
        m_inOne = false;
        if (!m_one.source.isEmpty()) {
            m_group.sparklines.append(m_one);
            ++count;
        }
    } else if (rel == 1 && m_inGroup) {
        m_inGroup = false;
        if (!m_group.sparklines.isEmpty()) groups.append(m_group);
    }
}

const char *sparklineExtensionUri() { return "{05C60535-1F16-4fd2-B633-F4F36F0B64E0}"; }

QByteArray sparklineExtensionXml(const QVector<SparklineGroup> &groups)
{
    QByteArray body;
    for (const SparklineGroup &g : groups) {
        if (g.sparklines.isEmpty()) continue;
        QByteArray e = "<x14:sparklineGroup";
        if (g.manualMax != 0 || g.maxAxis == SparklineGroup::Axis::Custom) e += " manualMax=\"" + num(g.manualMax) + "\"";
        if (g.manualMin != 0 || g.minAxis == SparklineGroup::Axis::Custom) e += " manualMin=\"" + num(g.manualMin) + "\"";
        if (g.lineWeight != 0.75) e += " lineWeight=\"" + num(g.lineWeight) + "\"";
        if (g.type != SparklineGroup::Type::Line) e += QByteArray(" type=\"") + typeName(g.type) + "\"";
        if (g.dateAxis) e += " dateAxis=\"1\"";
        if (g.empty != SparklineGroup::Empty::Zero) e += QByteArray(" displayEmptyCellsAs=\"") + (g.empty == SparklineGroup::Empty::Gap ? "gap" : "span") + "\"";
        if (g.markers) e += " markers=\"1\"";
        if (g.high) e += " high=\"1\"";
        if (g.low) e += " low=\"1\"";
        if (g.first) e += " first=\"1\"";
        if (g.last) e += " last=\"1\"";
        if (g.negative) e += " negative=\"1\"";
        if (g.showAxis) e += " displayXAxis=\"1\"";
        if (g.hidden) e += " displayHidden=\"1\"";
        if (g.minAxis != SparklineGroup::Axis::Individual) e += QByteArray(" minAxisType=\"") + axisName(g.minAxis) + "\"";
        if (g.maxAxis != SparklineGroup::Axis::Individual) e += QByteArray(" maxAxisType=\"") + axisName(g.maxAxis) + "\"";
        if (g.rightToLeft) e += " rightToLeft=\"1\"";
        e += g.otherAttributes + ">";
        auto color = [&](const char *name, const ColorSpec &c, const char *which) {
            (void)which;
            if (c.isSet()) e += colorXml(name, c);
        };
        color("x14:colorSeries", g.series, "series");
        color("x14:colorNegative", g.negativeColor, "negative");
        color("x14:colorAxis", g.axisColor, "axis");
        color("x14:colorMarkers", g.markerColor, "markers");
        color("x14:colorFirst", g.firstColor, "first");
        color("x14:colorLast", g.lastColor, "last");
        color("x14:colorHigh", g.highColor, "high");
        color("x14:colorLow", g.lowColor, "low");
        if (!g.dateRange.isEmpty()) e += "<xm:f>" + escaped(g.dateRange) + "</xm:f>";
        e += "<x14:sparklines>";
        for (const Sparkline &s : g.sparklines) e += "<x14:sparkline><xm:f>" + escaped(s.source) + "</xm:f><xm:sqref>" + cellName(s.row, s.col).toLatin1() + "</xm:sqref></x14:sparkline>";
        e += "</x14:sparklines></x14:sparklineGroup>";
        body += e;
    }
    if (body.isEmpty()) return QByteArray();
    return QByteArray("<ext xmlns:x14=\"") + kX14Ns + "\" uri=\"" + sparklineExtensionUri() + "\"><x14:sparklineGroups xmlns:xm=\"" + kXmNs + "\">" + body + "</x14:sparklineGroups></ext>";
}

} // namespace jp::office::sheet
