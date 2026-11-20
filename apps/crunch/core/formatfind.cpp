#include "core/formatfind.h"

#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QStringList>

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

bool formatMatches(const Styles &st, const CellFormat &format, const FormatChange &w)
{
    const FormatChange h = fullFormatChange(st, format);
    if (w.fontName && w.fontName->compare(*h.fontName, Qt::CaseInsensitive) != 0) return false;
    if (w.fontSize && std::abs(*w.fontSize - *h.fontSize) > 0.01) return false;
    if (w.bold && *w.bold != *h.bold) return false;
    if (w.italic && *w.italic != *h.italic) return false;
    if (w.strike && *w.strike != *h.strike) return false;
    if (w.underline && *w.underline != *h.underline) return false;
    if (w.vertAlign && *w.vertAlign != *h.vertAlign) return false;
    if (w.fontColor && !sameColor(st, *w.fontColor, *h.fontColor)) return false;
    if (w.fill && !sameFill(st, *w.fill, *h.fill)) return false;
    if (w.left && !sameSide(st, *w.left, *h.left)) return false;
    if (w.right && !sameSide(st, *w.right, *h.right)) return false;
    if (w.top && !sameSide(st, *w.top, *h.top)) return false;
    if (w.bottom && !sameSide(st, *w.bottom, *h.bottom)) return false;
    if (w.diagonal && !sameSide(st, *w.diagonal, *h.diagonal)) return false;
    if (w.numberFormat && *w.numberFormat != *h.numberFormat) return false;
    if (w.horizontal && *w.horizontal != *h.horizontal) return false;
    if (w.vertical && *w.vertical != *h.vertical) return false;
    if (w.wrap && *w.wrap != *h.wrap) return false;
    if (w.shrink && *w.shrink != *h.shrink) return false;
    if (w.indent && *w.indent != *h.indent) return false;
    if (w.rotation && *w.rotation != *h.rotation) return false;
    if (w.readingOrder && *w.readingOrder != *h.readingOrder) return false;
    if (w.locked && *w.locked != *h.locked) return false;
    if (w.formulaHidden && *w.formulaHidden != *h.formulaHidden) return false;
    return true;
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
