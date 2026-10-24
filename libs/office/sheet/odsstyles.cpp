#include "sheet/odsstyles.h"

#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet::ods {

bool lengthPoints(const QString &text, double *points)
{
    static const QRegularExpression re(QStringLiteral("^\\s*(-?(?:[0-9]+\\.?[0-9]*|\\.[0-9]+)(?:[eE][-+]?[0-9]+)?)\\s*(in|cm|mm|pt|pc|px)\\s*$"));
    const QRegularExpressionMatch m = re.match(text);
    if (!m.hasMatch()) return false;
    bool ok = false;
    const double v = m.captured(1).toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    const QString unit = m.captured(2);
    double factor = 1;
    if (unit == QLatin1String("in")) factor = 72;
    else if (unit == QLatin1String("cm")) factor = 72 / 2.54;
    else if (unit == QLatin1String("mm")) factor = 72 / 25.4;
    else if (unit == QLatin1String("pc")) factor = 12;
    else if (unit == QLatin1String("px")) factor = 0.75;
    *points = v * factor;
    return true;
}

bool colorOf(const QString &text, quint32 *rgb)
{
    const QString t = text.trimmed();
    if (t.size() != 7 || t[0] != QLatin1Char('#')) return false;
    bool ok = false;
    const quint32 v = t.mid(1).toUInt(&ok, 16);
    if (!ok) return false;
    *rgb = v;
    return true;
}

QString canonicalName(const QString &uri, const QString &local)
{
    static const QHash<QString, QString> known = {
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"), QStringLiteral("fo")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:style:1.0"), QStringLiteral("style")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"), QStringLiteral("svg")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"), QStringLiteral("number")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:table:1.0"), QStringLiteral("table")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0"), QStringLiteral("text")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0"), QStringLiteral("office")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"), QStringLiteral("draw")},
        {QStringLiteral("urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0"), QStringLiteral("calcext")},
        {QStringLiteral("http://openoffice.org/2009/table"), QStringLiteral("tableooo")},
        {QStringLiteral("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"), QStringLiteral("loext")},
    };
    const auto it = known.constFind(uri);
    return it == known.constEnd() ? local : it.value() + QLatin1Char(':') + local;
}

Props StyleStore::resolve(const QString &family, const QString &name) const
{
    QVector<const StyleDef *> chain;
    QSet<QString> seen;
    QString at = name;
    while (!at.isEmpty() && chain.size() < 32 && !seen.contains(at)) {
        seen.insert(at);
        const auto it = m_styles.constFind(family + QLatin1Char('\n') + at);
        if (it == m_styles.constEnd()) break;
        chain.append(&it.value());
        at = it->parent;
    }
    Props out = m_defaults.value(family);
    for (int i = int(chain.size()) - 1; i >= 0; --i) {
        const Props &p = chain[i]->props;
        // (a border written for all four sides replaces the sides a parent gave)
        if (p.contains(QStringLiteral("cell:fo:border"))) {
            for (const char *side : {"left", "right", "top", "bottom"}) out.remove(QStringLiteral("cell:fo:border-") + QLatin1String(side));
        }
        for (auto it = p.cbegin(); it != p.cend(); ++it) out.insert(it.key(), it.value());
    }
    return out;
}

QString StyleStore::dataStyleOf(const QString &family, const QString &name) const
{
    QSet<QString> seen;
    QString at = name;
    for (int depth = 0; !at.isEmpty() && depth < 32 && !seen.contains(at); ++depth) {
        seen.insert(at);
        const auto it = m_styles.constFind(family + QLatin1Char('\n') + at);
        if (it == m_styles.constEnd()) break;
        if (!it->dataStyle.isEmpty()) return it->dataStyle;
        at = it->parent;
    }
    return m_defaults.value(family).value(QStringLiteral("data-style"));
}

// ---- data styles

namespace {

QString zeros(int n) { return QString(std::max(0, std::min(n, 40)), QLatin1Char('0')); }

QString literal(const QString &text)
{
    if (text.isEmpty()) return QString();
    bool plain = true;
    for (const QChar c : text)
        if (!(c == QLatin1Char(' ') || c == QLatin1Char('-') || c == QLatin1Char('/') || c == QLatin1Char(':'))) plain = false;
    if (plain) return text;
    // a text in quotes (a quote in it is written with the backslash, outside the quotes)
    QString out;
    bool open = false;
    for (const QChar c : text) {
        if (c == QLatin1Char('"')) {
            if (open) out += QLatin1Char('"');
            open = false;
            out += QLatin1String("\\\"");
        } else {
            if (!open) out += QLatin1Char('"');
            open = true;
            out += c;
        }
    }
    if (open) out += QLatin1Char('"');
    return out;
}

// The digits before the point: `minInteger` of them are always shown, the leftmost `blank` of those as a space when they are zero ("?"), and with grouping a comma before every three.
QString integerDigits(int minInteger, bool grouping, int blank = 0)
{
    minInteger = std::max(0, std::min(minInteger, 30));
    blank = std::max(0, std::min(blank, minInteger));
    auto digits = [&](int n) {
        QString d;
        for (int i = 0; i < n; ++i) d += i < blank ? QLatin1Char('?') : QLatin1Char('0');
        return d;
    };
    if (!grouping) return minInteger == 0 ? QStringLiteral("#") : digits(minInteger);
    if (minInteger <= 3) return QStringLiteral("#,") + QString(3 - minInteger, QLatin1Char('#')) + digits(minInteger);
    const QString d = digits(minInteger);
    QString out;
    for (int i = 0; i < d.size(); ++i) {
        if (i > 0 && (d.size() - i) % 3 == 0) out += QLatin1Char(',');
        out += d[i];
    }
    return out;
}

const char *colorName(quint32 rgb)
{
    switch (rgb & 0xFFFFFF) {
    case 0xFF0000: return "Red";
    case 0x0000FF: return "Blue";
    case 0x00FF00: return "Green";
    case 0xFFFF00: return "Yellow";
    case 0xFF00FF: return "Magenta";
    case 0x00FFFF: return "Cyan";
    case 0x000000: return "Black";
    case 0xFFFFFF: return "White";
    default: return nullptr;
    }
}

// "value()>=0" as the condition of a section of a format code ("[>=0]"); empty for a condition that is not a comparison with a number.
QString sectionCondition(QString condition)
{
    condition.remove(QLatin1Char(' '));
    static const QRegularExpression re(QStringLiteral("^value\\(\\)(>=|<=|<>|!=|>|<|=)(-?[0-9.]+(?:[eE][-+]?[0-9]+)?)$"));
    const QRegularExpressionMatch m = re.match(condition);
    if (!m.hasMatch()) return QString();
    QString op = m.captured(1);
    if (op == QLatin1String("!=")) op = QStringLiteral("<>");
    return QLatin1Char('[') + op + m.captured(2) + QLatin1Char(']');
}

} // namespace

QString StyleStore::dataStyleCode(const QString &name) const
{
    const auto cached = m_codes.constFind(name);
    if (cached != m_codes.constEnd()) return cached.value();
    const QString code = dataStyleCodeDepth(name, 0);
    m_codes.insert(name, code);
    return code;
}

QString StyleStore::dataStyleCodeDepth(const QString &name, int depth) const
{
    const auto it = m_data.constFind(name);
    if (it == m_data.constEnd() || depth > 4) return QStringLiteral("General");
    const DataStyle &ds = it.value();
    const bool percentage = ds.kind == QLatin1String("percentage");
    QString body;
    bool anyDate = false;
    bool minutesFirst = true;   // a time that is elapsed shows its first unit as [h], [mm], or [ss]
    for (const DataPart &p : ds.parts) {
        switch (p.kind) {
        case DataPart::Number: {
            if (!p.hasDecimals && !p.grouping && p.maxBlank == 0 && p.minInteger <= 1 && p.displayFactor == 1) {
                body += QStringLiteral("General");   // (the digits that are needed, which is what a cell with no format of its own shows)
                break;
            }
            QString digits = integerDigits(p.minInteger, p.grouping, p.maxBlank);
            const int places = p.hasDecimals ? p.decimals : 2;
            if (places > 0) {
                const int min = p.hasDecimals ? (p.minDecimals >= 0 ? std::min(p.minDecimals, places) : places) : 2;
                digits += QLatin1Char('.') + zeros(min) + QString(std::max(0, std::min(places, 40) - min), QLatin1Char('#'));
            }
            for (int f = p.displayFactor; f >= 1000 && f % 1000 == 0; f /= 1000) digits += QLatin1Char(',');
            body += digits;
            break;
        }
        case DataPart::Scientific: {
            QString digits = integerDigits(p.minInteger, false);
            if (p.decimals > 0) digits += QLatin1Char('.') + zeros(p.decimals);
            body += digits + (p.forcedExponentSign ? QStringLiteral("E+") : QStringLiteral("E-")) + zeros(std::max(1, p.minExponent));
            break;
        }
        case DataPart::Fraction: {
            const QString whole = p.minInteger > 0 ? QStringLiteral("# ") : QString();
            const QString numerator = QString(std::max(1, std::min(p.minNumerator, 9)), QLatin1Char('?'));
            const QString denominator = p.denominatorValue > 0 ? QString::number(p.denominatorValue) : QString(std::max(1, std::min(p.minDenominator, 9)), QLatin1Char('?'));
            body += whole + numerator + QLatin1Char('/') + denominator;
            break;
        }
        case DataPart::Text: {
            QString text = p.text;
            // a space as wide as another character stands for the underscore and that character ("_("), the first space of the text
            if (!p.blankWidth.isNull()) {
                const int at = int(text.indexOf(QLatin1Char(' ')));
                if (at >= 0) {
                    body += literal(text.left(at)) + QLatin1Char('_') + p.blankWidth;
                    text = text.mid(at + 1);
                }
            }
            if (percentage && text == QLatin1String("%")) body += QLatin1Char('%');
            else body += literal(text);
            break;
        }
        case DataPart::FillCharacter:
            if (!p.text.isEmpty()) body += QLatin1Char('*') + p.text.left(1);
            break;
        case DataPart::CurrencySymbol: body += p.text.isEmpty() ? QString() : QLatin1Char('"') + QString(p.text).replace(QLatin1Char('"'), QString()) + QLatin1Char('"'); break;
        case DataPart::Day: body += p.longStyle ? QStringLiteral("dd") : QStringLiteral("d"); anyDate = true; break;
        case DataPart::Month:
            body += p.textual ? (p.longStyle ? QStringLiteral("mmmm") : QStringLiteral("mmm")) : (p.longStyle ? QStringLiteral("mm") : QStringLiteral("m"));
            anyDate = true;
            break;
        case DataPart::Year: body += p.longStyle ? QStringLiteral("yyyy") : QStringLiteral("yy"); anyDate = true; break;
        case DataPart::DayOfWeek: body += p.longStyle ? QStringLiteral("dddd") : QStringLiteral("ddd"); anyDate = true; break;
        case DataPart::Era: case DataPart::WeekOfYear: case DataPart::Quarter: break;
        case DataPart::Hours: {
            const QString h = p.longStyle ? QStringLiteral("hh") : QStringLiteral("h");
            body += !ds.truncateOnOverflow && minutesFirst ? QStringLiteral("[h]") : h;
            minutesFirst = false;
            break;
        }
        case DataPart::Minutes: {
            const QString m = p.longStyle ? QStringLiteral("mm") : QStringLiteral("m");
            body += !ds.truncateOnOverflow && minutesFirst ? QStringLiteral("[mm]") : m;
            minutesFirst = false;
            break;
        }
        case DataPart::Seconds: {
            const QString s = p.longStyle ? QStringLiteral("ss") : QStringLiteral("s");
            body += !ds.truncateOnOverflow && minutesFirst ? QStringLiteral("[ss]") : s;
            if (p.decimals > 0) body += QLatin1Char('.') + zeros(p.decimals);
            minutesFirst = false;
            break;
        }
        case DataPart::AmPm: body += QStringLiteral("AM/PM"); break;
        case DataPart::TextContent: body += QLatin1Char('@'); break;
        case DataPart::Boolean: break;
        }
    }
    Q_UNUSED(anyDate)
    if (ds.kind == QLatin1String("boolean")) return QStringLiteral("General");
    // (a style with nothing in it shows nothing: it is a section of a format that is left empty, such as the zeros of "0;-0;"; with no style to follow it is General)
    if (body.isEmpty() && (depth > 0 || ds.maps.isEmpty())) return depth > 0 ? QString() : QStringLiteral("General");
    QString code = body;
    if (ds.color != 0xFFFFFFFF && colorName(ds.color)) code = QLatin1Char('[') + QLatin1String(colorName(ds.color)) + QLatin1Char(']') + code;
    // the styles that take the values the conditions pick, each a section of its own: the usual three conditions (positive, negative, zero) are the format's own sections, in order, and
    // the style itself is the last (the text); otherwise a section has its condition written in front
    if (!ds.maps.isEmpty() && depth == 0) {
        struct Slot {
            QString condition, op;
            QString code;
        };
        QVector<Slot> slots;
        for (const DataStyle::Map &map : ds.maps) {
            QString condition = map.condition;
            condition.remove(QLatin1Char(' '));
            Slot slot;
            slot.condition = sectionCondition(map.condition);
            if (slot.condition.isEmpty()) continue;
            slot.op = condition.mid(int(qstrlen("value()")));
            slot.code = dataStyleCodeDepth(map.applyStyle, depth + 1);
            slots.append(slot);
        }
        auto find = [&](const char *op, QString *out) {
            for (const Slot &slot : std::as_const(slots))
                if (slot.op == QLatin1String(op)) {
                    *out = slot.code;
                    return true;
                }
            return false;
        };
        QString pos, neg, zero, posZero;
        const bool hasPos = find(">0", &pos), hasNeg = find("<0", &neg), hasZero = find("=0", &zero), hasPosZero = find(">=0", &posZero);
        const bool textMain = ds.kind == QLatin1String("text");
        if (slots.size() == 3 && hasPos && hasNeg && hasZero) return pos + QLatin1Char(';') + neg + QLatin1Char(';') + zero + (textMain ? QLatin1Char(';') + code : QString());
        if (slots.size() == 1 && hasPosZero && !textMain) return posZero + QLatin1Char(';') + code;
        if (slots.size() == 2 && hasPos && hasNeg && !textMain) return pos + QLatin1Char(';') + neg + QLatin1Char(';') + code;
        if (slots.size() == 2 && hasPos && hasZero && !textMain) return pos + QLatin1Char(';') + code + QLatin1Char(';') + zero;
        QStringList sections;
        for (const Slot &slot : std::as_const(slots)) sections << slot.condition + slot.code;
        sections << code;
        if (sections.size() > 1 && sections.size() <= 3) return sections.join(QLatin1Char(';'));
    }
    return code;
}

// ---- fonts, fills, borders, and cell formats

FontSpec StyleStore::fontOf(const Props &p) const
{
    FontSpec f;
    QString family = p.value(QStringLiteral("text:fo:font-family"));
    const QString faceName = p.value(QStringLiteral("text:style:font-name"));
    if (!faceName.isEmpty()) family = m_fonts.value(faceName, faceName);
    family = family.trimmed();
    if (family.startsWith(QLatin1Char('\'')) && family.endsWith(QLatin1Char('\'')) && family.size() >= 2) family = family.mid(1, family.size() - 2);
    if (family.contains(QLatin1Char(','))) family = family.section(QLatin1Char(','), 0, 0).trimmed();
    f.name = family.left(kMaxFontNameChars);
    double size = 0;
    if (lengthPoints(p.value(QStringLiteral("text:fo:font-size")), &size) && size > 0) f.size = std::min(size, 409.0);
    else f.size = 10;
    const QString weight = p.value(QStringLiteral("text:fo:font-weight"));
    f.bold = weight == QLatin1String("bold") || weight.toInt() >= 600;
    const QString style = p.value(QStringLiteral("text:fo:font-style"));
    f.italic = style == QLatin1String("italic") || style == QLatin1String("oblique");
    const QString underline = p.value(QStringLiteral("text:style:text-underline-style"));
    if (!underline.isEmpty() && underline != QLatin1String("none")) {
        const QString type = p.value(QStringLiteral("text:style:text-underline-type"));
        f.underline = type == QLatin1String("double") ? Underline::Double : Underline::Single;
    }
    const QString strike = p.value(QStringLiteral("text:style:text-line-through-style"));
    f.strike = !strike.isEmpty() && strike != QLatin1String("none");
    const QString position = p.value(QStringLiteral("text:style:text-position")).trimmed();
    if (position.startsWith(QLatin1String("super"))) f.vertAlign = 1;
    else if (position.startsWith(QLatin1String("sub"))) f.vertAlign = 2;
    quint32 rgb = 0;
    if (colorOf(p.value(QStringLiteral("text:fo:color")), &rgb)) {
        f.color.kind = ColorSpec::Kind::Rgb;
        f.color.rgb = rgb;
    }
    return f;
}

namespace {

BorderSide borderSideOf(const QString &spec)
{
    BorderSide side;
    const QStringList tokens = spec.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (tokens.isEmpty() || tokens[0] == QLatin1String("none") || tokens[0] == QLatin1String("hidden")) return side;
    double width = 0.74;
    QString style = QStringLiteral("solid");
    quint32 rgb = 0;
    bool haveColor = false;
    for (const QString &t : tokens) {
        double w = 0;
        if (lengthPoints(t, &w)) width = w;
        else if (colorOf(t, &rgb)) haveColor = true;
        else style = t;
    }
    if (style == QLatin1String("none") || style == QLatin1String("hidden")) return side;
    if (style == QLatin1String("double") || style == QLatin1String("double-thin")) side.style = BorderStyle::Double;
    else if (style == QLatin1String("dotted")) side.style = BorderStyle::Dotted;
    else if (style == QLatin1String("dashed")) side.style = width >= 1.2 ? BorderStyle::MediumDashed : BorderStyle::Dashed;
    else if (width < 0.3) side.style = BorderStyle::Hair;
    else if (width < 1.2) side.style = BorderStyle::Thin;
    else if (width < 2.2) side.style = BorderStyle::Medium;
    else side.style = BorderStyle::Thick;
    if (haveColor) {
        side.color.kind = ColorSpec::Kind::Rgb;
        side.color.rgb = rgb;
    }
    return side;
}

QString fontKey(const FontSpec &f)
{
    return QStringLiteral("%1|%2|%3%4%5|%6|%7|%8|%9").arg(f.name).arg(f.size).arg(int(f.bold)).arg(int(f.italic)).arg(int(f.strike)).arg(int(f.underline)).arg(f.vertAlign)
        .arg(int(f.color.kind)).arg(f.color.rgb);
}

QString sideKey(const BorderSide &s) { return QStringLiteral("%1/%2/%3").arg(int(s.style)).arg(int(s.color.kind)).arg(s.color.rgb); }

QString borderKey(const BorderSpec &b)
{
    return sideKey(b.left) + QLatin1Char('|') + sideKey(b.right) + QLatin1Char('|') + sideKey(b.top) + QLatin1Char('|') + sideKey(b.bottom) + QLatin1Char('|') + sideKey(b.diagonal)
           + QStringLiteral("|%1%2").arg(int(b.diagonalUp)).arg(int(b.diagonalDown));
}

struct Builder {
    Styles *styles;
    QHash<QString, qint32> fonts, fills, borders, codes;
    QHash<QString, quint32> formats;
    int nextId = 164;

    quint32 fontIndex(const FontSpec &f)
    {
        const QString key = fontKey(f);
        const auto it = fonts.constFind(key);
        if (it != fonts.constEnd()) return quint32(it.value());
        styles->fonts.append(f);
        fonts.insert(key, qint32(styles->fonts.size() - 1));
        return quint32(styles->fonts.size() - 1);
    }
    quint32 fillIndex(const FillSpec &f)
    {
        if (f.pattern == PatternKind::None) return 0;
        const QString key = QStringLiteral("%1|%2").arg(int(f.pattern)).arg(f.fg.rgb);
        const auto it = fills.constFind(key);
        if (it != fills.constEnd()) return quint32(it.value());
        styles->fills.append(f);
        fills.insert(key, qint32(styles->fills.size() - 1));
        return quint32(styles->fills.size() - 1);
    }
    quint32 borderIndex(const BorderSpec &b)
    {
        const QString key = borderKey(b);
        const auto it = borders.constFind(key);
        if (it != borders.constEnd()) return quint32(it.value());
        styles->borders.append(b);
        borders.insert(key, qint32(styles->borders.size() - 1));
        return quint32(styles->borders.size() - 1);
    }
    int numberFormatId(const QString &code)
    {
        if (code.isEmpty() || code == QLatin1String("General")) return 0;
        const auto it = codes.constFind(code);
        if (it != codes.constEnd()) return it.value();
        int id = -1;
        for (int b = 1; b <= 58; ++b)
            if (jp::calc::builtinFormatCode(b) == code) {
                id = b;
                break;
            }
        if (id < 0) {
            id = nextId++;
            styles->numFmts.insert(id, code);
            styles->chars += code.size();
        }
        codes.insert(code, id);
        return id;
    }
    quint32 formatIndex(const CellFormat &f)
    {
        const QString key = QStringLiteral("%1|%2|%3|%4|%5|%6|%7%8|%9|%10|%11").arg(f.numFmtId).arg(f.font).arg(f.fill).arg(f.border).arg(int(f.horizontal)).arg(int(f.vertical))
                                .arg(int(f.wrap)).arg(int(f.shrink)).arg(f.indent).arg(f.rotation).arg(int(f.locked));
        const auto it = formats.constFind(key);
        if (it != formats.constEnd()) return it.value();
        styles->formats.append(f);
        styles->cellXfs.append(f.numFmtId);
        formats.insert(key, quint32(styles->formats.size() - 1));
        return quint32(styles->formats.size() - 1);
    }
};

} // namespace

bool StyleStore::buildCellFormats(const Limits &limits, Styles *styles, QHash<QString, quint32> *xfOf, QString *error) const
{
    Builder b;
    b.styles = styles;
    // the fonts, fills, and borders every workbook starts with
    FillSpec none;
    FillSpec gray;
    gray.pattern = PatternKind::Gray125;
    styles->fills = {none, gray};
    styles->borders = {BorderSpec()};
    styles->fonts.clear();
    styles->formats.clear();
    styles->cellXfs.clear();
    auto makeFormat = [&](const QString &styleName) -> quint32 {
        const Props p = resolve(QStringLiteral("table-cell"), styleName);
        const QString dataName = dataStyleOf(QStringLiteral("table-cell"), styleName);
        CellFormat f;
        f.numFmtId = b.numberFormatId(dataName.isEmpty() ? QString() : dataStyleCode(dataName));
        f.font = b.fontIndex(fontOf(p));
        // fill
        FillSpec fill;
        quint32 rgb = 0;
        if (colorOf(p.value(QStringLiteral("cell:fo:background-color")), &rgb)) {
            fill.pattern = PatternKind::Solid;
            fill.fg.kind = ColorSpec::Kind::Rgb;
            fill.fg.rgb = rgb;
        }
        f.fill = b.fillIndex(fill);
        // border
        BorderSpec border;
        const QString all = p.value(QStringLiteral("cell:fo:border"));
        auto side = [&](const char *name) {
            const QString key = QStringLiteral("cell:fo:border-") + QLatin1String(name);
            return borderSideOf(p.contains(key) ? p.value(key) : all);
        };
        border.left = side("left");
        border.right = side("right");
        border.top = side("top");
        border.bottom = side("bottom");
        const BorderSide up = borderSideOf(p.value(QStringLiteral("cell:style:diagonal-bl-tr")));
        const BorderSide down = borderSideOf(p.value(QStringLiteral("cell:style:diagonal-tl-br")));
        if (up.style != BorderStyle::None) {
            border.diagonal = up;
            border.diagonalUp = true;
        }
        if (down.style != BorderStyle::None) {
            if (border.diagonal.style == BorderStyle::None) border.diagonal = down;
            border.diagonalDown = true;
        }
        f.border = b.borderIndex(border);
        // alignment
        const QString align = p.value(QStringLiteral("para:fo:text-align"));
        if (p.value(QStringLiteral("cell:style:text-align-source")) != QLatin1String("value-type")) {
            if (align == QLatin1String("left") || align == QLatin1String("start")) f.horizontal = HAlign::Left;
            else if (align == QLatin1String("right") || align == QLatin1String("end")) f.horizontal = HAlign::Right;
            else if (align == QLatin1String("center")) f.horizontal = HAlign::Center;
            else if (align == QLatin1String("justify")) f.horizontal = HAlign::Justify;
        }
        const QString vertical = p.value(QStringLiteral("cell:style:vertical-align"));
        if (vertical == QLatin1String("top")) f.vertical = VAlign::Top;
        else if (vertical == QLatin1String("middle")) f.vertical = VAlign::Center;
        f.wrap = p.value(QStringLiteral("cell:fo:wrap-option")) == QLatin1String("wrap");
        f.shrink = p.value(QStringLiteral("cell:style:shrink-to-fit")) == QLatin1String("true");
        double margin = 0;
        if (lengthPoints(p.value(QStringLiteral("para:fo:margin-left")), &margin) && margin > 0) f.indent = quint8(std::min(15.0, std::round(margin / 10.0)));
        bool ok = false;
        const int angle = p.value(QStringLiteral("cell:style:rotation-angle")).toInt(&ok);
        if (p.value(QStringLiteral("cell:style:direction")) == QLatin1String("ttb")) f.rotation = 255;
        else if (ok && angle > 0 && angle <= 90) f.rotation = quint8(angle);
        else if (ok && angle >= 270 && angle < 360) f.rotation = quint8(90 + (360 - angle));
        f.locked = !p.contains(QStringLiteral("cell:style:cell-protect")) || p.value(QStringLiteral("cell:style:cell-protect")).contains(QLatin1String("protected"));
        return b.formatIndex(f);
    };
    // the default look first: the document's default style, or the style named Default that cells with no style of their own follow
    const QString base = hasStyle(QStringLiteral("table-cell"), QStringLiteral("Default")) ? QStringLiteral("Default") : QString();
    const quint32 first = makeFormat(base);
    xfOf->insert(QString(), first);
    QStringList names;
    for (auto it = m_styles.cbegin(); it != m_styles.cend(); ++it)
        if (it->family == QLatin1String("table-cell")) names << it->name;
    std::sort(names.begin(), names.end());
    qint64 made = 0;
    for (const QString &name : std::as_const(names)) {
        // (the person can stop the read of a document of many styles)
        if ((++made & 255) == 0 && limits.cancelled && limits.cancelled()) {
            if (error) *error = QCoreApplication::translate("jp::office::sheet::Ods", "The read was stopped.");
            return false;
        }
        if (styles->formats.size() > limits.cellXfs) {
            if (error) *error = QCoreApplication::translate("jp::office::sheet::Ods", "The file has more cell formats than any real file has.");
            return false;
        }
        xfOf->insert(name, makeFormat(name));
    }
    if (styles->fonts.isEmpty()) styles->fonts.append(FontSpec());
    return true;
}

} // namespace jp::office::sheet::ods
