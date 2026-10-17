#include "office/drawingml_styleref.h"

#include "office/drawingml_style.h"
#include "office/xml.h"

namespace jp::office {

namespace {

// The color a reference holds: a theme color or an RGB one, with the changes made to it. False for another kind, or one the standard has no name for.
bool readColorOf(const QDomElement &ref, StyleColor *out)
{
    const QDomElement c = ref.firstChildElement();
    if (c.isNull()) return true;
    const QString kind = c.localName();
    if (kind == QLatin1String("schemeClr")) out->name = attr(c, QStringLiteral("val"));
    else if (kind == QLatin1String("srgbClr")) out->name = QLatin1Char('#') + attr(c, QStringLiteral("val")).toUpper();
    else return false;
    if (!isStyleColorName(out->name)) {
        out->name.clear();
        return false;
    }
    for (QDomElement m = c.firstChildElement(); !m.isNull(); m = m.nextSiblingElement()) {
        bool ok = false;
        const qint64 v = parseInt(attr(m, QStringLiteral("val")), 0, &ok);
        if (!ok || out->mods.size() >= 16 || !isStyleColorChange(m.localName())) return false;
        out->mods.append({m.localName(), int(std::clamp<qint64>(v, -10000000, 10000000))});
    }
    return true;
}

bool readPart(const QDomElement &ref, StyleRef::Part *out)
{
    out->idx = int(std::clamp<qint64>(parseInt(attr(ref, QStringLiteral("idx")), 0), 0, 1000000));
    return readColorOf(ref, &out->color);
}

void writeColorOf(QXmlStreamWriter &w, const StyleColor &c)
{
    if (c.name.isEmpty()) return;
    const bool rgb = c.name.startsWith(QLatin1Char('#'));
    w.writeStartElement(QLatin1String(rgb ? "a:srgbClr" : "a:schemeClr"));
    w.writeAttribute(QStringLiteral("val"), rgb ? c.name.mid(1) : c.name);
    for (const auto &m : c.mods) {
        w.writeStartElement(QStringLiteral("a:") + m.first);
        w.writeAttribute(QStringLiteral("val"), QString::number(m.second));
        w.writeEndElement();
    }
    w.writeEndElement();
}

void writePart(QXmlStreamWriter &w, const char *tag, const StyleRef::Part &p)
{
    w.writeStartElement(QLatin1String(tag));
    w.writeAttribute(QStringLiteral("idx"), QString::number(p.idx));
    writeColorOf(w, p.color);
    w.writeEndElement();
}

} // namespace

StyleRef readStyleRef(const QDomElement &style)
{
    StyleRef out;
    if (style.isNull()) return out;
    const QDomElement ln = childNamed(style, "lnRef"), fill = childNamed(style, "fillRef"), fx = childNamed(style, "effectRef"), font = childNamed(style, "fontRef");
    if (ln.isNull() || fill.isNull() || fx.isNull() || font.isNull()) return out;
    StyleRef r;
    if (!readPart(ln, &r.line) || !readPart(fill, &r.fill) || !readPart(fx, &r.effect) || !readColorOf(font, &r.fontColor)) return out;
    const QString face = attr(font, QStringLiteral("idx"));
    r.font = face == QLatin1String("major") || face == QLatin1String("minor") ? face : QString();
    r.on = true;
    return r;
}

void writeStyleRef(QXmlStreamWriter &w, const StyleRef &ref)
{
    w.writeStartElement(QStringLiteral("p:style"));
    writePart(w, "a:lnRef", ref.line);
    writePart(w, "a:fillRef", ref.fill);
    writePart(w, "a:effectRef", ref.effect);
    w.writeStartElement(QStringLiteral("a:fontRef"));
    w.writeAttribute(QStringLiteral("idx"), ref.font.isEmpty() ? QStringLiteral("none") : ref.font);
    writeColorOf(w, ref.fontColor);
    w.writeEndElement();
    w.writeEndElement();
}

StyleResolver::StyleResolver(const QByteArray &themeXml, const ColorMap &map)
{
    Theme theme;
    if (!readTheme(themeXml, &theme) || !parseXml(themeXml, &m_dom)) return;
    m_ctx.theme = theme;
    m_ctx.colors = map;
    m_ctx.formats = childNamed(childNamed(m_dom.documentElement(), "themeElements"), "fmtScheme");
    m_valid = true;
}

namespace {

// A reference as an element of a document of its own, which the resolving reads as it reads a file's.
QDomElement elementOf(const StyleRef &ref, QDomDocument *dom)
{
    QByteArray bytes;
    QXmlStreamWriter w(&bytes);
    w.writeStartElement(QStringLiteral("holder"));
    w.writeNamespace(ns::P, QStringLiteral("p"));
    w.writeNamespace(ns::A, QStringLiteral("a"));
    writeStyleRef(w, ref);
    w.writeEndElement();
    if (!parseXml(bytes, dom)) return QDomElement();
    return childNamed(dom->documentElement(), "style");
}

} // namespace

StyleLook StyleResolver::look(const StyleRef &ref, const QSizeF &frame) const
{
    StyleLook out;
    if (!m_valid || !ref.on) return out;
    QDomDocument dom;
    const QDomElement style = elementOf(ref, &dom);
    if (style.isNull()) return out;
    StyleContext sc;
    sc.ctx = &m_ctx;
    sc.frame = frame;
    out.fill = resolveFill({}, style, sc);
    out.stroke = resolveLine({}, style, sc);
    out.fx = resolveEffects({}, style, sc);
    const QDomElement font = childNamed(style, "fontRef");
    out.text = readColor(font, m_ctx.theme, m_ctx.colors);
    return out;
}

QColor StyleResolver::color(const StyleColor &c) const
{
    if (!m_valid || c.name.isEmpty()) return QColor();
    StyleRef ref;
    ref.on = true;
    ref.fontColor = c;
    QDomDocument dom;
    const QDomElement style = elementOf(ref, &dom);
    return style.isNull() ? QColor() : readColor(childNamed(style, "fontRef"), m_ctx.theme, m_ctx.colors);
}

} // namespace jp::office
