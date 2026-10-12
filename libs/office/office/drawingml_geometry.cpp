#include "office/drawingml_geometry.h"

#include "office/xml.h"

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

GuideEvaluator::GuideEvaluator(double width, double height)
{
    width = finite(width);
    height = finite(height);
    const double ss = std::min(width, height), ls = std::max(width, height);
    const struct { const char *name; double value; } names[] = {
        {"w", width}, {"h", height}, {"l", 0}, {"t", 0}, {"r", width}, {"b", height}, {"hc", width / 2}, {"vc", height / 2},
        {"wd2", width / 2}, {"wd3", width / 3}, {"wd4", width / 4}, {"wd5", width / 5}, {"wd6", width / 6}, {"wd8", width / 8},
        {"wd10", width / 10}, {"wd32", width / 32},
        {"hd2", height / 2}, {"hd3", height / 3}, {"hd4", height / 4}, {"hd5", height / 5}, {"hd6", height / 6}, {"hd8", height / 8},
        {"ss", ss}, {"ls", ls}, {"ssd2", ss / 2}, {"ssd4", ss / 4}, {"ssd6", ss / 6}, {"ssd8", ss / 8}, {"ssd16", ss / 16}, {"ssd32", ss / 32},
        {"cd2", 10800000}, {"cd4", 5400000}, {"cd8", 2700000}, {"3cd4", 16200000}, {"3cd8", 8100000}, {"5cd8", 13500000}, {"7cd8", 18900000}};
    for (const auto &n : names) m_values.insert(QString::fromLatin1(n.name), n.value);
}

void GuideEvaluator::define(const QString &name, const QString &formula)
{
    if (name.isEmpty() || m_values.contains(name) || m_formulas.contains(name) || m_formulas.size() >= kMaxGuides) return;
    m_formulas.insert(name, formula);
}

void GuideEvaluator::set(const QString &name, double value)
{
    if (name.isEmpty()) return;
    m_formulas.remove(name);
    m_values.insert(name, finite(value));
}

double GuideEvaluator::value(const QString &token)
{
    const auto known = m_values.constFind(token);
    if (known != m_values.constEnd()) return *known;
    const auto formula = m_formulas.constFind(token);
    if (formula != m_formulas.constEnd()) {
        if (m_depth >= kMaxGuideDepth || m_active.contains(token)) return 0;
        ++m_depth;
        m_active.insert(token);
        const double v = evaluate(*formula);
        --m_depth;
        m_active.remove(token);
        m_values.insert(token, v);
        return v;
    }
    bool ok = false;
    const double number = token.toDouble(&ok);
    return ok ? finite(number) : 0.0;
}

double GuideEvaluator::evaluate(const QString &formula)
{
    const QStringList parts = formula.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return 0;
    const QString &op = parts[0];
    auto arg = [&](int i) { return i < parts.size() ? value(parts[i]) : 0.0; };
    auto divide = [](double n, double d) { return d == 0 ? 0.0 : n / d; };
    double r = 0;
    if (op == QLatin1String("val")) r = arg(1);
    else if (op == QLatin1String("*/")) { const double x = arg(1), y = arg(2), z = arg(3); r = divide(x * y, z); }
    else if (op == QLatin1String("+-")) { const double x = arg(1), y = arg(2), z = arg(3); r = x + y - z; }
    else if (op == QLatin1String("+/")) { const double x = arg(1), y = arg(2), z = arg(3); r = divide(x + y, z); }
    else if (op == QLatin1String("?:")) { const double x = arg(1), y = arg(2), z = arg(3); r = x > 0 ? y : z; }
    else if (op == QLatin1String("abs")) r = std::fabs(arg(1));
    else if (op == QLatin1String("at2")) { const double x = arg(1), y = arg(2); r = toAngle60k(std::atan2(y, x)); }
    else if (op == QLatin1String("cat2")) { const double x = arg(1), y = arg(2), z = arg(3); r = x * std::cos(std::atan2(z, y)); }
    else if (op == QLatin1String("sat2")) { const double x = arg(1), y = arg(2), z = arg(3); r = x * std::sin(std::atan2(z, y)); }
    else if (op == QLatin1String("cos")) { const double x = arg(1), y = arg(2); r = x * std::cos(toRadians(y)); }
    else if (op == QLatin1String("sin")) { const double x = arg(1), y = arg(2); r = x * std::sin(toRadians(y)); }
    else if (op == QLatin1String("tan")) { const double x = arg(1), y = arg(2); r = x * std::tan(toRadians(y)); }
    else if (op == QLatin1String("max")) { const double x = arg(1), y = arg(2); r = std::max(x, y); }
    else if (op == QLatin1String("min")) { const double x = arg(1), y = arg(2); r = std::min(x, y); }
    else if (op == QLatin1String("mod")) { const double x = arg(1), y = arg(2), z = arg(3); r = std::sqrt(x * x + y * y + z * z); }
    else if (op == QLatin1String("pin")) { const double x = arg(1), y = arg(2), z = arg(3); r = y < x ? x : y > z ? z : y; }
    else if (op == QLatin1String("sqrt")) { const double x = arg(1); r = x > 0 ? std::sqrt(x) : 0.0; }
    return finite(r);
}

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

// What a:path says to do, before any value is known.
struct PathCommand {
    enum Kind { Move, Line, Arc, Quad, Cubic, Close } kind = Close;
    QString arg[6];
};

struct PathSpec {
    QString width, height;
    GeometryPath::Fill fill = GeometryPath::Normal;
    bool stroke = true;
    QVector<PathCommand> commands;
};

// A shape's definition as the standard writes it: guides and paths.
struct ShapeSpec {
    QVector<QPair<QString, QString>> adjust, guides;
    QString textRect[4];
    bool hasTextRect = false;
    QVector<PathSpec> paths;
};

QVector<QPair<QString, QString>> readGuideList(const QDomElement &list)
{
    QVector<QPair<QString, QString>> out;
    for (QDomElement gd = list.firstChildElement(); !gd.isNull() && out.size() < kMaxGuides; gd = gd.nextSiblingElement())
        if (isNamed(gd, "gd")) out.append({attr(gd, QStringLiteral("name")), attr(gd, QStringLiteral("fmla"))});
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

ShapeSpec readSpec(const QDomElement &custGeom)
{
    ShapeSpec spec;
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
        PathSpec ps;
        ps.width = attr(path, QStringLiteral("w"));
        ps.height = attr(path, QStringLiteral("h"));
        ps.fill = fillMode(attr(path, QStringLiteral("fill")));
        ps.stroke = parseBool(attr(path, QStringLiteral("stroke")), true);
        for (QDomElement c = path.firstChildElement(); !c.isNull() && commands < kMaxPathCommands; c = c.nextSiblingElement()) {
            PathCommand cmd;
            QVector<QDomElement> pts;
            for (QDomElement pt = c.firstChildElement(); !pt.isNull() && pts.size() < 3; pt = pt.nextSiblingElement())
                if (isNamed(pt, "pt")) pts.append(pt);
            int need = 0;
            if (isNamed(c, "moveTo")) { cmd.kind = PathCommand::Move; need = 1; }
            else if (isNamed(c, "lnTo")) { cmd.kind = PathCommand::Line; need = 1; }
            else if (isNamed(c, "quadBezTo")) { cmd.kind = PathCommand::Quad; need = 2; }
            else if (isNamed(c, "cubicBezTo")) { cmd.kind = PathCommand::Cubic; need = 3; }
            else if (isNamed(c, "close")) cmd.kind = PathCommand::Close;
            else if (isNamed(c, "arcTo")) {
                cmd.kind = PathCommand::Arc;
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

// An elliptical arc from the path's current point. The angles are those of
// the standard's: a ray from the ellipse's center, so on an ellipse they are
// not the angle that a point's coordinates are made from; the arc begins
// where the ray through `startAngle` meets the ellipse.
void addArc(QPainterPath &p, double wR, double hR, double startAngle60k, double sweep60k)
{
    const double a = std::fabs(wR), b = std::fabs(hR);
    const double sweep = std::clamp(toRadians(sweep60k), -kTwoPi, kTwoPi);
    if (a < 1e-9 || b < 1e-9 || sweep == 0) return;
    if (p.elementCount() == 0) p.moveTo(0, 0);
    const QPointF from = p.currentPosition();
    const double start = toRadians(startAngle60k);
    // The parameter of the ellipse's equation at the start and at the end.
    const double t1 = std::atan2(a * std::sin(start), b * std::cos(start));
    const double t2 = std::atan2(a * std::sin(start + sweep), b * std::cos(start + sweep));
    double dt = t2 - t1;
    dt += kTwoPi * std::round((sweep - dt) / kTwoPi);
    const QPointF center(from.x() - a * std::cos(t1), from.y() - b * std::sin(t1));
    const int pieces = std::max(1, int(std::ceil(std::fabs(dt) / (M_PI / 2) - 1e-9)));
    const double step = dt / pieces, k = 4.0 / 3.0 * std::tan(step / 4);
    for (int i = 0; i < pieces; ++i) {
        const double ta = t1 + step * i, tb = ta + step;
        const QPointF pa(center.x() + a * std::cos(ta), center.y() + b * std::sin(ta));
        const QPointF pb(center.x() + a * std::cos(tb), center.y() + b * std::sin(tb));
        const QPointF c1(pa.x() - k * a * std::sin(ta), pa.y() + k * b * std::cos(ta));
        const QPointF c2(pb.x() + k * a * std::sin(tb), pb.y() - k * b * std::cos(tb));
        p.cubicTo(c1, c2, pb);
    }
}

Geometry build(const ShapeSpec &spec, const QHash<QString, double> &adjust, const QSizeF &size)
{
    Geometry out;
    const double w = std::isfinite(size.width()) ? size.width() : 0.0, h = std::isfinite(size.height()) ? size.height() : 0.0;
    if (w < 0 || h < 0 || (w == 0 && h == 0)) return out;
    GuideEvaluator ev(w * kEmuPerPoint, h * kEmuPerPoint);
    for (auto it = adjust.constBegin(); it != adjust.constEnd(); ++it) ev.set(it.key(), it.value());
    for (const auto &g : spec.adjust) ev.define(g.first, g.second);
    for (const auto &g : spec.guides) ev.define(g.first, g.second);
    // In order, so that a guide's operands are known by the time it is used.
    for (const auto &g : spec.adjust) ev.value(g.first);
    for (const auto &g : spec.guides) ev.value(g.first);

    if (spec.hasTextRect) {
        const double l = ev.value(spec.textRect[0]), t = ev.value(spec.textRect[1]), r = ev.value(spec.textRect[2]), b = ev.value(spec.textRect[3]);
        auto pt = [](double emu) { return std::clamp(emu / kEmuPerPoint, -kMaxPoint, kMaxPoint); };
        out.textRect = QRectF(QPointF(pt(l), pt(t)), QPointF(pt(r), pt(b))).normalized();
    }
    for (const PathSpec &ps : spec.paths) {
        // A path's own width and height, if it gives them, are what its numbers
        // are out of; without, they are EMU.
        bool ok = false;
        const double pw = ps.width.toDouble(&ok), phv = ps.height.toDouble(&ok);
        const double kx = pw > 0 ? w / pw : 1 / kEmuPerPoint, ky = phv > 0 ? h / phv : 1 / kEmuPerPoint;
        GeometryPath gp;
        gp.fill = ps.fill;
        gp.stroke = ps.stroke;
        QPainterPath &p = gp.path;
        auto x = [&](const QString &token) { return std::clamp(ev.value(token) * kx, -kMaxPoint, kMaxPoint); };
        auto y = [&](const QString &token) { return std::clamp(ev.value(token) * ky, -kMaxPoint, kMaxPoint); };
        for (const PathCommand &c : ps.commands) {
            switch (c.kind) {
            case PathCommand::Move: p.moveTo(x(c.arg[0]), y(c.arg[1])); break;
            case PathCommand::Line: p.lineTo(x(c.arg[0]), y(c.arg[1])); break;
            case PathCommand::Quad: p.quadTo(x(c.arg[0]), y(c.arg[1]), x(c.arg[2]), y(c.arg[3])); break;
            case PathCommand::Cubic: p.cubicTo(x(c.arg[0]), y(c.arg[1]), x(c.arg[2]), y(c.arg[3]), x(c.arg[4]), y(c.arg[5])); break;
            case PathCommand::Arc:
                addArc(p, std::clamp(ev.value(c.arg[0]) * kx, -kMaxPoint, kMaxPoint), std::clamp(ev.value(c.arg[1]) * ky, -kMaxPoint, kMaxPoint),
                       ev.value(c.arg[2]), ev.value(c.arg[3]));
                break;
            case PathCommand::Close: if (p.elementCount() > 0) p.closeSubpath(); break;
            }
        }
        if (p.elementCount() > 1) out.paths.append(gp);
    }
    return out;
}

} // namespace

Geometry readCustomGeometry(const QDomElement &custGeom, const QSizeF &size)
{
    if (custGeom.isNull()) return Geometry();
    return build(readSpec(custGeom), {}, size);
}

Geometry presetGeometry(const QString &, const QHash<QString, double> &, const QSizeF &) { return {}; }
bool hasPresetGeometry(const QString &) { return false; }
bool prefersPresetGeometry(const QString &) { return false; }
QVector<double> libraryAdjustments(const QString &, const QHash<QString, double> &, const QSizeF &) { return {}; }

} // namespace jp::office
