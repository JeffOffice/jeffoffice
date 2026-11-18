#include "office/drawingml_geometry.h"

#include "office/drawingml_write.h"
#include "office/xml.h"
#include "render/shapes.h"

#include <QLineF>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp::office {

namespace {

constexpr int kMaxGuides = 2000;
constexpr int kMaxGuideDepth = 64;
constexpr int kMaxPaths = 64;
constexpr int kMaxPathCommands = 100000;
constexpr double kMaxValue = 1e15;
constexpr double kMaxPoint = 1e6;     // points, as for every position in a drawing
constexpr double kEmuPerPoint = 12700;
constexpr double kTwoPi = 2 * M_PI;

double finite(double v)
{
    return std::isfinite(v) ? std::clamp(v, -kMaxValue, kMaxValue) : 0.0;
}

double toRadians(double angle60k)
{
    return angle60k / 60000.0 * M_PI / 180.0;
}

double toAngle60k(double radians)
{
    return radians * 180.0 / M_PI * 60000.0;
}

QDomElement childNamed(const QDomElement &e, const char *local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(local)) return c;
    return QDomElement();
}

bool isNamed(const QDomElement &e, const char *local)
{
    return e.localName() == QLatin1String(local);
}

} // namespace

QHash<QString, double> readAdjustments(const QDomElement &avLst)
{
    QHash<QString, double> out;
    int n = 0;
    for (QDomElement gd = avLst.firstChildElement(); !gd.isNull() && n < kMaxGuides; gd = gd.nextSiblingElement(), ++n) {
        if (!isNamed(gd, "gd")) continue;
        const QStringList parts = attr(gd, QStringLiteral("fmla")).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
        bool ok = false;
        const double v = parts.size() == 2 && parts[0] == QLatin1String("val") ? parts[1].toDouble(&ok) : 0.0;
        if (ok && std::isfinite(v) && !attr(gd, QStringLiteral("name")).isEmpty()) out.insert(attr(gd, QStringLiteral("name")), finite(v));
    }
    return out;
}

namespace {

QVector<GeomGuide> readGuideList(const QDomElement &list)
{
    QVector<GeomGuide> out;
    for (QDomElement gd = list.firstChildElement(); !gd.isNull() && out.size() < kMaxGuides; gd = gd.nextSiblingElement())
        if (isNamed(gd, "gd")) out.append({attr(gd, QStringLiteral("name")), attr(gd, QStringLiteral("fmla")).simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts)});
    return out;
}

GeometryPath::Fill fillMode(const QString &s)
{
    if (s == QLatin1String("none")) return GeometryPath::None;
    if (s == QLatin1String("lighten")) return GeometryPath::Lighten;
    if (s == QLatin1String("lightenLess")) return GeometryPath::LightenLess;
    if (s == QLatin1String("darken")) return GeometryPath::Darken;
    if (s == QLatin1String("darkenLess")) return GeometryPath::DarkenLess;
    return GeometryPath::Normal;
}

GeomSpec readSpec(const QDomElement &custGeom)
{
    GeomSpec spec;
    spec.adjust = readGuideList(childNamed(custGeom, "avLst"));
    spec.guides = readGuideList(childNamed(custGeom, "gdLst"));
    const QDomElement rect = childNamed(custGeom, "rect");
    if (!rect.isNull() && hasAttr(rect, QStringLiteral("l")) && hasAttr(rect, QStringLiteral("t")) && hasAttr(rect, QStringLiteral("r")) && hasAttr(rect, QStringLiteral("b"))) {
        spec.hasTextRect = true;
        spec.textRect[0] = attr(rect, QStringLiteral("l"));
        spec.textRect[1] = attr(rect, QStringLiteral("t"));
        spec.textRect[2] = attr(rect, QStringLiteral("r"));
        spec.textRect[3] = attr(rect, QStringLiteral("b"));
    }
    int commands = 0;
    for (QDomElement path = childNamed(childNamed(custGeom, "pathLst"), "path"); !path.isNull() && spec.paths.size() < kMaxPaths; path = path.nextSiblingElement()) {
        if (!isNamed(path, "path")) continue;
        GeomPath ps;
        ps.width = attr(path, QStringLiteral("w"));
        ps.height = attr(path, QStringLiteral("h"));
        ps.fill = fillMode(attr(path, QStringLiteral("fill")));
        ps.stroke = parseBool(attr(path, QStringLiteral("stroke")), true);
        ps.extrusionOk = parseBool(attr(path, QStringLiteral("extrusionOk")), true);
        for (QDomElement c = path.firstChildElement(); !c.isNull() && commands < kMaxPathCommands; c = c.nextSiblingElement()) {
            GeomCommand cmd;
            QVector<QDomElement> pts;
            for (QDomElement pt = c.firstChildElement(); !pt.isNull() && pts.size() < 3; pt = pt.nextSiblingElement())
                if (isNamed(pt, "pt")) pts.append(pt);
            int need = 0;
            if (isNamed(c, "moveTo")) { cmd.kind = GeomCommand::Move; need = 1; }
            else if (isNamed(c, "lnTo")) { cmd.kind = GeomCommand::Line; need = 1; }
            else if (isNamed(c, "quadBezTo")) { cmd.kind = GeomCommand::Quad; need = 2; }
            else if (isNamed(c, "cubicBezTo")) { cmd.kind = GeomCommand::Cubic; need = 3; }
            else if (isNamed(c, "close")) cmd.kind = GeomCommand::Close;
            else if (isNamed(c, "arcTo")) {
                cmd.kind = GeomCommand::Arc;
                cmd.arg[0] = attr(c, QStringLiteral("wR"));
                cmd.arg[1] = attr(c, QStringLiteral("hR"));
                cmd.arg[2] = attr(c, QStringLiteral("stAng"));
                cmd.arg[3] = attr(c, QStringLiteral("swAng"));
            } else continue;
            if (pts.size() < need) continue;
            for (int i = 0; i < need; ++i) {
                cmd.arg[2 * i] = attr(pts[i], QStringLiteral("x"));
                cmd.arg[2 * i + 1] = attr(pts[i], QStringLiteral("y"));
            }
            ps.commands.append(cmd);
            ++commands;
        }
        spec.paths.append(ps);
    }
    return spec;
}

} // namespace

Geometry readCustomGeometry(const QDomElement &custGeom, const QSizeF &size)
{
    if (custGeom.isNull()) return Geometry();
    return buildGeometry(readSpec(custGeom), {}, size);
}

namespace {

// The presets that have a definition here, for a file's use: those the shape library does not have. (`preferred`: one the library has too, whose definition is to be used for
// a file's shape anyway; the library draws every preset it has as the standard defines it, so none is.) The definitions themselves are core/presetgeom.h's table.
struct Handled {
    const char *name;
    bool preferred;
};

const Handled kHandled[] = {
    {"pie", false},
    {"chord", false},
    {"arc", false},
    {"blockArc", false},
    {"pieWedge", false},
    {"wedgeRectCallout", false},
    {"wedgeRoundRectCallout", false},
    {"wedgeEllipseCallout", false},
    {"borderCallout1", false},
    {"borderCallout2", false},
    {"borderCallout3", false},
    {"callout1", false},
    {"callout2", false},
    {"callout3", false},
    {"accentCallout1", false},
    {"accentCallout2", false},
    {"accentCallout3", false},
    {"accentBorderCallout1", false},
    {"accentBorderCallout2", false},
    {"rightArrowCallout", false},
    {"leftArrowCallout", false},
    {"downArrowCallout", false},
    {"upArrowCallout", false},
    {"leftRightArrowCallout", false},
    {"bentConnector2", false},
    {"bentConnector3", false},
    {"bentConnector4", false},
    {"bentConnector5", false},
    {"curvedConnector2", false},
    {"curvedConnector3", false},
    {"curvedConnector4", false},
    {"curvedConnector5", false},
    {"nonIsoscelesTrapezoid", false},
    {"accentBorderCallout3", false},
    {"upDownArrowCallout", false},
};

const Handled *handled(const QString &prst)
{
    for (const Handled &h : kHandled)
        if (prst == QLatin1String(h.name)) return &h;
    return nullptr;
}

} // namespace

Geometry presetGeometry(const QString &prst, const QHash<QString, double> &adjust, const QSizeF &size)
{
    const GeomSpec *spec = handled(prst) ? presetSpec(prst) : nullptr;
    return spec ? buildGeometry(*spec, adjust, size) : Geometry();
}

bool hasPresetGeometry(const QString &prst)
{
    return handled(prst) != nullptr;
}

bool prefersPresetGeometry(const QString &prst)
{
    const Handled *h = handled(prst);
    return h && h->preferred;
}

// ---- the shape library's presets

QString shapeForPreset(const QString &prst)
{
    return shapeIdOfPreset(prst);
}

QString presetForShape(const QString &shapeId)
{
    return presetOfShapeId(shapeId);
}

QVector<double> libraryAdjustments(const QString &prst, const QHash<QString, double> &given, const QSizeF &size)
{
    const QVector<double> values = presetAdjustmentValues(prst, given, size);
    QVector<double> out;
    for (int i = 0; i < values.size(); ++i) out.append(values[i] / presetAdjustmentScale(prst, i));
    return out;
}

QVector<QPair<QString, double>> presetAdjustments(const QString &prst, const QVector<double> &adj, const QSizeF &)
{
    QVector<QPair<QString, double>> out;
    const QStringList names = presetAdjustmentList(prst);
    const QVector<double> defaults = presetAdjustmentDefaults(prst);
    QVector<double> values = defaults;
    bool changed = false;
    for (int i = 0; i < values.size() && i < adj.size(); ++i) {
        const double value = std::round(adj[i] * presetAdjustmentScale(prst, i));
        if (!std::isfinite(value) || std::fabs(value - defaults[i]) < 1) continue;
        values[i] = value;
        changed = true;
    }
    // All or none: the presentation program refuses a file with a shape that gives some of its preset's adjustments and leaves out the rest.
    if (!changed) return out;
    for (int i = 0; i < values.size(); ++i) out.append({names[i], std::round(values[i])});
    return out;
}

QByteArray customGeometryXml(const QPainterPath &outline, const QSizeF &size, const QRectF &textRect, const QVector<QPointF> &sites)
{
    auto emu = [](double v) { return QByteArray::number(qint64(std::llround(v * 12700.0))); };
    auto pt = [&](const QPointF &p) { return "<a:pt x=\"" + emu(p.x()) + "\" y=\"" + emu(p.y()) + "\"/>"; };
    const QRectF text = textRect.isNull() ? QRectF(QPointF(0, 0), size) : textRect;
    QByteArray out = "<a:custGeom><a:avLst/><a:gdLst/><a:ahLst/>";
    if (sites.size() == 4) {
        static const char *angles[4] = {"16200000", "10800000", "5400000", "0"};
        out += "<a:cxnLst>";
        for (int i = 0; i < 4; ++i) out += QByteArray("<a:cxn ang=\"") + angles[i] + "\"><a:pos x=\"" + emu(sites[i].x()) + "\" y=\"" + emu(sites[i].y()) + "\"/></a:cxn>";
        out += "</a:cxnLst>";
    } else {
        out += "<a:cxnLst/>";
    }
    out += "<a:rect l=\"" + emu(text.left()) + "\" t=\"" + emu(text.top()) + "\" r=\"" + emu(text.right()) + "\" b=\"" + emu(text.bottom()) + "\"/><a:pathLst><a:path w=\"" + emu(size.width()) +
           "\" h=\"" + emu(size.height()) + "\">";
    QPointF start;
    for (int i = 0; i < outline.elementCount(); ++i) {
        const QPainterPath::Element e = outline.elementAt(i);
        const QPointF p(e.x, e.y);
        const bool lastOfSubpath = [&](int k) { return k + 1 >= outline.elementCount() || outline.elementAt(k + 1).type == QPainterPath::MoveToElement; }(i);
        if (e.type == QPainterPath::MoveToElement) {
            out += "<a:moveTo>" + pt(p) + "</a:moveTo>";
            start = p;
        } else if (e.type == QPainterPath::LineToElement) {
            out += "<a:lnTo>" + pt(p) + "</a:lnTo>";
            if (lastOfSubpath && QLineF(p, start).length() < 1e-6) out += "<a:close/>";
        } else if (e.type == QPainterPath::CurveToElement && i + 2 < outline.elementCount()) {
            const QPointF c2(outline.elementAt(i + 1).x, outline.elementAt(i + 1).y), end(outline.elementAt(i + 2).x, outline.elementAt(i + 2).y);
            out += "<a:cubicBezTo>" + pt(p) + pt(c2) + pt(end) + "</a:cubicBezTo>";
            i += 2;
            const bool lastCurve = i + 1 >= outline.elementCount() || outline.elementAt(i + 1).type == QPainterPath::MoveToElement;
            if (lastCurve && QLineF(end, start).length() < 1e-6) out += "<a:close/>";
        }
    }
    return out + "</a:path></a:pathLst></a:custGeom>";
}

} // namespace jp::office
