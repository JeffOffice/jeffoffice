#include "core/formatfind.h"

#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QStringList>

#include <cmath>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

bool sameColor(const Styles &st, const ColorSpec &a, const ColorSpec &b)
{
    const bool autoA = !a.isSet() || a.kind == ColorSpec::Kind::Auto, autoB = !b.isSet() || b.kind == ColorSpec::Kind::Auto;
    if (autoA || autoB) return autoA == autoB;
    return (st.resolve(a, 0) & 0xFFFFFF) == (st.resolve(b, 0) & 0xFFFFFF);
}

bool sameSide(const Styles &st, const BorderSide &a, const BorderSide &b)
{
    if (a.style != b.style) return false;
    return a.style == BorderStyle::None || sameColor(st, a.color, b.color);
}

bool sameFill(const Styles &st, const FillSpec &a, const FillSpec &b)
{
    if (a.gradient || b.gradient) return a == b;
    if (a.pattern != b.pattern) return false;
    if (a.pattern == PatternKind::None) return true;
    return sameColor(st, a.fg, b.fg) && (a.pattern == PatternKind::Solid || sameColor(st, a.bg, b.bg));
}

} // namespace

FormatChange fullFormatChange(const Styles &st, quint32 xf) { return fullFormatChange(st, st.format(xf)); }

bool formatMatches(const Styles &st, quint32 xf, const FormatChange &w) { return formatMatches(st, st.format(xf), w); }

FormatChange fullFormatChange(const Styles &st, const CellFormat &f)
{
    const FontSpec &font = st.font(f.font);
    FormatChange c;
    c.fontName = st.fontNameOf(font);
    c.fontSize = font.size;
    c.bold = font.bold;
    c.italic = font.italic;
    c.strike = font.strike;
    c.underline = font.underline;
    c.vertAlign = font.vertAlign;
    c.fontColor = font.color;
    c.fontFamily = font.family;
    c.fontScheme = font.scheme;
    c.fill = st.fill(f.fill);
    const BorderSpec &b = st.border(f.border);
    c.left = b.left;
    c.right = b.right;
    c.top = b.top;
    c.bottom = b.bottom;
    c.diagonal = b.diagonal;
    {
        const auto it = st.numFmts.constFind(f.numFmtId);
        QString code = it != st.numFmts.constEnd() ? it.value() : jp::calc::builtinFormatCode(f.numFmtId, QLocale::c());
        c.numberFormat = code.isEmpty() ? QStringLiteral("General") : code;
    }
    c.horizontal = f.horizontal;
    c.vertical = f.vertical;
    c.wrap = f.wrap;
    c.shrink = f.shrink;
    c.indent = f.indent;
    c.rotation = f.rotation;
    c.readingOrder = f.readingOrder;
    c.locked = f.locked;
    c.formulaHidden = f.formulaHidden;
    return c;
}

namespace {

// The fields of `mask` that are set in both looks and in which the looks agree (`alike`), or in which they differ (not `alike`): each with the value `mask` has. A color is compared as it shows
// in this workbook, a font name without regard to capitals. The font's family and scheme are carried with a look and are not part of how it looks, so they are not compared.
FormatChange selectFields(const Styles &st, const FormatChange &a, const FormatChange &b, const FormatChange &mask, bool alike)
{
    FormatChange out;
    auto pick = [&](auto member, auto same) {
        if ((mask.*member) && (a.*member) && (b.*member) && same(*(a.*member), *(b.*member)) == alike) out.*member = mask.*member;
    };
    auto equal = [](const auto &x, const auto &y) { return x == y; };
    pick(&FormatChange::fontName, [](const QString &x, const QString &y) { return x.compare(y, Qt::CaseInsensitive) == 0; });
    pick(&FormatChange::fontSize, [](double x, double y) { return std::abs(x - y) <= 0.01; });
    pick(&FormatChange::bold, equal);
    pick(&FormatChange::italic, equal);
    pick(&FormatChange::strike, equal);
    pick(&FormatChange::underline, equal);
    pick(&FormatChange::vertAlign, equal);
    pick(&FormatChange::fontColor, [&](const ColorSpec &x, const ColorSpec &y) { return sameColor(st, x, y); });
    pick(&FormatChange::fill, [&](const FillSpec &x, const FillSpec &y) { return sameFill(st, x, y); });
    for (auto side : {&FormatChange::left, &FormatChange::right, &FormatChange::top, &FormatChange::bottom, &FormatChange::diagonal})
        pick(side, [&](const BorderSide &x, const BorderSide &y) { return sameSide(st, x, y); });
    pick(&FormatChange::diagonalUp, equal);
    pick(&FormatChange::diagonalDown, equal);
    pick(&FormatChange::numberFormat, equal);
    pick(&FormatChange::horizontal, equal);
    pick(&FormatChange::vertical, equal);
    pick(&FormatChange::wrap, equal);
    pick(&FormatChange::shrink, equal);
    pick(&FormatChange::indent, equal);
    pick(&FormatChange::rotation, equal);
    pick(&FormatChange::readingOrder, equal);
    pick(&FormatChange::locked, equal);
    pick(&FormatChange::formulaHidden, equal);
    // (the family and scheme of a font go with its name)
    if (out.fontName) {
        out.fontFamily = mask.fontFamily;
        out.fontScheme = mask.fontScheme;
    }
    return out;
}

int fieldCount(const FormatChange &c)
{
    int n = 0;
    for (bool set : {c.fontName.has_value(), c.fontSize.has_value(), c.bold.has_value(), c.italic.has_value(), c.strike.has_value(), c.underline.has_value(), c.vertAlign.has_value(),
                     c.fontColor.has_value(), c.fill.has_value(), c.left.has_value(), c.right.has_value(), c.top.has_value(), c.bottom.has_value(), c.diagonal.has_value(),
                     c.diagonalUp.has_value(), c.diagonalDown.has_value(), c.numberFormat.has_value(), c.horizontal.has_value(), c.vertical.has_value(), c.wrap.has_value(),
                     c.shrink.has_value(), c.indent.has_value(), c.rotation.has_value(), c.readingOrder.has_value(), c.locked.has_value(), c.formulaHidden.has_value()})
        n += set ? 1 : 0;
    return n;
}

} // namespace

bool formatMatches(const Styles &st, const CellFormat &format, const FormatChange &w)
{
    FormatChange have = fullFormatChange(st, format);
    // (a field the format does not hold at all, a diagonal's direction for one, is not looked at)
    FormatChange mask = w;
    mask.diagonalUp.reset();
    mask.diagonalDown.reset();
    mask.fontFamily.reset();
    mask.fontScheme.reset();
    return fieldCount(selectFields(st, have, mask, mask, true)) == fieldCount(mask);
}

FormatChange inheritedFields(const Styles &st, const CellFormat &cell, const CellFormat &styleBefore, const FormatChange &change)
{
    FormatChange c = change;
    c.namedStyle.reset();
    const FormatChange have = fullFormatChange(st, cell), was = fullFormatChange(st, styleBefore);
    FormatChange out = selectFields(st, have, was, c, true);
    return out;
}

FormatChange changedFields(const Styles &st, const FormatChange &was, const FormatChange &now) { return selectFields(st, was, now, now, false); }

FormatChange changedFields(const Styles &st, const CellFormat &before, const CellFormat &after) { return changedFields(st, fullFormatChange(st, before), fullFormatChange(st, after)); }

FormatChange withColorsResolved(const Styles &source, FormatChange look)
{
    auto resolved = [&](ColorSpec &c) {
        if (c.kind == ColorSpec::Kind::Theme || c.kind == ColorSpec::Kind::Indexed) c = ColorSpec::fromRgb(source.resolve(c, 0));
    };
    if (look.fontColor) resolved(*look.fontColor);
    if (look.fill) {
        resolved(look.fill->fg);
        resolved(look.fill->bg);
        for (GradientStop &g : look.fill->stops) resolved(g.color);
    }
    for (auto side : {&FormatChange::left, &FormatChange::right, &FormatChange::top, &FormatChange::bottom, &FormatChange::diagonal})
        if (look.*side) resolved((look.*side)->color);
    if (look.fontScheme) look.fontScheme = QString();
    return look;
}

QString themeKeyOf(const Styles &styles)
{
    QString key;
    for (int i = 0; i < kThemeColors; ++i) key += QString::number(styles.theme[i] & 0xFFFFFF, 16) + QLatin1Char(',');
    return key + styles.majorFont + QLatin1Char('|') + styles.minorFont;
}

FormatMatcher::FormatMatcher(const Styles &styles, const FormatChange &want) : m_styles(styles), m_want(want) {}

bool FormatMatcher::matches(quint32 xf)
{
    const auto found = m_seen.find(xf);
    if (found != m_seen.end()) return found->second;
    ++m_resolutions;
    return m_seen[xf] = formatMatches(m_styles, m_styles.format(xf), m_want);
}

bool formatIsEmpty(const FormatChange &c)
{
    return !c.touchesFont() && !c.fill && !c.touchesBorder() && !c.numberFormat && !c.horizontal && !c.vertical && !c.wrap && !c.shrink && !c.indent && !c.rotation && !c.readingOrder
           && !c.locked && !c.formulaHidden;
}

QString describeFormat(const FormatChange &c)
{
    QStringList parts;
    if (c.numberFormat) parts << QCoreApplication::translate("jp::crunch::formatfind", "Number");
    if (c.horizontal || c.vertical || c.wrap || c.shrink || c.indent || c.rotation || c.readingOrder) parts << QCoreApplication::translate("jp::crunch::formatfind", "Alignment");
    if (c.touchesFont()) parts << QCoreApplication::translate("jp::crunch::formatfind", "Font");
    if (c.touchesBorder()) parts << QCoreApplication::translate("jp::crunch::formatfind", "Border");
    if (c.fill) parts << QCoreApplication::translate("jp::crunch::formatfind", "Fill");
    if (c.locked || c.formulaHidden) parts << QCoreApplication::translate("jp::crunch::formatfind", "Protection");
    return parts.join(QStringLiteral(", "));
}

} // namespace jp::crunch
