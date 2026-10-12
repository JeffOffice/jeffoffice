// Styles of an open document presentation (odpimport_p.h): the values of its
// attributes, the style elements and what they come to once their parents are
// taken in, and the fills, outlines, and shadows they describe.

#include "io/odpimport_p.h"

#include "office/drawingml_style.h"

#include <QImage>
#include <QRegularExpression>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp::deck::odp {

namespace {

constexpr int kMaxParents = 32;
const QLatin1String kXmlnsUri("http://www.w3.org/2000/xmlns/");

QString familyKey(const QString &family, const QString &name) { return family + QLatin1Char('|') + name; }

} // namespace

QString canonicalPrefix(const QString &uri)
{
    static const QHash<QString, QString> known = {
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:office:1.0"), QStringLiteral("office")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:style:1.0"), QStringLiteral("style")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0"), QStringLiteral("text")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"), QStringLiteral("draw")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:table:1.0"), QStringLiteral("table")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"), QStringLiteral("presentation")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"), QStringLiteral("fo")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"), QStringLiteral("svg")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0"), QStringLiteral("smil")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:animation:1.0"), QStringLiteral("anim")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"), QStringLiteral("meta")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"), QStringLiteral("number")},
        {QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:chart:1.0"), QStringLiteral("chart")},
        {QStringLiteral("http://www.w3.org/1999/xlink"), QStringLiteral("xlink")},
        {QStringLiteral("http://purl.org/dc/elements/1.1/"), QStringLiteral("dc")},
        {QStringLiteral("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"), QStringLiteral("loext")},
        {QStringLiteral("http://openoffice.org/2010/draw"), QStringLiteral("drawooo")},
        {QStringLiteral("http://openoffice.org/2009/office"), QStringLiteral("officeooo")},
    };
    const auto it = known.constFind(uri);
    return it == known.cend() ? uri : it.value();
}

QString tagOf(const QDomElement &e)
{
    if (e.isNull()) return QString();
    return canonicalPrefix(e.namespaceURI()) + QLatin1Char(':') + e.localName();
}

Attrs::Attrs(const QDomElement &e)
{
    const QDomNamedNodeMap attributes = e.attributes();
    for (int i = 0; i < attributes.length(); ++i) {
        const QDomAttr a = attributes.item(i).toAttr();
        const QString uri = a.namespaceURI();
        if (uri == kXmlnsUri || a.name() == QLatin1String("xmlns") || a.name().startsWith(QLatin1String("xmlns:"))) continue;
        m_values.insert(uri.isEmpty() ? a.name() : canonicalPrefix(uri) + QLatin1Char(':') + a.localName(), a.value());
    }
}

QDomElement kid(const QDomElement &e, const char *name)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (tagOf(c) == QLatin1String(name)) return c;
    return QDomElement();
}

QVector<QDomElement> kids(const QDomElement &e, const char *name)
{
    QVector<QDomElement> out;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (tagOf(c) == QLatin1String(name)) out << c;
    return out;
}

QString at(const QDomElement &e, const char *name, const QString &fallback)
{
    const QString key = QString::fromLatin1(name);
    const int colon = key.indexOf(QLatin1Char(':'));
    const QString prefix = key.left(colon), local = key.mid(colon + 1);
    const QDomNamedNodeMap attributes = e.attributes();
    for (int i = 0; i < attributes.length(); ++i) {
        const QDomAttr a = attributes.item(i).toAttr();
        if (a.localName() == local && canonicalPrefix(a.namespaceURI()) == prefix) return a.value();
    }
    return fallback;
}

// ---- values

bool parseNumber(const QString &s, double *value)
{
    bool ok = false;
    const double v = s.trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    *value = v;
    return true;
}

bool parseLength(const QString &s, double *points)
{
    static const struct { QLatin1String unit; double factor; } units[] = {
        {QLatin1String("cm"), 72 / 2.54}, {QLatin1String("mm"), 72 / 25.4}, {QLatin1String("in"), 72}, {QLatin1String("pt"), 1},
        {QLatin1String("pc"), 12}, {QLatin1String("px"), 0.75}, {QLatin1String("ft"), 864}, {QLatin1String("m"), 72 / 0.0254}};
    const QString t = s.trimmed();
    if (t.isEmpty() || t.endsWith(QLatin1Char('%'))) return false;
    for (const auto &u : units)
        if (t.endsWith(u.unit)) {
            double v = 0;
            if (!parseNumber(t.chopped(u.unit.size()), &v)) return false;
            *points = v * u.factor;
            return std::isfinite(*points);
        }
    return parseNumber(t, points);
}

double lengthOr(const QString &s, double fallback)
{
    double v = 0;
    return parseLength(s, &v) ? v : fallback;
}

bool parsePercent(const QString &s, double *fraction)
{
    const QString t = s.trimmed();
    if (!t.endsWith(QLatin1Char('%'))) return false;
    double v = 0;
    if (!parseNumber(t.chopped(1), &v)) return false;
    *fraction = v / 100;
    return true;
}

QColor parseColor(const QString &s)
{
    const QString t = s.trimmed();
    if (t.isEmpty() || t.compare(QLatin1String("transparent"), Qt::CaseInsensitive) == 0) return QColor();
    const QColor c(t);
    return c.isValid() ? QColor(c.red(), c.green(), c.blue()) : QColor();
}

bool parseDuration(const QString &s, double *seconds)
{
    static const QRegularExpression pattern(QStringLiteral("^P(?:([0-9.]+)D)?(?:T(?:([0-9.]+)H)?(?:([0-9.]+)M)?(?:([0-9.]+)S)?)?$"));
    const QRegularExpressionMatch m = pattern.match(s.trimmed());
    if (!m.hasMatch()) return false;
    double total = 0;
    const double factors[4] = {86400, 3600, 60, 1};
    bool any = false;
    for (int i = 0; i < 4; ++i) {
        double v = 0;
        if (m.captured(i + 1).isEmpty()) continue;
        if (!parseNumber(m.captured(i + 1), &v)) return false;
        total += v * factors[i];
        any = true;
    }
    if (!any) return false;
    *seconds = total;
    return true;
}

// ---- styles

void Bag::overlay(const Bag &top)
{
    for (int k = 0; k < KindCount; ++k)
        for (auto it = top.values[k].cbegin(); it != top.values[k].cend(); ++it) values[k].insert(it.key(), it.value());
    if (!top.tabStops.isNull()) tabStops = top.tabStops;
    if (!top.graphicListStyle.isNull()) graphicListStyle = top.graphicListStyle;
    if (!top.listStyle.isEmpty()) listStyle = top.listStyle;
}

StyleDef StyleSheet::makeStyle(const QDomElement &e) const
{
    StyleDef s;
    const Attrs a(e);
    s.name = a.get("style:name");
    s.family = a.get("style:family");
    s.parent = a.get("style:parent-style-name");
    s.listStyle = a.get("style:list-style-name");
    s.displayName = a.get("style:display-name");
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString t = tagOf(c);
        Kind kind = KindCount;
        if (t == QLatin1String("style:text-properties")) kind = Text;
        else if (t == QLatin1String("style:paragraph-properties")) kind = Para;
        else if (t == QLatin1String("style:graphic-properties") || t == QLatin1String("style:drawing-page-properties") || t == QLatin1String("style:chart-properties")) kind = Graphic;
        else if (t == QLatin1String("style:table-cell-properties") || t == QLatin1String("loext:graphic-properties")) kind = Cell;
        else if (t == QLatin1String("style:table-row-properties")) kind = Row;
        else if (t == QLatin1String("style:table-column-properties")) kind = Column;
        else if (t == QLatin1String("style:table-properties")) kind = Table;
        if (kind == KindCount) continue;
        const Attrs props(c);
        for (auto it = props.all().cbegin(); it != props.all().cend(); ++it) s.values[kind].insert(it.key(), it.value());
        if (kind == Para) s.tabStops = kid(c, "style:tab-stops");
        if (kind == Graphic) s.graphicListStyle = kid(c, "text:list-style");
    }
    return s;
}

void StyleSheet::read(const QDomElement &root, bool automatic)
{
    for (QDomElement c = root.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString t = tagOf(c);
        if (t == QLatin1String("style:style")) {
            StyleDef s = makeStyle(c);
            if (s.name.isEmpty()) continue;
            (automatic ? m_styles : m_common->styles).insert(familyKey(s.family, s.name), s);
        } else if (t == QLatin1String("style:default-style")) {
            StyleDef s = makeStyle(c);
            if (!s.family.isEmpty() && !automatic) m_common->defaults.insert(s.family, s);
        } else if (t == QLatin1String("text:list-style")) {
            (automatic ? m_lists : m_common->lists).insert(at(c, "style:name"), c);
        } else if (t == QLatin1String("style:page-layout")) {
            (automatic ? m_pageLayouts : m_common->pageLayouts).insert(at(c, "style:name"), c);
        } else if (t == QLatin1String("draw:gradient")) {
            (automatic ? m_gradients : m_common->gradients).insert(at(c, "draw:name"), c);
        } else if (t == QLatin1String("draw:hatch")) {
            (automatic ? m_hatches : m_common->hatches).insert(at(c, "draw:name"), c);
        } else if (t == QLatin1String("draw:fill-image")) {
            (automatic ? m_images : m_common->images).insert(at(c, "draw:name"), c);
        } else if (t == QLatin1String("draw:stroke-dash")) {
            (automatic ? m_dashes : m_common->dashes).insert(at(c, "draw:name"), c);
        } else if (t == QLatin1String("draw:marker")) {
            (automatic ? m_markers : m_common->markers).insert(at(c, "draw:name"), c);
        } else if (t == QLatin1String("draw:opacity")) {
            (automatic ? m_opacities : m_common->opacities).insert(at(c, "draw:name"), c);
        }
    }
}

void StyleSheet::readFonts(const QDomElement &decls)
{
    for (const QDomElement &f : kids(decls, "style:font-face")) {
        QString family = at(f, "svg:font-family");
        if (family.size() >= 2 && (family.startsWith(QLatin1Char('\'')) || family.startsWith(QLatin1Char('"'))) && family.endsWith(family[0])) family = family.mid(1, family.size() - 2);
        if (!family.isEmpty()) m_common->fonts.insert(at(f, "style:name"), family);
    }
}

QString StyleSheet::fontFamily(const QString &faceName) const
{
    const auto it = m_common->fonts.constFind(faceName);
    return it == m_common->fonts.cend() ? faceName : it.value();
}

QDomElement StyleSheet::list(const QString &name) const
{
    const auto it = m_lists.constFind(name);
    if (it != m_lists.cend()) return it.value();
    return m_common->lists.value(name);
}

const StyleDef *StyleSheet::find(const QString &family, const QString &name) const
{
    const QString key = familyKey(family, name);
    if (const auto it = m_styles.constFind(key); it != m_styles.cend()) return &it.value();
    if (const auto it = m_common->styles.constFind(key); it != m_common->styles.cend()) return &it.value();
    return nullptr;
}

void StyleSheet::merge(Bag &bag, const StyleDef &s) const
{
    for (int k = 0; k < KindCount; ++k)
        for (auto it = s.values[k].cbegin(); it != s.values[k].cend(); ++it) {
            // A size in percent is of the size it is based on.
            if (k == Text && it.key() == QLatin1String("fo:font-size") && it.value().endsWith(QLatin1Char('%'))) {
                double percent = 0, base = 0;
                if (parsePercent(it.value(), &percent) && parseLength(bag.values[Text].value(it.key()), &base)) {
                    bag.values[Text].insert(it.key(), QString::number(base * percent, 'f', 2) + QLatin1String("pt"));
                    continue;
                }
            }
            bag.values[k].insert(it.key(), it.value());
        }
    if (!s.tabStops.isNull()) bag.tabStops = s.tabStops;
    if (!s.graphicListStyle.isNull()) bag.graphicListStyle = s.graphicListStyle;
    if (!s.listStyle.isEmpty()) bag.listStyle = s.listStyle;
}

void StyleSheet::applyFrom(Bag &bag, const QString &family, const QString &name, int depth) const
{
    if (name.isEmpty() || depth > kMaxParents) return;
    const StyleDef *s = find(family, name);
    if (!s) return;
    if (!s->parent.isEmpty() && s->parent != name) applyFrom(bag, family, s->parent, depth + 1);
    merge(bag, *s);
}

void StyleSheet::apply(Bag &bag, const QString &family, const QString &name) const { applyFrom(bag, family, name, 0); }

void StyleSheet::applyDefaults(Bag &bag, const QString &family) const
{
    auto it = m_common->defaults.constFind(family);
    // A presentation's objects take what the graphic objects do.
    if (it == m_common->defaults.cend() && family == QLatin1String("presentation")) it = m_common->defaults.constFind(QStringLiteral("graphic"));
    if (it != m_common->defaults.cend()) merge(bag, it.value());
}

Bag StyleSheet::resolve(const QString &family, const QString &name) const
{
    Bag bag;
    applyDefaults(bag, family);
    apply(bag, family, name);
    return bag;
}

// ---- pictures, fills, outlines, and shadows

QString Reader::loadPicture(const QString &href, QString *format)
{
    if (href.isEmpty()) return QString();
    const QString part = office::resolvePartName(QStringLiteral("/"), href);
    // A picture outside the file (a link to an address or to a file beside it) is not followed.
    if (part.isEmpty() || href.contains(QLatin1String("://")) || !m_pkg.hasPart(part)) return QString();
    const QString key = part.toLower();
    if (const auto it = m_pictures.constFind(key); it != m_pictures.cend()) {
        if (format) *format = m_doc.images.value(it.value()).format;
        return it.value();
    }
    const QByteArray bytes = m_pkg.part(part);
    const QString kind = office::imageFormat(bytes);
    if (kind.isEmpty()) {
        m_pictures.insert(key, QString());
        return QString();
    }
    const QString id = m_doc.addImage(bytes, kind);
    m_pictures.insert(key, id);
    if (format) *format = kind;
    return id;
}

namespace {

// A color at a share of its brightness (the intensities of a gradient).
QColor intensity(const QColor &c, double fraction)
{
    if (!c.isValid() || fraction >= 0.999) return c;
    const double k = std::clamp(fraction, 0.0, 1.0);
    return QColor(qRound(c.red() * k), qRound(c.green() * k), qRound(c.blue() * k));
}

// A gradient's angle in degrees: "30deg" or tenths of a degree.
double gradientAngle(const QString &s)
{
    double v = 0;
    if (s.endsWith(QLatin1String("deg"))) return parseNumber(s.chopped(3), &v) ? v : 0;
    return parseNumber(s, &v) ? v / 10 : 0;
}

} // namespace

Fill Reader::gradientFill(const QDomElement &gradient, double transparency, double endTransparency, bool varyTransparency)
{
    const Attrs g(gradient);
    const QString style = g.get("draw:style", QStringLiteral("linear"));
    double border = 0, start = 1, end = 1;
    parsePercent(g.get("draw:border"), &border);
    parsePercent(g.get("draw:start-intensity", QStringLiteral("100%")), &start);
    parsePercent(g.get("draw:end-intensity", QStringLiteral("100%")), &end);
    QColor from = intensity(parseColor(g.get("draw:start-color")), start), to = intensity(parseColor(g.get("draw:end-color")), end);
    // The stops a newer file lists itself, from the start of the gradient to its end.
    QVector<QPair<double, QColor>> listed;
    for (const QDomElement &s : kids(gradient, "loext:gradient-stop")) {
        double offset = 0;
        const QColor c = parseColor(at(s, "loext:color-value"));
        if (listed.size() < 64 && parseNumber(at(s, "svg:offset"), &offset) && c.isValid()) listed.append({std::clamp(offset, 0.0, 1.0), c});
    }
    if (listed.size() >= 2) {
        from = listed.first().second;
        to = listed.last().second;
    }
    if (!from.isValid()) from = QColor(Qt::black);
    if (!to.isValid()) to = QColor(Qt::white);

    Fill f;
    f.type = Fill::Gradient;
    f.transparency = transparency;
    const bool linear = style == QLatin1String("linear"), axial = style == QLatin1String("axial");
    f.gradType = linear || axial ? Fill::Linear : style == QLatin1String("radial") ? Fill::Radial : style == QLatin1String("ellipsoid") ? Fill::PathGrad : Fill::Rectangular;
    f.angle = std::fmod(90 - gradientAngle(g.get("draw:angle")) + 720, 360.0);
    auto stop = [&](double pos, const QColor &c) {
        GradientStop s;
        s.pos = std::clamp(pos, 0.0, 1.0);
        s.color = ColorRef::rgb(c);
        // The opacity runs the way the colors do: from the first to the last, from the edges to the middle, or from the edge to the middle.
        auto mixed = [&](double a, double b, double k) { return a + (b - a) * std::clamp(k, 0.0, 1.0); };
        s.transparency = !varyTransparency ? transparency
                       : linear ? mixed(transparency, endTransparency, s.pos)
                       : axial ? (s.pos <= 0.5 ? mixed(endTransparency, transparency, s.pos * 2) : mixed(transparency, endTransparency, (s.pos - 0.5) * 2))
                       : mixed(endTransparency, transparency, s.pos);
        f.stops.append(s);
    };
    // The first color of the file is at the top of a linear gradient, at the middle of an axial one, and at the edge of the others;
    // the last is at the bottom, at the edges, and at the middle.
    if (linear) {
        if (listed.size() >= 2) {
            for (const auto &s : listed) stop(s.first, s.second);
        } else {
            stop(0, from);
            if (border > 0) stop(border, from);
            stop(1, to);
        }
    } else if (axial) {
        // The first color is at the middle, and the last at the edges.
        stop(0, to);
        if (border > 0) stop(border / 2, to);
        stop(0.5, from);
        if (border > 0) stop(1 - border / 2, to);
        stop(1, to);
    } else {
        // Centered: the middle is the last color, the edge the first.
        stop(0, to);
        if (border > 0) stop(1 - border, from);
        stop(1, from);
    }
    f.color = ColorRef::rgb(f.stops.first().color.rgbValue());
    f.color2 = ColorRef::rgb(f.stops.last().color.rgbValue());
    return f;
}

Fill Reader::fillOf(const Bag &bag, const StyleSheet &styles)
{
    const QString mode = bag.get(Graphic, "draw:fill");
    double opacity = 1;
    if (const QString o = bag.get(Graphic, "draw:opacity"); !o.isEmpty()) parsePercent(o, &opacity);
    double transparency = std::clamp(1 - opacity, 0.0, 1.0);

    // A fill that fades: the opacity named by a gradient of opacities (draw:opacity, from the percent of
    // opacity at its start to that at its end), or by a gray one (white is opaque, black is clear).
    double endTransparency = transparency;
    bool varies = false;
    QDomElement fade;
    if (const QString name = bag.get(Graphic, "draw:opacity-name"); !name.isEmpty()) {
        fade = styles.opacity(name);
        double first = 1, last = 1;
        if (!fade.isNull()) {
            parsePercent(at(fade, "draw:start", QStringLiteral("100%")), &first);
            parsePercent(at(fade, "draw:end", QStringLiteral("100%")), &last);
        } else if ((fade = styles.gradient(name)); !fade.isNull()) {
            auto opaque = [](const QColor &c, double intensityFraction) { return c.isValid() ? c.lightnessF() * intensityFraction : 1.0; };
            double si = 1, ei = 1;
            parsePercent(at(fade, "draw:start-intensity", QStringLiteral("100%")), &si);
            parsePercent(at(fade, "draw:end-intensity", QStringLiteral("100%")), &ei);
            first = opaque(parseColor(at(fade, "draw:start-color")), si);
            last = opaque(parseColor(at(fade, "draw:end-color")), ei);
        }
        if (!fade.isNull()) {
            transparency = std::clamp(1 - first * opacity, 0.0, 1.0);
            endTransparency = std::clamp(1 - last * opacity, 0.0, 1.0);
            varies = std::fabs(transparency - endTransparency) > 0.01;
            if (!varies) endTransparency = transparency;
        }
    }

    if (mode == QLatin1String("solid")) {
        const QColor c = parseColor(bag.get(Graphic, "draw:fill-color"));
        if (!c.isValid()) return Fill();
        if (varies) {
            // A solid color that fades is a gradient of that color.
            Fill f = gradientFill(fade, transparency, endTransparency, true);
            for (GradientStop &s : f.stops) s.color = ColorRef::rgb(c);
            f.color = f.color2 = ColorRef::rgb(c);
            return f;
        }
        return Fill::solid(ColorRef::rgb(c), transparency);
    }
    if (mode == QLatin1String("gradient")) {
        const QDomElement g = styles.gradient(bag.get(Graphic, "draw:fill-gradient-name"));
        if (g.isNull()) return Fill();
        // One color that fades takes the shape of the fade.
        if (varies && parseColor(at(g, "draw:start-color")) == parseColor(at(g, "draw:end-color"))) {
            Fill f = gradientFill(fade, transparency, endTransparency, true);
            const ColorRef c = ColorRef::rgb(parseColor(at(g, "draw:start-color")));
            for (GradientStop &st : f.stops) st.color = c;
            f.color = f.color2 = c;
            return f;
        }
        return gradientFill(g, transparency, endTransparency, varies);
    }
    if (mode == QLatin1String("bitmap")) {
        const QDomElement img = styles.fillImage(bag.get(Graphic, "draw:fill-image-name"));
        const QString id = img.isNull() ? QString() : loadPicture(at(img, "xlink:href"));
        if (id.isEmpty()) return Fill();
        Fill f;
        f.type = Fill::Picture;
        f.imageId = id;
        f.transparency = transparency;
        if (bag.get(Graphic, "style:repeat") == QLatin1String("repeat")) {
            f.type = Fill::Texture;
            f.tile = true;
            // A tile is its picture at the size the file gives it (pixels at 96 to the inch).
            double w = 0;
            const QSize px = m_doc.imageSize(id);
            if (parseLength(bag.get(Graphic, "draw:fill-image-width"), &w) && w > 0 && px.width() > 0) f.tileScale = std::clamp(w / (px.width() * 0.75), 0.05, 20.0);
        }
        return f;
    }
    if (mode == QLatin1String("hatch")) {
        const QDomElement h = styles.hatch(bag.get(Graphic, "draw:fill-hatch-name"));
        const QColor lines = h.isNull() ? QColor(Qt::black) : parseColor(at(h, "draw:color"));
        const QString kind = at(h, "draw:style", QStringLiteral("single"));
        double rotation = 0;
        const QString r = at(h, "draw:rotation");
        parseNumber(r.endsWith(QLatin1String("deg")) ? r.chopped(3) : r, &rotation);
        if (!r.endsWith(QLatin1String("deg"))) rotation /= 10;
        rotation = std::fmod(rotation + 360, 180.0);
        Qt::BrushStyle brush;
        if (kind != QLatin1String("single")) brush = rotation > 22.5 && rotation < 67.5 ? Qt::DiagCrossPattern : Qt::CrossPattern;
        else if (rotation < 22.5 || rotation >= 157.5) brush = Qt::HorPattern;
        else if (rotation < 67.5) brush = Qt::BDiagPattern;   // lines rising to the right
        else if (rotation < 112.5) brush = Qt::VerPattern;
        else brush = Qt::FDiagPattern;
        Fill f;
        f.type = Fill::Pattern;
        f.pattern = std::max(0, int(patternBrushes().indexOf(brush)));
        f.color = ColorRef::rgb(lines.isValid() ? lines : QColor(Qt::black));
        f.transparency = transparency;
        // The color behind the lines, when the hatch is drawn on a background.
        if (bag.get(Graphic, "draw:fill-hatch-solid") == QLatin1String("true")) {
            const QColor back = parseColor(bag.get(Graphic, "draw:fill-color"));
            if (back.isValid()) f.color2 = ColorRef::rgb(back);
        }
        return f;
    }
    return Fill();
}

namespace {

Arrow arrowOf(const QDomElement &marker)
{
    if (marker.isNull()) return Arrow::None;
    const QString name = at(marker, "draw:name").toLower();
    if (name.contains(QLatin1String("stealth")) || name.contains(QLatin1String("concave"))) return Arrow::Stealth;
    if (name.contains(QLatin1String("diamond"))) return Arrow::Diamond;
    if (name.contains(QLatin1String("oval")) || name.contains(QLatin1String("circle")) || name.contains(QLatin1String("round"))) return Arrow::Oval;
    if (name.contains(QLatin1String("open")) || name.contains(QLatin1String("line arrow")) || name.contains(QLatin1String("line_20_arrow"))) return Arrow::Open;
    if (name.contains(QLatin1String("arrow")) || name.contains(QLatin1String("triangle"))) return Arrow::Triangle;
    // Another name: the shape of its path, a curve for a round one.
    const QString d = at(marker, "svg:d");
    if (d.contains(QLatin1Char('c')) || d.contains(QLatin1Char('C')) || d.contains(QLatin1Char('a')) || d.contains(QLatin1Char('A'))) return Arrow::Oval;
    return Arrow::Triangle;
}

int arrowSize(double markerWidth, double strokeWidth)
{
    const double ratio = markerWidth / std::max(strokeWidth, 1.0);
    return ratio < 3 ? 0 : ratio < 6 ? 1 : 2;
}

// A dash pattern of the file as the nearest of the library's.
Stroke::Dash dashOf(const QDomElement &dash, double width)
{
    if (dash.isNull()) return Stroke::DashLine;
    const Attrs a(dash);
    const bool round = a.get("draw:style") == QLatin1String("round");
    // Lengths in the pattern are a share of the line's width, or points.
    auto length = [&](const char *name) {
        double v = 0;
        const QString s = a.get(name);
        if (double p = 0; parsePercent(s, &p)) return p * std::max(width, 0.75);
        return parseLength(s, &v) ? v : 0.0;
    };
    double dots1 = 0, dots2 = 0;
    parseNumber(a.get("draw:dots1", QStringLiteral("1")), &dots1);
    parseNumber(a.get("draw:dots2", QStringLiteral("0")), &dots2);
    const double l1 = length("draw:dots1-length"), l2 = length("draw:dots2-length");
    const double unit = std::max(width, 0.75);
    auto isDot = [&](double l) { return l <= 1.5 * unit; };
    auto isLong = [&](double l) { return l >= 6 * unit; };
    if (dots2 > 0) {
        if (isDot(l1) && isDot(l2)) return Stroke::LongDashDotDot;
        return isLong(l1) ? Stroke::LongDashDot : Stroke::DashDot;
    }
    if (isDot(l1)) return round ? Stroke::RoundDot : Stroke::SquareDot;
    return isLong(l1) ? Stroke::LongDash : Stroke::DashLine;
}

} // namespace

Stroke Reader::strokeOf(const Bag &bag, const StyleSheet &styles)
{
    const QString mode = bag.get(Graphic, "draw:stroke");
    if (mode != QLatin1String("solid") && mode != QLatin1String("dash")) return Stroke::none();
    Stroke s;
    double width = 0;
    parseLength(bag.get(Graphic, "svg:stroke-width"), &width);
    // A line of no width is as thin as the screen draws one.
    s.width = width > 0 ? std::clamp(width, 0.1, 1000.0) : 0.5;
    const QColor c = parseColor(bag.get(Graphic, "svg:stroke-color"));
    s.color = ColorRef::rgb(c.isValid() ? c : QColor(Qt::black));
    if (double o = 1; parsePercent(bag.get(Graphic, "svg:stroke-opacity"), &o)) s.transparency = std::clamp(1 - o, 0.0, 1.0);
    if (mode == QLatin1String("dash")) s.dash = dashOf(styles.dash(bag.get(Graphic, "draw:stroke-dash")), s.width);
    const QString join = bag.get(Graphic, "draw:stroke-linejoin");
    s.join = join == QLatin1String("round") ? Qt::RoundJoin : join == QLatin1String("bevel") ? Qt::BevelJoin : Qt::MiterJoin;
    const QString cap = bag.get(Graphic, "svg:stroke-linecap");
    s.cap = cap == QLatin1String("round") ? Qt::RoundCap : cap == QLatin1String("square") ? Qt::SquareCap : Qt::FlatCap;
    s.startArrow = arrowOf(styles.marker(bag.get(Graphic, "draw:marker-start")));
    s.endArrow = arrowOf(styles.marker(bag.get(Graphic, "draw:marker-end")));
    s.startSize = arrowSize(lengthOr(bag.get(Graphic, "draw:marker-start-width"), 0), s.width);
    s.endSize = arrowSize(lengthOr(bag.get(Graphic, "draw:marker-end-width"), 0), s.width);
    return s;
}

Effects Reader::effectsOf(const Bag &bag)
{
    Effects fx;
    if (bag.get(Graphic, "draw:shadow") != QLatin1String("visible")) return fx;
    const double dx = lengthOr(bag.get(Graphic, "draw:shadow-offset-x"), 0), dy = lengthOr(bag.get(Graphic, "draw:shadow-offset-y"), 0);
    fx.shadow.on = true;
    const QColor c = parseColor(bag.get(Graphic, "draw:shadow-color"));
    fx.shadow.color = ColorRef::rgb(c.isValid() ? c : QColor(Qt::gray));
    double opacity = 1;
    parsePercent(bag.get(Graphic, "draw:shadow-opacity"), &opacity);
    fx.shadow.transparency = std::clamp(1 - opacity, 0.0, 1.0);
    fx.shadow.distance = std::hypot(dx, dy);
    fx.shadow.angle = std::fmod(qRadiansToDegrees(std::atan2(dy, dx)) + 360, 360.0);
    const QString blur = bag.has(Graphic, "draw:shadow-blur") ? bag.get(Graphic, "draw:shadow-blur") : bag.get(Graphic, "loext:shadow-blur");
    fx.shadow.blur = lengthOr(blur, 0);
    return fx;
}

} // namespace jp::deck::odp
