#include "office/chart.h"

#include "office/xml.h"

#include <QCoreApplication>
#include <QDate>
#include <QLocale>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::office {

namespace {

const QString kChartNs = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/chart");
const QString kStrictChartNs = QStringLiteral("http://purl.oclc.org/ooxml/drawingml/chart");
const QString kMarkupNs = QStringLiteral("http://schemas.openxmlformats.org/markup-compatibility/2006");

// What a chart part may hold: more is left out. The real charts here (five, in the reference decks) have one series of 63 points; a dozen
// series is a crowd, a few hundred points a big chart, and a long series of readings has a couple of thousand. These are many times the
// series, plots, and axes that a real chart has, and five times its longest series; and the picture drawn from them is coarser still
// (chart_svg.cpp).
constexpr int kMaxSeries = 256, kMaxPoints = 10000, kMaxPlots = 16, kMaxAxes = 16;
// The points all of a chart's series may have between them.
constexpr int kMaxChartPoints = 100000;
// A number format code is at most 255 characters in a spreadsheet; a longer one is none, and the numbers show as they are.
constexpr int kMaxFormatCode = 255;
// The text of a reference to cells (a list of areas can be long, but a few thousand characters is far more than a chart has).
constexpr int kMaxRefChars = 4096;
// A title and the like: a paragraph or two.
constexpr int kMaxTitle = 1000;

QString say(const char *text) { return QCoreApplication::translate("Chart", text); }

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

bool isChartNs(const QString &uri) { return uri == kChartNs || uri == kStrictChartNs; }

// The first child of the chart namespace with that local name; null if none.
QDomElement cChild(const QDomElement &e, const char *name)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(name) && isChartNs(c.namespaceURI())) return c;
    return QDomElement();
}

QVector<QDomElement> cChildren(const QDomElement &e, const char *name)
{
    QVector<QDomElement> out;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(name) && isChartNs(c.namespaceURI())) out << c;
    return out;
}

QDomElement aChild(const QDomElement &e, const char *name) { return child(e, ns::A, QLatin1String(name)); }

// An element's `val` attribute; `fallback` when there is none.
QString valOf(const QDomElement &e, const QString &fallback = QString()) { return attr(e, QStringLiteral("val"), fallback); }
// c:*'s boolean: a missing val means true.
bool flagOf(const QDomElement &e, bool fallback)
{
    if (e.isNull()) return fallback;
    return parseBool(valOf(e), true);
}

double numberOf(const QDomElement &e, double fallback = 0)
{
    bool ok = false;
    const double v = valOf(e).toDouble(&ok);
    return ok && std::isfinite(v) ? v : fallback;
}

// The first descendant with that local name, in the chart namespace or the one the other side's compatibility markup uses.
QDomElement findStyle(const QDomElement &e, int depth = 0)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (c.localName() == QLatin1String("style") && !c.hasChildNodes()) return c;
        if (depth < 4 && (c.localName() == QLatin1String("AlternateContent") || c.localName() == QLatin1String("Choice") || c.localName() == QLatin1String("Fallback")))
            if (const QDomElement s = findStyle(c, depth + 1); !s.isNull()) return s;
    }
    return QDomElement();
}

// ---- numbers as the file formats them

// A date and a time of day.
struct Moment {
    QDate d;
    QTime t;
    QDate date() const { return d; }
    QTime time() const { return t; }
};

// The date and time a serial number stands for (the 1900 date system, with its false February 29).
Moment dateOfSerial(double serial)
{
    const double days = std::floor(serial);
    const QDate base = days < 61 ? QDate(1899, 12, 31) : QDate(1899, 12, 30);
    const int seconds = int(std::lround((serial - days) * 86400));
    return {base.addDays(qint64(days)), QTime(0, 0).addSecs(std::clamp(seconds, 0, 86399))};
}

QString general(double v)
{
    if (v == 0) return QStringLiteral("0");
    const double a = std::fabs(v);
    if (a >= 1e11 || a < 1e-9) return QString::number(v, 'E', 5).replace(QRegularExpression(QStringLiteral("\\.?0+E")), QStringLiteral("E"));
    return QString::number(v, 'g', 10);
}

// The parts of a number format: its sections, split at ';' outside quotes.
QStringList formatSections(const QString &format)
{
    QStringList out;
    QString cur;
    bool quoted = false;
    for (int i = 0; i < format.size(); ++i) {
        const QChar c = format[i];
        if (c == QLatin1Char('"')) quoted = !quoted;
        if (c == QLatin1Char('\\') && i + 1 < format.size() && !quoted) {
            cur += c;
            cur += format[++i];
            continue;
        }
        if (c == QLatin1Char(';') && !quoted) {
            out << cur;
            cur.clear();
        } else {
            cur += c;
        }
    }
    out << cur;
    return out;
}

// Whether a section is a date or time: it has a letter that stands for one of their parts, outside quotes, brackets, and escapes.
bool isDateFormat(const QString &section)
{
    bool quoted = false, bracket = false;
    for (int i = 0; i < section.size(); ++i) {
        const QChar c = section[i];
        if (c == QLatin1Char('"')) quoted = !quoted;
        else if (quoted) continue;
        else if (c == QLatin1Char('[')) bracket = true;
        else if (c == QLatin1Char(']')) bracket = false;
        else if (bracket) continue;
        else if (c == QLatin1Char('\\') || c == QLatin1Char('_') || c == QLatin1Char('*')) ++i;
        else if (QStringLiteral("yYdDhHsS").contains(c)) return true;
        else if ((c == QLatin1Char('m') || c == QLatin1Char('M'))) return true;
    }
    return false;
}

QString formatDate(double serial, const QString &section)
{
    const Moment t = dateOfSerial(serial);
    const QLocale en = QLocale::c();
    QString out;
    const bool ampm = section.contains(QLatin1String("AM/PM"), Qt::CaseInsensitive) || section.contains(QLatin1String("A/P"), Qt::CaseInsensitive);
    int lastHour = -1;   // where an "m" after an hour or before a second is minutes, not a month
    for (int i = 0; i < section.size();) {
        const QChar c = section[i].toLower();
        auto run = [&](QChar ch) {
            int n = 0;
            while (i + n < section.size() && section[i + n].toLower() == ch) ++n;
            return n;
        };
        if (section[i] == QLatin1Char('"')) {
            const int end = section.indexOf(QLatin1Char('"'), i + 1);
            out += section.mid(i + 1, (end < 0 ? section.size() : end) - i - 1);
            i = end < 0 ? section.size() : end + 1;
        } else if (section[i] == QLatin1Char('[')) {
            const int end = section.indexOf(QLatin1Char(']'), i);
            i = end < 0 ? section.size() : end + 1;
        } else if (section[i] == QLatin1Char('\\') && i + 1 < section.size()) {
            out += section[i + 1];
            i += 2;
        } else if (section[i] == QLatin1Char('_') || section[i] == QLatin1Char('*')) {
            if (section[i] == QLatin1Char('_')) out += QLatin1Char(' ');
            i += 2;
        } else if (c == QLatin1Char('y')) {
            const int n = run(c);
            out += n <= 2 ? QStringLiteral("%1").arg(t.date().year() % 100, 2, 10, QLatin1Char('0')) : QString::number(t.date().year());
            i += n;
        } else if (c == QLatin1Char('d')) {
            const int n = run(c);
            out += n == 1 ? QString::number(t.date().day()) : n == 2 ? QStringLiteral("%1").arg(t.date().day(), 2, 10, QLatin1Char('0'))
                   : n == 3 ? en.dayName(t.date().dayOfWeek(), QLocale::ShortFormat) : en.dayName(t.date().dayOfWeek(), QLocale::LongFormat);
            i += n;
        } else if (c == QLatin1Char('h')) {
            const int n = run(c);
            const int hour = ampm ? (t.time().hour() % 12 == 0 ? 12 : t.time().hour() % 12) : t.time().hour();
            out += n == 1 ? QString::number(hour) : QStringLiteral("%1").arg(hour, 2, 10, QLatin1Char('0'));
            lastHour = out.size();
            i += n;
        } else if (c == QLatin1Char('s')) {
            const int n = run(c);
            out += n == 1 ? QString::number(t.time().second()) : QStringLiteral("%1").arg(t.time().second(), 2, 10, QLatin1Char('0'));
            i += n;
        } else if (c == QLatin1Char('m')) {
            const int n = run(c);
            // Minutes when an hour comes before it, or a second right after.
            int next = i + n;
            while (next < section.size() && !section[next].isLetter()) ++next;
            const bool minutes = lastHour >= 0 || (next < section.size() && section[next].toLower() == QLatin1Char('s'));
            if (minutes) out += n == 1 ? QString::number(t.time().minute()) : QStringLiteral("%1").arg(t.time().minute(), 2, 10, QLatin1Char('0'));
            else out += n == 1 ? QString::number(t.date().month()) : n == 2 ? QStringLiteral("%1").arg(t.date().month(), 2, 10, QLatin1Char('0'))
                        : n == 3 ? en.monthName(t.date().month(), QLocale::ShortFormat) : n == 4 ? en.monthName(t.date().month(), QLocale::LongFormat)
                        : en.monthName(t.date().month(), QLocale::LongFormat).left(1);
            i += n;
        } else if (QStringView(section).mid(i, 5).compare(QLatin1String("AM/PM"), Qt::CaseInsensitive) == 0) {
            out += t.time().hour() < 12 ? QStringLiteral("AM") : QStringLiteral("PM");
            i += 5;
        } else if (QStringView(section).mid(i, 3).compare(QLatin1String("A/P"), Qt::CaseInsensitive) == 0) {
            out += t.time().hour() < 12 ? QStringLiteral("A") : QStringLiteral("P");
            i += 3;
        } else {
            out += section[i];
            ++i;
        }
    }
    return out;
}

// What one section of a format code asks of a number, read once: a code is read for each of the numbers a chart shows with it (a category
// for each of thousands of points), and what it says doesn't change from one to the next.
struct NumberSection {
    QString text;                   // as written
    bool date = false;              // a date or a time: `text` is read for each number
    // What the section writes before the number's digits, how it wants the digits, and what it writes after.
    QString before, after;
    int integerZeros = 0, decimals = 0, optionalDecimals = 0, percents = 0, trailingCommas = 0, exponentDigits = 0;
    bool thousands = false, seenDigits = false, scientific = false, plain = false;
};

struct NumberFormat {
    bool general = true;            // no code, "General", or a code too long to be one
    QVector<NumberSection> sections;
};

NumberSection readSection(const QString &section)
{
    NumberSection out;
    out.text = section;
    if (isDateFormat(section)) {
        out.date = true;
        return out;
    }
    QString *literal = &out.before;
    bool seenDigits = false, afterPoint = false;
    auto isPlace = [](QChar c) { return c == QLatin1Char('0') || c == QLatin1Char('#') || c == QLatin1Char('?'); };
    for (int i = 0; i < section.size(); ++i) {
        const QChar c = section[i];
        if (c == QLatin1Char('"')) {
            const int end = section.indexOf(QLatin1Char('"'), i + 1);
            *literal += section.mid(i + 1, (end < 0 ? section.size() : end) - i - 1);
            i = end < 0 ? section.size() : end;
        } else if (c == QLatin1Char('\\') && i + 1 < section.size()) {
            *literal += section[++i];
        } else if (c == QLatin1Char('_') && i + 1 < section.size()) {
            *literal += QLatin1Char(' ');
            ++i;
        } else if (c == QLatin1Char('*') && i + 1 < section.size()) {
            ++i;
        } else if (c == QLatin1Char('[')) {
            // A currency, [$€-407], shows its symbol; a color or a condition shows nothing.
            const int end = section.indexOf(QLatin1Char(']'), i);
            const QString inside = section.mid(i + 1, (end < 0 ? section.size() : end) - i - 1);
            if (inside.startsWith(QLatin1Char('$'))) *literal += inside.mid(1).section(QLatin1Char('-'), 0, 0);
            i = end < 0 ? section.size() : end;
        } else if (isPlace(c) && !seenDigits) {
            seenDigits = true;
            literal = &out.after;
            if (c == QLatin1Char('0')) ++out.integerZeros;
        } else if (isPlace(c) && seenDigits) {
            if (out.scientific) ++out.exponentDigits;
            else if (afterPoint) (c == QLatin1Char('0') ? out.decimals : out.optionalDecimals) += 1;
            else if (c == QLatin1Char('0')) ++out.integerZeros;
            out.trailingCommas = 0;
        } else if (c == QLatin1Char('.') && !afterPoint && !out.scientific && (seenDigits || (i + 1 < section.size() && isPlace(section[i + 1])))) {
            seenDigits = true;
            literal = &out.after;
            afterPoint = true;
        } else if (c == QLatin1Char(',') && seenDigits && !out.scientific) {
            // Between places: a thousands separator; after the last of them: a division by 1,000.
            if (!afterPoint && i + 1 < section.size() && isPlace(section[i + 1])) out.thousands = true;
            else ++out.trailingCommas;
        } else if (c == QLatin1Char('%')) {
            ++out.percents;
            *literal += c;
        } else if ((c == QLatin1Char('E') || c == QLatin1Char('e')) && seenDigits && !out.scientific && i + 1 < section.size() && (section[i + 1] == QLatin1Char('+') || section[i + 1] == QLatin1Char('-'))) {
            out.scientific = true;
            ++i;
        } else if (QStringView(section).mid(i, 7).compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) {
            out.plain = true;
            seenDigits = true;
            literal = &out.after;
            i += 6;
        } else {
            *literal += c;
        }
    }
    out.seenDigits = seenDigits;
    return out;
}

NumberFormat readFormat(const QString &format)
{
    NumberFormat out;
    if (format.isEmpty() || format.size() > kMaxFormatCode || format.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) return out;
    out.general = false;
    for (const QString &section : formatSections(format)) out.sections << readSection(section);
    return out;
}

// A number as its format code shows it ("0.0%", "#,##0", "$#,##0.00;($#,##0.00)", "mmm-yy"...). "General" shows it as short as it can be.
QString formatWith(const NumberFormat &format, double v)
{
    if (!std::isfinite(v)) return QString();
    if (format.general) return general(v);
    const NumberSection *use = &format.sections[0];
    bool minus = false;
    if (v < 0) {
        if (format.sections.size() > 1 && !format.sections[1].text.isEmpty()) use = &format.sections[1];
        else minus = true;
        v = -v;
    } else if (v == 0 && format.sections.size() > 2 && !format.sections[2].text.isEmpty()) {
        use = &format.sections[2];
    }
    const NumberSection &section = *use;
    if (section.date) return formatDate(v, section.text);

    double scaled = v;
    for (int i = 0; i < section.percents; ++i) scaled *= 100;
    for (int i = 0; i < section.trailingCommas; ++i) scaled /= 1000;
    QString number;
    if (section.plain || !section.seenDigits) {
        number = section.plain ? general(scaled) : QString();
    } else if (section.scientific) {
        const QString mantissa = QString::number(scaled, 'E', section.decimals + section.optionalDecimals);
        const int e = mantissa.indexOf(QLatin1Char('E'));
        const QString exponent = mantissa.mid(e + 1);
        number = mantissa.left(e) + QLatin1Char('E') + exponent.left(1) + exponent.mid(1).rightJustified(std::max(section.exponentDigits, 2), QLatin1Char('0'));
    } else {
        number = QString::number(scaled, 'f', section.decimals + section.optionalDecimals);
        // Optional decimal places that come out zero are not written.
        for (int drop = 0; drop < section.optionalDecimals && number.endsWith(QLatin1Char('0')); ++drop) number.chop(1);
        if (number.endsWith(QLatin1Char('.'))) number.chop(1);
        QString whole = number.section(QLatin1Char('.'), 0, 0);
        const QString fraction = number.contains(QLatin1Char('.')) ? number.section(QLatin1Char('.'), 1) : QString();
        if (whole == QLatin1String("0") && section.integerZeros == 0 && !fraction.isEmpty()) whole.clear();
        while (whole.size() < section.integerZeros) whole.prepend(QLatin1Char('0'));
        if (section.thousands)
            for (int i = int(whole.size()) - 3; i > 0; i -= 3) whole.insert(i, QLatin1Char(','));
        number = whole + (fraction.isEmpty() ? QString() : QLatin1Char('.') + fraction);
    }
    return (minus ? QStringLiteral("-") : QString()) + section.before + number + section.after;
}

QString formatNumber(double v, const QString &format) { return formatWith(readFormat(format), v); }

// ---- reading

// The colors a chart's series and slices take when the file gives none: the
// accents in order, then each again for each of the variations the standard
// colors list names (darker, lighter...), as a color list in a chart style
// part spells them.
QVector<QColor> paletteOf(const Theme &theme, const ColorMap &map)
{
    struct Variation { double mod, off; };
    static const Variation variations[] = {{1, 0}, {0.6, 0}, {0.8, 0.2}, {0.8, 0}, {0.6, 0.4}, {0.5, 0}, {0.7, 0.3}, {0.7, 0}, {0.5, 0.5}};
    QVector<QColor> out;
    for (const Variation &v : variations)
        for (int i = 0; i < 6; ++i) {
            QColor c = theme.colors[map.accent[i]];
            if (!c.isValid()) c = QColor(Qt::gray);
            if (v.mod != 1 || v.off != 0) {
                const QColor hsl = c.toHsl();
                c = QColor::fromHslF(hsl.hslHueF() < 0 ? 0 : hsl.hslHueF(), hsl.hslSaturationF(), std::clamp(hsl.lightnessF() * v.mod + v.off, 0.0, 1.0));
            }
            out << c;
        }
    return out;
}

// The turn a text's a:bodyPr asks for, in degrees from -180 to 180; false for none, or for the number that stands for "as the chart sees fit" (-60000000).
bool turnOf(const QDomElement &bodyPr, double *degrees)
{
    if (!hasAttr(bodyPr, QStringLiteral("rot"))) return false;
    const double d = parseAngle(attr(bodyPr, QStringLiteral("rot")), 0);
    if (std::fabs(d) > 360) return false;
    *degrees = d > 180 ? d - 360 : d <= -180 ? d + 360 : d;
    return true;
}

// What a data cache holds: the strings or the numbers of a series' c:cat, c:val, c:xVal, or c:yVal.
struct Data {
    QStringList strings;
    QVector<double> numbers;
    QString format = QStringLiteral("General");
    bool numeric = false;   // the file stores numbers
    bool present = false;
    QString ref;            // the c:f of the c:numRef or c:strRef the cache is of
};

class Reader {
public:
    Reader(const Theme &theme, const ColorMap &colors, Chart *chart) : m_theme(theme), m_map(colors), m_chart(chart) {}
    bool read(const QDomElement &root, QString *error);

private:
    QColor colorIn(const QDomElement &parent) const { return readColor(parent, m_theme, m_map); }
    void readFill(const QDomElement &spPr, QColor *color, bool *none) const;
    ChartLine readLine(const QDomElement &spPr) const;
    void applyRunProps(const QDomElement &rPr, ChartFont *font) const;
    ChartFont readTxPr(const QDomElement &txPr, bool *hasRotation = nullptr, double *rotation = nullptr) const;
    ChartTitle readTitle(const QDomElement &title) const;
    QString richText(const QDomElement &rich, ChartFont *font, bool *hasRotation, double *rotation) const;
    Data readData(const QDomElement &parent) const;
    ChartDataLabels readLabels(const QDomElement &dLbls) const;
    ChartMarker readMarker(const QDomElement &marker) const;
    ChartSeries readSeries(const QDomElement &ser, const ChartPlot &plot, int *order, bool *ownLabels) const;
    bool readPlot(const QDomElement &e);
    ChartAxis readAxis(const QDomElement &e) const;
    void readLayout(const QDomElement &layout);
    void applyStyleColors();

    const Theme &m_theme;
    ColorMap m_map;
    Chart *m_chart;
    mutable int m_pointsLeft = kMaxChartPoints;
    int m_seriesLeft = kMaxSeries;
};

void Reader::readFill(const QDomElement &spPr, QColor *color, bool *none) const
{
    if (spPr.isNull()) return;
    if (!aChild(spPr, "noFill").isNull()) {
        *none = true;
        return;
    }
    if (const QDomElement solid = aChild(spPr, "solidFill"); !solid.isNull()) {
        const QColor c = colorIn(solid);
        if (c.isValid()) *color = c;
    } else if (const QDomElement grad = aChild(spPr, "gradFill"); !grad.isNull()) {
        // A gradient is drawn in the color of its first stop.
        const QDomElement stop = aChild(aChild(grad, "gsLst"), "gs");
        const QColor c = colorIn(stop);
        if (c.isValid()) *color = c;
    } else if (const QDomElement pattern = aChild(spPr, "pattFill"); !pattern.isNull()) {
        const QColor c = colorIn(aChild(pattern, "fgClr"));
        if (c.isValid()) *color = c;
    }
}

ChartLine Reader::readLine(const QDomElement &spPr) const
{
    ChartLine line;
    const QDomElement ln = aChild(spPr, "ln");
    if (ln.isNull()) return line;
    line.set = true;
    if (hasAttr(ln, QStringLiteral("w"))) line.width = std::clamp(parseLength(attr(ln, QStringLiteral("w")), 0), 0.0, 100.0);
    if (!aChild(ln, "noFill").isNull()) {
        line.none = true;
    } else if (const QDomElement solid = aChild(ln, "solidFill"); !solid.isNull()) {
        line.color = colorIn(solid);
    } else if (const QDomElement grad = aChild(ln, "gradFill"); !grad.isNull()) {
        line.color = colorIn(aChild(aChild(grad, "gsLst"), "gs"));
    }
    line.dash = attr(aChild(ln, "prstDash"), QStringLiteral("val"));
    if (line.dash == QLatin1String("solid")) line.dash.clear();
    return line;
}

void Reader::applyRunProps(const QDomElement &r, ChartFont *font) const
{
    if (r.isNull()) return;
    if (hasAttr(r, QStringLiteral("sz"))) {
        const double size = parseInt(attr(r, QStringLiteral("sz"))) / 100.0;
        if (size >= 1 && size <= 400) font->size = size;
    }
    if (hasAttr(r, QStringLiteral("b"))) font->bold = parseBool(attr(r, QStringLiteral("b"))) ? 1 : 0;
    if (hasAttr(r, QStringLiteral("i"))) font->italic = parseBool(attr(r, QStringLiteral("i"))) ? 1 : 0;
    if (const QColor c = colorIn(aChild(r, "solidFill")); c.isValid()) font->color = c;
    const QString face = attr(aChild(r, "latin"), QStringLiteral("typeface"));
    if (face == QLatin1String("+mn-lt")) font->family = m_theme.minorLatin;
    else if (face == QLatin1String("+mj-lt")) font->family = m_theme.majorLatin;
    else if (!face.isEmpty() && !face.startsWith(QLatin1Char('+'))) font->family = face;
}

ChartFont Reader::readTxPr(const QDomElement &txPr, bool *hasRotation, double *rotation) const
{
    ChartFont font;
    if (txPr.isNull()) return font;
    if (hasRotation && turnOf(aChild(txPr, "bodyPr"), rotation)) *hasRotation = true;
    const QDomElement p = aChild(txPr, "p");
    applyRunProps(aChild(aChild(p, "pPr"), "defRPr"), &font);
    return font;
}

// The text of a c:rich (paragraphs joined by newlines) and the look of its first run.
QString Reader::richText(const QDomElement &rich, ChartFont *font, bool *hasRotation, double *rotation) const
{
    if (turnOf(aChild(rich, "bodyPr"), rotation)) *hasRotation = true;
    QStringList lines;
    bool first = true;
    for (const QDomElement &p : children(rich, ns::A, QStringLiteral("p"))) {
        if (first) applyRunProps(aChild(aChild(p, "pPr"), "defRPr"), font);
        QString line;
        bool firstRun = true;
        for (const QDomElement &r : children(p)) {
            if (r.localName() == QLatin1String("r") || r.localName() == QLatin1String("fld")) {
                if (first && firstRun) applyRunProps(aChild(r, "rPr"), font);
                firstRun = false;
                line += aChild(r, "t").text();
            } else if (r.localName() == QLatin1String("br")) {
                line += QLatin1Char('\n');
            }
        }
        lines << line;
        first = false;
    }
    return lines.join(QLatin1Char('\n')).left(kMaxTitle);
}

ChartTitle Reader::readTitle(const QDomElement &e) const
{
    ChartTitle t;
    if (e.isNull()) return t;
    t.present = true;
    const QDomElement tx = cChild(e, "tx");
    if (const QDomElement r = cChild(tx, "rich"); !r.isNull()) {
        t.text = richText(r, &t.font, &t.hasRotation, &t.rotation);
    } else if (const QDomElement ref = cChild(tx, "strRef"); !ref.isNull()) {
        t.text = cChild(cChild(cChild(ref, "strCache"), "pt"), "v").text().left(kMaxTitle);
        t.ref = cChild(ref, "f").text().trimmed().left(kMaxRefChars);
    }
    t.overlay = flagOf(cChild(e, "overlay"), false);
    const ChartFont fromProps = readTxPr(cChild(e, "txPr"), &t.hasRotation, &t.rotation);
    // The title's own text properties are what the runs leave unsaid.
    if (t.font.size == 0) t.font.size = fromProps.size;
    if (t.font.bold < 0) t.font.bold = fromProps.bold;
    if (t.font.italic < 0) t.font.italic = fromProps.italic;
    if (!t.font.color.isValid()) t.font.color = fromProps.color;
    if (t.font.family.isEmpty()) t.font.family = fromProps.family;
    return t;
}

// A c:cat, c:val, c:xVal, or c:yVal: the cache of whatever it refers to, or its literal data.
Data Reader::readData(const QDomElement &parent) const
{
    Data data;
    if (parent.isNull()) return data;
    QDomElement cache;
    struct Kind { const char *ref, *cache; bool numeric; };
    for (const Kind &k : {Kind{"numRef", "numCache", true}, Kind{"strRef", "strCache", false}, Kind{"multiLvlStrRef", "multiLvlStrCache", false}})
        if (const QDomElement ref = cChild(parent, k.ref); !ref.isNull()) {
            cache = cChild(ref, k.cache);
            data.numeric = k.numeric;
            data.ref = cChild(ref, "f").text().trimmed().left(kMaxRefChars);
            break;
        }
    if (cache.isNull())
        for (const Kind &k : {Kind{"numLit", "", true}, Kind{"strLit", "", false}})
            if (const QDomElement lit = cChild(parent, k.ref); !lit.isNull()) {
                cache = lit;
                data.numeric = k.numeric;
                break;
            }
    if (cache.isNull()) return data;
    data.present = true;
    // A cache of several levels: the labels of the innermost.
    QDomElement points = cache;
    if (const QDomElement lvl = cChild(cache, "lvl"); !lvl.isNull()) points = lvl;
    if (data.numeric) data.format = cChild(cache, "formatCode").text().trimmed();
    if (data.format.isEmpty()) data.format = QStringLiteral("General");
    qint64 count = parseInt(valOf(cChild(cache, "ptCount")), -1);
    const QVector<QDomElement> pts = cChildren(points, "pt");
    if (count < 0) {
        count = 0;
        for (const QDomElement &pt : pts) count = std::max(count, parseInt(attr(pt, QStringLiteral("idx")), -1) + 1);
    }
    count = std::clamp<qint64>(count, 0, std::min(kMaxPoints, m_pointsLeft));
    m_pointsLeft -= int(count);
    if (data.numeric) data.numbers = QVector<double>(int(count), kNaN);
    else data.strings = QStringList(int(count));
    // A cache has no more points than it says it has (and each place only once): the rest of a longer list is not read.
    for (const QDomElement &pt : pts.mid(0, int(count))) {
        const qint64 idx = parseInt(attr(pt, QStringLiteral("idx")), -1);
        if (idx < 0 || idx >= count) continue;
        const QString text = cChild(pt, "v").text();
        if (data.numeric) {
            bool ok = false;
            const double v = text.trimmed().toDouble(&ok);
            if (ok && std::isfinite(v)) data.numbers[int(idx)] = v;
        } else {
            data.strings[int(idx)] = text.left(kChartMaxLabel);
        }
    }
    return data;
}

ChartDataLabels Reader::readLabels(const QDomElement &e) const
{
    ChartDataLabels l;
    if (e.isNull() || flagOf(cChild(e, "delete"), false)) return l;
    l.value = flagOf(cChild(e, "showVal"), false);
    l.category = flagOf(cChild(e, "showCatName"), false);
    l.series = flagOf(cChild(e, "showSerName"), false);
    l.percent = flagOf(cChild(e, "showPercent"), false);
    l.position = valOf(cChild(e, "dLblPos"));
    if (const QDomElement fmt = cChild(e, "numFmt"); !fmt.isNull() && !parseBool(attr(fmt, QStringLiteral("sourceLinked")), false)) l.format = attr(fmt, QStringLiteral("formatCode"));
    l.font = readTxPr(cChild(e, "txPr"));
    l.separator = cChild(e, "separator").text().left(kChartMaxLabel);
    return l;
}

ChartMarker Reader::readMarker(const QDomElement &e) const
{
    ChartMarker m;
    if (e.isNull()) return m;
    static const QMap<QString, ChartMarker::Symbol> symbols = {
        {QStringLiteral("auto"), ChartMarker::Auto}, {QStringLiteral("none"), ChartMarker::None}, {QStringLiteral("circle"), ChartMarker::Circle},
        {QStringLiteral("dash"), ChartMarker::Dash}, {QStringLiteral("diamond"), ChartMarker::Diamond}, {QStringLiteral("dot"), ChartMarker::Dot},
        {QStringLiteral("plus"), ChartMarker::Plus}, {QStringLiteral("square"), ChartMarker::Square}, {QStringLiteral("star"), ChartMarker::Star},
        {QStringLiteral("triangle"), ChartMarker::Triangle}, {QStringLiteral("x"), ChartMarker::X}};
    m.symbol = symbols.value(valOf(cChild(e, "symbol")), ChartMarker::Auto);
    if (const QDomElement size = cChild(e, "size"); !size.isNull()) m.size = std::clamp(numberOf(size, 5), 2.0, 72.0);
    const QDomElement spPr = cChild(e, "spPr");
    readFill(spPr, &m.fill, &m.noFill);
    const ChartLine line = readLine(spPr);
    m.line = line.color;
    m.noLine = line.none;
    return m;
}

ChartSeries Reader::readSeries(const QDomElement &ser, const ChartPlot &plot, int *order, bool *ownLabels) const
{
    ChartSeries s;
    const int idx = int(std::clamp<qint64>(parseInt(valOf(cChild(ser, "idx")), 0), 0, 1000000));
    *order = int(std::clamp<qint64>(parseInt(valOf(cChild(ser, "order")), idx), 0, 1000000));
    // Its name.
    const QDomElement tx = cChild(ser, "tx");
    if (const QDomElement ref = cChild(tx, "strRef"); !ref.isNull()) {
        const QDomElement pt = cChild(cChild(ref, "strCache"), "pt");
        s.name = cChild(pt, "v").text().left(kChartMaxLabel);
        s.nameRef = cChild(ref, "f").text().trimmed().left(kMaxRefChars);
    } else if (!tx.isNull()) {
        s.name = cChild(tx, "v").text().left(kChartMaxLabel);
    }
    if (s.name.isEmpty()) s.name = say("Series%1").arg(idx + 1);
    // Its look.
    const QDomElement spPr = cChild(ser, "spPr");
    const bool stroked = plot.type == ChartType::Line || plot.type == ChartType::Scatter;
    QColor fill;
    readFill(spPr, &fill, &s.noFill);
    s.outline = readLine(spPr);
    QColor color = stroked ? s.outline.color : fill;
    if (stroked && !color.isValid()) color = fill;
    s.colorFromFile = color.isValid();
    s.color = color.isValid() ? color : m_chart->palette.value(idx % std::max<int>(1, m_chart->palette.size()), QColor(Qt::gray));
    s.marker = readMarker(cChild(ser, "marker"));
    s.smooth = flagOf(cChild(ser, "smooth"), false);
    s.explosion = std::clamp(numberOf(cChild(ser, "explosion"), 0), 0.0, 400.0);
    for (const QDomElement &dPt : cChildren(ser, "dPt")) {
        const int at = int(parseInt(valOf(cChild(dPt, "idx")), -1));
        if (at < 0 || at > kMaxPoints) continue;
        QColor c;
        bool none = false;
        readFill(cChild(dPt, "spPr"), &c, &none);
        if (c.isValid()) s.pointColors.insert(at, c);
    }
    *ownLabels = !cChild(ser, "dLbls").isNull();
    if (*ownLabels) s.labels = readLabels(cChild(ser, "dLbls"));
    // Its data.
    const bool scatter = plot.type == ChartType::Scatter;
    const Data values = readData(cChild(ser, scatter ? "yVal" : "val"));
    const Data categories = readData(cChild(ser, scatter ? "xVal" : "cat"));
    s.values = values.numbers;
    s.format = values.format;
    s.valueRef = values.ref;
    (scatter ? s.xRef : s.categoryRef) = categories.ref;
    s.categoryRefNumeric = categories.numeric;
    if (scatter) {
        s.xValues = categories.numbers;
        if (!categories.numeric && categories.present) {
            // Text for x: the points are put at 1, 2, 3...
            s.xValues = QVector<double>(int(categories.strings.size()));
            for (int i = 0; i < s.xValues.size(); ++i) s.xValues[i] = i + 1;
        }
        s.categories = categories.strings;
    } else if (categories.numeric) {
        const NumberFormat format = readFormat(categories.format);
        for (double v : categories.numbers) s.categories << (std::isnan(v) ? QString() : formatWith(format, v).left(kChartMaxLabel));
        s.categoryNumbers = categories.numbers;
        s.categoryFormat = categories.format;
    } else {
        s.categories = categories.strings;
    }
    return s;
}

bool Reader::readPlot(const QDomElement &e)
{
    const QString name = e.localName();
    ChartPlot plot;
    plot.element = name;
    const bool threeD = name.endsWith(QLatin1String("3DChart"));
    const QString base = threeD ? name.left(name.size() - 7) : name.left(name.size() - 5);   // "bar" of barChart and bar3DChart
    if (base == QLatin1String("bar")) plot.type = ChartType::Bar;
    else if (base == QLatin1String("line")) plot.type = ChartType::Line;
    else if (base == QLatin1String("area")) plot.type = ChartType::Area;
    else if (base == QLatin1String("pie") || base == QLatin1String("ofPie")) plot.type = ChartType::Pie;
    else if (base == QLatin1String("doughnut")) plot.type = ChartType::Doughnut;
    else if (base == QLatin1String("scatter") || base == QLatin1String("bubble")) plot.type = ChartType::Scatter;
    else plot.type = ChartType::Other;

    const QString grouping = valOf(cChild(e, "grouping"));
    if (grouping == QLatin1String("stacked")) plot.grouping = ChartGrouping::Stacked;
    else if (grouping == QLatin1String("percentStacked")) plot.grouping = ChartGrouping::PercentStacked;
    else if (grouping == QLatin1String("standard")) plot.grouping = ChartGrouping::Standard;
    else plot.grouping = plot.type == ChartType::Bar ? ChartGrouping::Clustered : ChartGrouping::Standard;
    plot.horizontal = valOf(cChild(e, "barDir")) == QLatin1String("bar");
    plot.varyColors = flagOf(cChild(e, "varyColors"), plot.type == ChartType::Pie || plot.type == ChartType::Doughnut);
    plot.gapWidth = int(std::clamp<qint64>(parseInt(valOf(cChild(e, "gapWidth")), 150), 0, 500));
    plot.overlap = int(std::clamp<qint64>(parseInt(valOf(cChild(e, "overlap")), plot.grouping == ChartGrouping::Stacked || plot.grouping == ChartGrouping::PercentStacked ? 100 : 0), -100, 100));
    plot.markers = flagOf(cChild(e, "marker"), true);
    plot.firstSliceAngle = int(std::clamp<qint64>(parseInt(valOf(cChild(e, "firstSliceAng")), 0), 0, 360));
    plot.holeSize = int(std::clamp<qint64>(parseInt(valOf(cChild(e, "holeSize")), 50), 10, 90));
    if (plot.type == ChartType::Scatter) {
        const QString style = valOf(cChild(e, "scatterStyle"), QStringLiteral("lineMarker"));
        plot.lines = base == QLatin1String("scatter") && style != QLatin1String("marker") && style != QLatin1String("none");
    }
    const QVector<QDomElement> ids = cChildren(e, "axId");
    for (int i = 0; i < 2 && i < ids.size(); ++i) plot.axisIds[i] = parseInt(valOf(ids[i]), 0);

    struct Found {
        int order;
        ChartSeries series;
        bool ownLabels;
    };
    QVector<Found> found;
    for (const QDomElement &ser : cChildren(e, "ser")) {
        if (found.size() >= m_seriesLeft) break;
        Found f;
        f.series = readSeries(ser, plot, &f.order, &f.ownLabels);
        found << f;
    }
    m_seriesLeft -= int(found.size());
    std::stable_sort(found.begin(), found.end(), [](const Found &a, const Found &b) { return a.order < b.order; });
    // A series with no labels of its own has the plot's.
    const bool plotHasLabels = !cChild(e, "dLbls").isNull();
    const ChartDataLabels plotLabels = readLabels(cChild(e, "dLbls"));
    for (Found &f : found) {
        if (!f.ownLabels && plotHasLabels) f.series.labels = plotLabels;
        plot.series << f.series;
    }
    m_chart->plots << plot;
    return true;
}

ChartAxis Reader::readAxis(const QDomElement &e) const
{
    ChartAxis a;
    const QString name = e.localName();
    a.kind = name == QLatin1String("valAx") ? ChartAxis::Value : name == QLatin1String("dateAx") ? ChartAxis::Date : name == QLatin1String("serAx") ? ChartAxis::Series : ChartAxis::Category;
    a.id = parseInt(valOf(cChild(e, "axId")), 0);
    a.crossId = parseInt(valOf(cChild(e, "crossAx")), 0);
    a.deleted = flagOf(cChild(e, "delete"), false);
    a.position = valOf(cChild(e, "axPos"), a.kind == ChartAxis::Value ? QStringLiteral("l") : QStringLiteral("b"));
    const QDomElement scaling = cChild(e, "scaling");
    a.reversed = valOf(cChild(scaling, "orientation")) == QLatin1String("maxMin");
    if (const QDomElement max = cChild(scaling, "max"); !max.isNull()) {
        a.autoMax = false;
        a.max = numberOf(max);
    }
    if (const QDomElement min = cChild(scaling, "min"); !min.isNull()) {
        a.autoMin = false;
        a.min = numberOf(min);
    }
    if (const QDomElement grid = cChild(e, "majorGridlines"); !grid.isNull()) {
        a.majorGridlines = true;
        a.gridline = readLine(cChild(grid, "spPr"));
    }
    a.minorGridlines = !cChild(e, "minorGridlines").isNull();
    a.title = readTitle(cChild(e, "title"));
    if (const QDomElement fmt = cChild(e, "numFmt"); !fmt.isNull()) {
        a.format = attr(fmt, QStringLiteral("formatCode"), QStringLiteral("General"));
        a.formatLinked = parseBool(attr(fmt, QStringLiteral("sourceLinked")), true);
    }
    a.majorTick = valOf(cChild(e, "majorTickMark"), QStringLiteral("out"));
    a.minorTick = valOf(cChild(e, "minorTickMark"), QStringLiteral("none"));
    a.labelPosition = valOf(cChild(e, "tickLblPos"), QStringLiteral("nextTo"));
    a.line = readLine(cChild(e, "spPr"));
    a.font = readTxPr(cChild(e, "txPr"), &a.hasRotation, &a.labelRotation);
    a.crosses = valOf(cChild(e, "crosses"), QStringLiteral("autoZero"));
    if (const QDomElement at = cChild(e, "crossesAt"); !at.isNull()) {
        a.hasCrossesAt = true;
        a.crossesAt = numberOf(at);
    }
    a.labelSkip = int(std::clamp<qint64>(parseInt(valOf(cChild(e, "tickLblSkip")), 0), 0, 10000));
    a.majorUnit = std::max(0.0, numberOf(cChild(e, "majorUnit"), 0));
    a.minorUnit = std::max(0.0, numberOf(cChild(e, "minorUnit"), 0));
    a.between = valOf(cChild(e, "crossBetween"), QStringLiteral("between")) != QLatin1String("midCat");
    return a;
}

// c:layout's c:manualLayout: the plot area's place as fractions of the chart, when the file fixes it.
void Reader::readLayout(const QDomElement &layout)
{
    const QDomElement manual = cChild(layout, "manualLayout");
    if (manual.isNull()) return;
    if (valOf(cChild(manual, "xMode"), QStringLiteral("factor")) != QLatin1String("edge") || valOf(cChild(manual, "yMode"), QStringLiteral("factor")) != QLatin1String("edge")) return;
    const double w = numberOf(cChild(manual, "w"), -1), h = numberOf(cChild(manual, "h"), -1);
    if (w <= 0 || h <= 0) return;
    m_chart->plotManual = true;
    m_chart->plotInner = valOf(cChild(manual, "layoutTarget"), QStringLiteral("outer")) == QLatin1String("inner");
    m_chart->plotX = std::clamp(numberOf(cChild(manual, "x"), 0), 0.0, 1.0);
    m_chart->plotY = std::clamp(numberOf(cChild(manual, "y"), 0), 0.0, 1.0);
    m_chart->plotW = std::clamp(w, 0.01, 1.0);
    m_chart->plotH = std::clamp(h, 0.01, 1.0);
}

// The color of the series (or point) number `k` of `n` in a chart style that colors them in shades of one color: from darker than it to lighter.
static QColor shadeOf(const QColor &base, int k, int n)
{
    const double f = n <= 1 ? 0.0 : -0.45 + 1.0 * k / (n - 1);
    const QColor hsl = base.toHsl();
    const double l = hsl.lightnessF();
    return QColor::fromHslF(hsl.hslHueF() < 0 ? 0 : hsl.hslHueF(), hsl.hslSaturationF(), std::clamp(f < 0 ? l * (1 + f) : l + (1 - l) * f, 0.0, 1.0));
}

// The chart styles count in blocks of eight: the first of each is shades of gray, the second the accents in turn, and the other six shades of
// one accent each. A series the file gives no color takes the one its style says.
void Reader::applyStyleColors()
{
    Chart &c = *m_chart;
    const int kind = (c.style - 1) % 8;
    if (kind == 1) return;
    int total = 0;
    for (const ChartPlot &p : std::as_const(c.plots)) total += int(p.series.size());
    const QColor base = kind == 0 ? QColor(128, 128, 128) : m_theme.colors[m_map.accent[kind - 2]];
    int k = 0;
    for (ChartPlot &p : c.plots)
        for (ChartSeries &s : p.series) {
            if (!s.colorFromFile) s.color = shadeOf(base, k, total);
            ++k;
        }
    // Slices, and points that vary their color, step through the shades too.
    for (int i = 0; i < c.palette.size(); ++i) c.palette[i] = shadeOf(base, i % 9, 9);
}

bool Reader::read(const QDomElement &root, QString *error)
{
    Chart &c = *m_chart;
    if (!isElement(root, kChartNs, QStringLiteral("chartSpace")) && !isElement(root, kStrictChartNs, QStringLiteral("chartSpace"))) {
        if (error) *error = say("The part is not a chart.");
        return false;
    }
    const QDomElement chart = cChild(root, "chart");
    if (chart.isNull()) {
        if (error) *error = say("The chart has no drawing in it.");
        return false;
    }
    if (const QDomElement over = cChild(root, "clrMapOvr"); !over.isNull()) m_map = readColorMap(over);
    c.palette = paletteOf(m_theme, m_map);
    c.fontFamily = m_theme.minorLatin.isEmpty() ? QStringLiteral("Carlito") : m_theme.minorLatin;
    c.textColor = m_theme.colors[m_map.tx1].isValid() ? m_theme.colors[m_map.tx1] : QColor(Qt::black);
    c.backgroundColor = m_theme.colors[m_map.bg1].isValid() ? m_theme.colors[m_map.bg1] : QColor(Qt::white);
    if (const QDomElement style = findStyle(root); !style.isNull()) {
        const int n = int(parseInt(valOf(style), 2));
        c.style = n > 100 ? n - 100 : n;
        if (c.style < 1 || c.style > 48) c.style = 2;
    }
    c.font = readTxPr(cChild(root, "txPr"));
    {
        const QDomElement spPr = cChild(root, "spPr");
        bool none = false;
        readFill(spPr, &c.background, &none);
        c.border = readLine(spPr);
    }

    c.title = readTitle(cChild(chart, "title"));
    if (const QDomElement area = cChild(chart, "plotArea"); !area.isNull()) {
        readLayout(cChild(area, "layout"));
        for (QDomElement e = area.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!isChartNs(e.namespaceURI())) continue;
            const QString name = e.localName();
            if (name.endsWith(QLatin1String("Chart")) && c.plots.size() < kMaxPlots) readPlot(e);
            else if ((name == QLatin1String("catAx") || name == QLatin1String("valAx") || name == QLatin1String("dateAx") || name == QLatin1String("serAx")) && c.axes.size() < kMaxAxes)
                c.axes << readAxis(e);
        }
        const QDomElement spPr = cChild(area, "spPr");
        bool none = false;
        readFill(spPr, &c.plotFill, &none);
        c.plotBorder = readLine(spPr);
    }
    if (const QDomElement legend = cChild(chart, "legend"); !legend.isNull()) {
        c.legend.shown = true;
        c.legend.position = valOf(cChild(legend, "legendPos"), QStringLiteral("r"));
        c.legend.overlay = flagOf(cChild(legend, "overlay"), false);
        c.legend.font = readTxPr(cChild(legend, "txPr"));
    }
    // Numbers for categories (dates, say) are shown through the category axis's own number format when it is not linked to the source's.
    for (ChartPlot &plot : c.plots) {
        const ChartAxis *axis = c.axis(plot.axisIds[0]);
        if (!axis || axis->formatLinked || (axis->kind != ChartAxis::Category && axis->kind != ChartAxis::Date)) continue;
        const NumberFormat format = readFormat(axis->format);
        for (ChartSeries &s : plot.series) {
            if (s.categoryNumbers.isEmpty()) continue;
            s.categories.clear();
            for (double v : std::as_const(s.categoryNumbers)) s.categories << (std::isnan(v) ? QString() : formatWith(format, v).left(kChartMaxLabel));
        }
    }
    c.blanksAs = valOf(cChild(chart, "dispBlanksAs"), QStringLiteral("gap"));
    c.plotVisibleOnly = flagOf(cChild(chart, "plotVisOnly"), true);
    applyStyleColors();
    // A title with no text of its own is named for the series, or has the usual words.
    if (c.title.present && c.title.text.isEmpty()) {
        c.title.automatic = c.title.ref.isEmpty();
        int count = 0;
        QString only;
        for (const ChartPlot &p : std::as_const(c.plots))
            for (const ChartSeries &s : p.series) {
                ++count;
                only = s.name;
            }
        c.title.text = count == 1 ? only : say("Chart Title");
    }
    for (ChartAxis &a : c.axes)
        if (a.title.present && a.title.text.isEmpty()) a.title.text = say("Axis Title");
    return true;
}

} // namespace

// ---- frames

bool isChartFrame(const QDomElement &graphicFrame)
{
    if (graphicFrame.isNull() || graphicFrame.localName() != QLatin1String("graphicFrame")) return false;
    const QString uri = attr(aChild(aChild(graphicFrame, "graphic"), "graphicData"), QStringLiteral("uri"));
    return isChartNs(uri);
}

QString chartFrameRelId(const QDomElement &graphicFrame)
{
    if (!isChartFrame(graphicFrame)) return QString();
    for (QDomElement c = aChild(aChild(graphicFrame, "graphic"), "graphicData").firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String("chart") && isChartNs(c.namespaceURI())) return attrNs(c, ns::R, QStringLiteral("id"));
    return QString();
}

const ChartAxis *Chart::axis(qint64 id) const
{
    for (const ChartAxis &a : axes)
        if (a.id == id) return &a;
    return nullptr;
}

QString formatChartNumber(double value, const QString &formatCode)
{
    return formatNumber(value, formatCode);
}

bool readChart(const QByteArray &chartPartXml, const Theme &theme, const ColorMap &colors, Chart *out, QString *error)
{
    QDomDocument doc;
    QString why;
    if (!parseXml(chartPartXml, &doc, &why)) {
        if (error) *error = why;
        return false;
    }
    Chart chart;
    Reader reader(theme, colors, &chart);
    if (!reader.read(doc.documentElement(), error)) return false;
    *out = chart;
    return true;
}

} // namespace jp::office
