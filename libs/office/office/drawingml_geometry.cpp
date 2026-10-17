#include "office/drawingml_geometry.h"

#include "office/drawingml_write.h"
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
    auto kept = [](const QPointF &pt) { return QPointF(std::clamp(pt.x(), -kMaxPoint, kMaxPoint), std::clamp(pt.y(), -kMaxPoint, kMaxPoint)); };
    for (int i = 0; i < pieces; ++i) {
        const double ta = t1 + step * i, tb = ta + step;
        const QPointF pa(center.x() + a * std::cos(ta), center.y() + b * std::sin(ta));
        const QPointF pb(center.x() + a * std::cos(tb), center.y() + b * std::sin(tb));
        const QPointF c1(pa.x() - k * a * std::sin(ta), pa.y() + k * b * std::cos(ta));
        const QPointF c2(pb.x() + k * a * std::sin(tb), pb.y() - k * b * std::cos(tb));
        p.cubicTo(kept(c1), kept(c2), kept(pb));
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

namespace {

// A preset's definition in a short form of the standard's own: the default
// adjustments ("adj1=16667 adj2=0"), the guides ("name=formula;..."), and
// the paths. A path is its commands, M(oveTo) L(ineTo) A(rcTo) Q(uadBezTo)
// C(ubicBezTo) and Z (close), with the points' names or numbers after
// them, behind [nofill] and [nostroke] if it has them; paths are separated by #.
struct PresetText {
    const char *name;
    const char *adjust;
    const char *guides;
    const char *paths;
    bool preferred;     // used though the shape library has the preset
    const char *textRect = nullptr;   // "l t r b", where text sits if not all of the frame
};

const char kAngles[] =
    "stAng=pin 0 adj1 21599999;enAng=pin 0 adj2 21599999;sw1=+- enAng 0 stAng;sw2=+- sw1 21600000 0;swAng=?: sw1 sw1 sw2;"
    "wt1=sin wd2 stAng;ht1=cos hd2 stAng;dx1=cat2 wd2 ht1 wt1;dy1=sat2 hd2 ht1 wt1;x1=+- hc dx1 0;y1=+- vc dy1 0;"
    "wt2=sin wd2 enAng;ht2=cos hd2 enAng;dx2=cat2 wd2 ht2 wt2;dy2=sat2 hd2 ht2 wt2;x2=+- hc dx2 0;y2=+- vc dy2 0";

// Where a callout's pointer meets the box, and the sides it leaves from.
#define WEDGE_GUIDES \
    "dxPos=*/ w adj1 100000;dyPos=*/ h adj2 100000;xPos=+- hc dxPos 0;yPos=+- vc dyPos 0;dq=*/ dxPos h w;ady=abs dyPos;adq=abs dq;dz=+- ady 0 adq;" \
    "xg1=?: dxPos 7 2;xg2=?: dxPos 10 5;x1=*/ w xg1 12;x2=*/ w xg2 12;yg1=?: dyPos 7 2;yg2=?: dyPos 10 5;y1=*/ h yg1 12;y2=*/ h yg2 12;" \
    "t1=?: dxPos l xPos;xl=?: dz l t1;t2=?: dyPos x1 xPos;xt=?: dz t2 x1;t3=?: dxPos xPos r;xr=?: dz r t3;t4=?: dyPos xPos x1;xb=?: dz t4 x1;" \
    "t5=?: dxPos y1 yPos;yl=?: dz y1 t5;t6=?: dyPos t yPos;yt=?: dz t6 t;t7=?: dxPos yPos y1;yr=?: dz y1 t7;t8=?: dyPos yPos b;yb=?: dz t8 b"

const char kLine1[] = "y1=*/ h adj1 100000;x1=*/ w adj2 100000;y2=*/ h adj3 100000;x2=*/ w adj4 100000";
const char kLine2[] = "y1=*/ h adj1 100000;x1=*/ w adj2 100000;y2=*/ h adj3 100000;x2=*/ w adj4 100000;y3=*/ h adj5 100000;x3=*/ w adj6 100000";
const char kLine3[] = "y1=*/ h adj1 100000;x1=*/ w adj2 100000;y2=*/ h adj3 100000;x2=*/ w adj4 100000;y3=*/ h adj5 100000;x3=*/ w adj6 100000;"
                      "y4=*/ h adj7 100000;x4=*/ w adj8 100000";
const char kAdjust1[] = "adj1=18750 adj2=-8333 adj3=112500 adj4=-38333";
const char kAdjust2[] = "adj1=18750 adj2=-8333 adj3=18750 adj4=-16667 adj5=112500 adj6=-46667";
const char kAdjust3[] = "adj1=18750 adj2=-8333 adj3=18750 adj4=-16667 adj5=100000 adj6=-16667 adj7=112963 adj8=-8333";

const PresetText kPresets[] = {
    {"pie", "adj1=0 adj2=16200000", kAngles, "M x1 y1 A wd2 hd2 stAng swAng L hc vc Z", true},
    {"chord", "adj1=2700000 adj2=16200000", kAngles, "M x1 y1 A wd2 hd2 stAng swAng Z", true},
    {"arc", "adj1=16200000 adj2=0", kAngles, "[nostroke] M x1 y1 A wd2 hd2 stAng swAng L hc vc Z # [nofill] M x1 y1 A wd2 hd2 stAng swAng", true},
    {"blockArc", "adj1=10800000 adj2=0 adj3=25000",
     "stAng=pin 0 adj1 21599999;istAng=pin 0 adj2 21599999;a3=pin 0 adj3 50000;sw11=+- istAng 0 stAng;sw12=+- sw11 21600000 0;swAng=?: sw11 sw11 sw12;"
     "iswAng=+- 0 0 swAng;wt1=sin wd2 stAng;ht1=cos hd2 stAng;dx1=cat2 wd2 ht1 wt1;dy1=sat2 hd2 ht1 wt1;x1=+- hc dx1 0;y1=+- vc dy1 0;"
     "dr=*/ ss a3 100000;iwd2=+- wd2 0 dr;ihd2=+- hd2 0 dr;wt2=sin iwd2 istAng;ht2=cos ihd2 istAng;dx2=cat2 iwd2 ht2 wt2;dy2=sat2 ihd2 ht2 wt2;"
     "x2=+- hc dx2 0;y2=+- vc dy2 0",
     "M x1 y1 A wd2 hd2 stAng swAng L x2 y2 A iwd2 ihd2 istAng iswAng Z", true},
    {"pieWedge", "", "", "M l b A w h cd2 cd4 L r b Z", false},
    {"nonIsoscelesTrapezoid", "adj1=25000 adj2=25000",
     "maxAdj=*/ 50000 w ss;a1=pin 0 adj1 maxAdj;a2=pin 0 adj2 maxAdj;x2=*/ ss a1 100000;x3=*/ ss a2 100000;x4=+- r 0 x3",
     "M l b L x2 t L x4 t L r b Z", false},

    // Callouts: a box with a pointer, or with a line to what it is about.
    {"wedgeRectCallout", "adj1=-20833 adj2=62500", WEDGE_GUIDES,
     "M l t L x1 t L xt yt L x2 t L r t L r y1 L xr yr L r y2 L r b L x2 b L xb yb L x1 b L l b L l y2 L xl yl L l y1 Z", true},
    {"wedgeRoundRectCallout", "adj1=-20833 adj2=62500 adj3=16667", WEDGE_GUIDES ";u1=*/ ss adj3 100000;u2=+- r 0 u1;v2=+- b 0 u1",
     "M l u1 A u1 u1 cd2 cd4 L x1 t L xt yt L x2 t L u2 t A u1 u1 3cd4 cd4 L r y1 L xr yr L r y2 L r v2 A u1 u1 0 cd4 "
     "L x2 b L xb yb L x1 b L u1 b A u1 u1 cd4 cd4 L l y2 L xl yl L l y1 Z", true},
    {"wedgeEllipseCallout", "adj1=-20833 adj2=62500",
     "dxPos=*/ w adj1 100000;dyPos=*/ h adj2 100000;xPos=+- hc dxPos 0;yPos=+- vc dyPos 0;sdx=*/ dxPos h 1;sdy=*/ dyPos w 1;pang=at2 sdx sdy;"
     "stAng=+- pang 660000 0;enAng=+- pang 0 660000;dx1=cos wd2 stAng;dy1=sin hd2 stAng;x1=+- hc dx1 0;y1=+- vc dy1 0;"
     "dx2=cos wd2 enAng;dy2=sin hd2 enAng;x2=+- hc dx2 0;y2=+- vc dy2 0;stAng1=at2 dx1 dy1;enAng1=at2 dx2 dy2;swAng0=+- enAng1 0 stAng1;"
     "swAng1=+- swAng0 21600000 0;swAng=?: swAng0 swAng0 swAng1",
     "M x1 y1 A wd2 hd2 stAng1 swAng L xPos yPos Z", true},
    {"borderCallout1", kAdjust1, kLine1, "M l t L r t L r b L l b Z # [nofill] M x1 y1 L x2 y2", true},
    {"borderCallout2", kAdjust2, kLine2, "M l t L r t L r b L l b Z # [nofill] M x1 y1 L x2 y2 L x3 y3", false},
    {"borderCallout3", kAdjust3, kLine3, "M l t L r t L r b L l b Z # [nofill] M x1 y1 L x2 y2 L x3 y3 L x4 y4", false},
    {"callout1", kAdjust1, kLine1, "[nostroke] M l t L r t L r b L l b Z # [nofill] M x1 y1 L x2 y2", true},
    {"callout2", kAdjust2, kLine2, "[nostroke] M l t L r t L r b L l b Z # [nofill] M x1 y1 L x2 y2 L x3 y3", false},
    {"callout3", kAdjust3, kLine3, "[nostroke] M l t L r t L r b L l b Z # [nofill] M x1 y1 L x2 y2 L x3 y3 L x4 y4", false},
    {"accentCallout1", kAdjust1, kLine1, "[nostroke] M l t L r t L r b L l b Z # [nofill] M x1 t L x1 b # [nofill] M x1 y1 L x2 y2", true},
    {"accentCallout2", kAdjust2, kLine2, "[nostroke] M l t L r t L r b L l b Z # [nofill] M x1 t L x1 b # [nofill] M x1 y1 L x2 y2 L x3 y3", false},
    {"accentCallout3", kAdjust3, kLine3, "[nostroke] M l t L r t L r b L l b Z # [nofill] M x1 t L x1 b # [nofill] M x1 y1 L x2 y2 L x3 y3 L x4 y4", false},
    {"accentBorderCallout1", kAdjust1, kLine1, "M l t L r t L r b L l b Z # [nofill] M x1 t L x1 b # [nofill] M x1 y1 L x2 y2", false},
    {"accentBorderCallout2", kAdjust2, kLine2, "M l t L r t L r b L l b Z # [nofill] M x1 t L x1 b # [nofill] M x1 y1 L x2 y2 L x3 y3", false},
    {"accentBorderCallout3", kAdjust3, kLine3, "M l t L r t L r b L l b Z # [nofill] M x1 t L x1 b # [nofill] M x1 y1 L x2 y2 L x3 y3 L x4 y4", false},

    // Block arrows with a box behind the arrow.
    {"rightArrowCallout", "adj1=25000 adj2=25000 adj3=25000 adj4=64977",
     "maxAdj2=*/ 50000 h ss;a2=pin 0 adj2 maxAdj2;maxAdj1=*/ a2 2 1;a1=pin 0 adj1 maxAdj1;maxAdj3=*/ 100000 w ss;a3=pin 0 adj3 maxAdj3;"
     "q2=*/ a3 ss w;maxAdj4=+- 100000 0 q2;a4=pin 0 adj4 maxAdj4;dy1=*/ ss a2 100000;dy2=*/ ss a1 200000;y1=+- vc 0 dy1;y2=+- vc 0 dy2;"
     "y3=+- vc dy2 0;y4=+- vc dy1 0;dx3=*/ ss a3 100000;x3=+- r 0 dx3;x2=*/ w a4 100000",
     "M l t L x2 t L x2 y2 L x3 y2 L x3 y1 L r vc L x3 y4 L x3 y3 L x2 y3 L x2 b L l b Z", true, "l t x2 b"},
    {"leftArrowCallout", "adj1=25000 adj2=25000 adj3=25000 adj4=64977",
     "maxAdj2=*/ 50000 h ss;a2=pin 0 adj2 maxAdj2;maxAdj1=*/ a2 2 1;a1=pin 0 adj1 maxAdj1;maxAdj3=*/ 100000 w ss;a3=pin 0 adj3 maxAdj3;"
     "q2=*/ a3 ss w;maxAdj4=+- 100000 0 q2;a4=pin 0 adj4 maxAdj4;dy1=*/ ss a2 100000;dy2=*/ ss a1 200000;y1=+- vc 0 dy1;y2=+- vc 0 dy2;"
     "y3=+- vc dy2 0;y4=+- vc dy1 0;x1=*/ ss a3 100000;dx2=*/ w a4 100000;x2=+- r 0 dx2",
     "M l vc L x1 y1 L x1 y2 L x2 y2 L x2 t L r t L r b L x2 b L x2 y3 L x1 y3 L x1 y4 Z", false, "x2 t r b"},
    {"downArrowCallout", "adj1=25000 adj2=25000 adj3=25000 adj4=64977",
     "maxAdj2=*/ 50000 w ss;a2=pin 0 adj2 maxAdj2;maxAdj1=*/ a2 2 1;a1=pin 0 adj1 maxAdj1;maxAdj3=*/ 100000 h ss;a3=pin 0 adj3 maxAdj3;"
     "q2=*/ a3 ss h;maxAdj4=+- 100000 0 q2;a4=pin 0 adj4 maxAdj4;dx1=*/ ss a2 100000;dx2=*/ ss a1 200000;x1=+- hc 0 dx1;x2=+- hc 0 dx2;"
     "x3=+- hc dx2 0;x4=+- hc dx1 0;dy3=*/ ss a3 100000;y3=+- b 0 dy3;y2=*/ h a4 100000",
     "M l t L r t L r y2 L x3 y2 L x3 y3 L x4 y3 L hc b L x1 y3 L x2 y3 L x2 y2 L l y2 Z", true, "l t r y2"},
    {"upArrowCallout", "adj1=25000 adj2=25000 adj3=25000 adj4=64977",
     "maxAdj2=*/ 50000 w ss;a2=pin 0 adj2 maxAdj2;maxAdj1=*/ a2 2 1;a1=pin 0 adj1 maxAdj1;maxAdj3=*/ 100000 h ss;a3=pin 0 adj3 maxAdj3;"
     "q2=*/ a3 ss h;maxAdj4=+- 100000 0 q2;a4=pin 0 adj4 maxAdj4;dx1=*/ ss a2 100000;dx2=*/ ss a1 200000;x1=+- hc 0 dx1;x2=+- hc 0 dx2;"
     "x3=+- hc dx2 0;x4=+- hc dx1 0;y1=*/ ss a3 100000;dy2=*/ h a4 100000;y2=+- b 0 dy2",
     "M l y2 L x2 y2 L x2 y1 L x1 y1 L hc t L x4 y1 L x3 y1 L x3 y2 L r y2 L r b L l b Z", false, "l y2 r b"},
    {"leftRightArrowCallout", "adj1=25000 adj2=25000 adj3=25000 adj4=48123",
     "maxAdj2=*/ 50000 h ss;a2=pin 0 adj2 maxAdj2;maxAdj1=*/ a2 2 1;a1=pin 0 adj1 maxAdj1;maxAdj3=*/ 50000 w ss;a3=pin 0 adj3 maxAdj3;"
     "q2=*/ a3 ss wd2;maxAdj4=+- 100000 0 q2;a4=pin 0 adj4 maxAdj4;dy1=*/ ss a2 100000;dy2=*/ ss a1 200000;y1=+- vc 0 dy1;y2=+- vc 0 dy2;"
     "y3=+- vc dy2 0;y4=+- vc dy1 0;x1=*/ ss a3 100000;x4=+- r 0 x1;dx2=*/ w a4 200000;x2=+- hc 0 dx2;x3=+- hc dx2 0",
     "M l vc L x1 y1 L x1 y2 L x2 y2 L x2 t L x3 t L x3 y2 L x4 y2 L x4 y1 L r vc L x4 y4 L x4 y3 L x3 y3 L x3 b L x2 b L x2 y3 L x1 y3 L x1 y4 Z", false, "x2 t x3 b"},
    {"upDownArrowCallout", "adj1=25000 adj2=25000 adj3=25000 adj4=48123",
     "maxAdj2=*/ 50000 w ss;a2=pin 0 adj2 maxAdj2;maxAdj1=*/ a2 2 1;a1=pin 0 adj1 maxAdj1;maxAdj3=*/ 50000 h ss;a3=pin 0 adj3 maxAdj3;"
     "q2=*/ a3 ss hd2;maxAdj4=+- 100000 0 q2;a4=pin 0 adj4 maxAdj4;dx1=*/ ss a2 100000;dx2=*/ ss a1 200000;x1=+- hc 0 dx1;x2=+- hc 0 dx2;"
     "x3=+- hc dx2 0;x4=+- hc dx1 0;y1=*/ ss a3 100000;y4=+- b 0 y1;dy2=*/ h a4 200000;y2=+- vc 0 dy2;y3=+- vc dy2 0",
     "M l y2 L x2 y2 L x2 y1 L x1 y1 L hc t L x4 y1 L x3 y1 L x3 y2 L r y2 L r y3 L x3 y3 L x3 y4 L x4 y4 L hc b L x1 y4 L x2 y4 L x2 y3 L l y3 Z", false, "l y2 r y3"},

    // Connectors: open paths from the frame's top left to its bottom right.
    {"bentConnector2", "", "", "[nofill] M l t L r t L r b", false},
    {"bentConnector3", "adj1=50000", "x1=*/ w adj1 100000", "[nofill] M l t L x1 t L x1 b L r b", false},
    {"bentConnector4", "adj1=50000 adj2=50000", "x1=*/ w adj1 100000;y2=*/ h adj2 100000", "[nofill] M l t L x1 t L x1 y2 L r y2 L r b", false},
    {"bentConnector5", "adj1=50000 adj2=50000 adj3=50000", "x1=*/ w adj1 100000;x3=*/ w adj3 100000;y2=*/ h adj2 100000",
     "[nofill] M l t L x1 t L x1 y2 L x3 y2 L x3 b L r b", false},
    {"curvedConnector2", "", "", "[nofill] M l t C wd2 t r hd2 r b", false},
    {"curvedConnector3", "adj1=50000", "x2=*/ w adj1 100000;x1=+/ l x2 2;x3=+/ r x2 2;y3=*/ h 3 4",
     "[nofill] M l t C x1 t x2 hd4 x2 vc C x2 y3 x3 b r b", false},
    {"curvedConnector4", "adj1=50000 adj2=50000",
     "x2=*/ w adj1 100000;x1=+/ l x2 2;x3=+/ r x2 2;x4=+/ x2 x3 2;x5=+/ x3 r 2;y4=*/ h adj2 100000;y1=+/ t y4 2;y2=+/ t y1 2;y3=+/ y1 y4 2;y5=+/ b y4 2",
     "[nofill] M l t C x1 t x2 y2 x2 y1 C x2 y3 x4 y4 x3 y4 C x5 y4 r y5 r b", false},
    {"curvedConnector5", "adj1=50000 adj2=50000 adj3=50000",
     "x3=*/ w adj1 100000;x6=*/ w adj3 100000;x1=+/ x3 x6 2;x2=+/ l x3 2;x4=+/ x3 x1 2;x5=+/ x6 x1 2;x7=+/ x6 r 2;y4=*/ h adj2 100000;"
     "y1=+/ t y4 2;y2=+/ t y1 2;y3=+/ y1 y4 2;y5=+/ b y4 2;y6=+/ y5 y4 2;y7=+/ y5 b 2",
     "[nofill] M l t C x2 t x3 y2 x3 y1 C x3 y3 x4 y4 x1 y4 C x5 y4 x6 y6 x6 y5 C x6 y7 x7 b r b", false},
};

// The definitions, read once.
const QHash<QString, ShapeSpec> &presetSpecs()
{
    static const QHash<QString, ShapeSpec> specs = [] {
        QHash<QString, ShapeSpec> out;
        for (const PresetText &t : kPresets) {
            ShapeSpec spec;
            for (const QString &a : QString::fromLatin1(t.adjust).split(QLatin1Char(' '), Qt::SkipEmptyParts))
                spec.adjust.append({a.section(QLatin1Char('='), 0, 0), QStringLiteral("val ") + a.section(QLatin1Char('='), 1)});
            for (const QString &g : QString::fromLatin1(t.guides).split(QLatin1Char(';'), Qt::SkipEmptyParts))
                spec.guides.append({g.section(QLatin1Char('='), 0, 0), g.section(QLatin1Char('='), 1)});
            if (t.textRect) {
                const QStringList r = QString::fromLatin1(t.textRect).split(QLatin1Char(' '), Qt::SkipEmptyParts);
                if (r.size() == 4) {
                    spec.hasTextRect = true;
                    for (int i = 0; i < 4; ++i) spec.textRect[i] = r[i];
                }
            }
            for (const QString &pathText : QString::fromLatin1(t.paths).split(QLatin1Char('#'), Qt::SkipEmptyParts)) {
                PathSpec ps;
                const QStringList tokens = pathText.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
                for (int i = 0; i < tokens.size();) {
                    const QString &tok = tokens[i];
                    PathCommand cmd;
                    int n = 0;
                    if (tok == QLatin1String("[nofill]")) { ps.fill = GeometryPath::None; ++i; continue; }
                    if (tok == QLatin1String("[nostroke]")) { ps.stroke = false; ++i; continue; }
                    if (tok == QLatin1String("M")) { cmd.kind = PathCommand::Move; n = 2; }
                    else if (tok == QLatin1String("L")) { cmd.kind = PathCommand::Line; n = 2; }
                    else if (tok == QLatin1String("A")) { cmd.kind = PathCommand::Arc; n = 4; }
                    else if (tok == QLatin1String("Q")) { cmd.kind = PathCommand::Quad; n = 4; }
                    else if (tok == QLatin1String("C")) { cmd.kind = PathCommand::Cubic; n = 6; }
                    else if (tok == QLatin1String("Z")) cmd.kind = PathCommand::Close;
                    else { ++i; continue; }
                    for (int k = 0; k < n && i + 1 + k < tokens.size(); ++k) cmd.arg[k] = tokens[i + 1 + k];
                    ps.commands.append(cmd);
                    i += 1 + n;
                }
                spec.paths.append(ps);
            }
            out.insert(QString::fromLatin1(t.name), spec);
        }
        return out;
    }();
    return specs;
}

const PresetText *presetText(const QString &prst)
{
    for (const PresetText &t : kPresets)
        if (prst == QLatin1String(t.name)) return &t;
    return nullptr;
}

} // namespace

Geometry presetGeometry(const QString &prst, const QHash<QString, double> &adjust, const QSizeF &size)
{
    const auto it = presetSpecs().constFind(prst);
    return it == presetSpecs().constEnd() ? Geometry() : build(*it, adjust, size);
}

bool hasPresetGeometry(const QString &prst)
{
    return presetText(prst) != nullptr;
}

bool prefersPresetGeometry(const QString &prst)
{
    const PresetText *t = presetText(prst);
    return t && t->preferred;
}

// ---- the shape library's presets

namespace {

using Converter = QVector<double> (*)(const double *a, double w, double h);

struct AdjustSpec {
    const char *prst;
    int count;
    double defaults[8];
    Converter convert;
};

double pin(double lo, double v, double hi) { return v < lo ? lo : v > hi ? hi : v; }
double side(double w, double h) { return std::min(w, h); }
double over(double v, double d) { return d > 0 ? v / d : 0.0; }

// Most shapes have one adjustment, a size across the shorter side that the
// library takes as a fraction of it too; the standard's range for it is
// 0 to `most` (in 100,000ths).
#define SIDE_FRACTION(most) [](const double *a, double, double) { return QVector<double>{pin(0, a[0], most) / 100000}; }

const AdjustSpec kAdjustments[] = {
    // The arc-based presets: the standard's angles as a fraction of a turn (of 21,600,000), and a block arc's band as a fraction of 100,000.
    {"arc", 2, {16200000, 0}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 21599999) / 21600000, pin(0, a[1], 21599999) / 21600000}; }},
    {"chord", 2, {2700000, 16200000}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 21599999) / 21600000, pin(0, a[1], 21599999) / 21600000}; }},
    {"pie", 2, {0, 16200000}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 21599999) / 21600000, pin(0, a[1], 21599999) / 21600000}; }},
    {"blockArc", 3, {10800000, 0, 25000}, [](const double *a, double, double) {
        return QVector<double>{pin(0, a[0], 21599999) / 21600000, pin(0, a[1], 21599999) / 21600000, pin(0, a[2], 50000) / 100000}; }},
    {"roundRect", 1, {16667}, SIDE_FRACTION(50000)},
    {"snip1Rect", 1, {16667}, SIDE_FRACTION(50000)},
    {"snip2SameRect", 2, {16667, 0}, SIDE_FRACTION(50000)},
    // Diagonal snips: the standard's first is for the top left and bottom right (none), the second for the others.
    {"snip2DiagRect", 2, {0, 16667}, [](const double *a, double, double) { return QVector<double>{pin(0, a[1], 50000) / 100000}; }},
    {"snipRoundRect", 2, {16667, 16667}, SIDE_FRACTION(50000)},
    {"round1Rect", 1, {16667}, SIDE_FRACTION(50000)},
    {"round2SameRect", 2, {16667, 0}, SIDE_FRACTION(50000)},
    {"round2DiagRect", 2, {16667, 0}, SIDE_FRACTION(50000)},
    {"triangle", 1, {50000}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 100000) / 100000}; }},
    {"parallelogram", 1, {25000}, [](const double *a, double w, double h) {
        const double ss = side(w, h);
        return QVector<double>{over(ss * pin(0, a[0], over(100000 * w, ss)) / 100000, w)}; }},
    {"trapezoid", 1, {25000}, [](const double *a, double w, double h) {
        const double ss = side(w, h);
        return QVector<double>{over(ss * pin(0, a[0], over(50000 * w, ss)) / 100000, w)}; }},
    {"hexagon", 1, {25000}, [](const double *a, double w, double h) {
        const double ss = side(w, h);
        return QVector<double>{over(ss * pin(0, a[0], over(50000 * w, ss)) / 100000, w)}; }},
    {"octagon", 1, {29289}, SIDE_FRACTION(50000)},
    {"plaque", 1, {16667}, SIDE_FRACTION(50000)},
    {"homePlate", 1, {50000}, [](const double *a, double w, double h) { return QVector<double>{pin(0, a[0], over(100000 * w, side(w, h))) / 100000}; }},
    {"chevron", 1, {50000}, [](const double *a, double w, double h) { return QVector<double>{pin(0, a[0], over(100000 * w, side(w, h))) / 100000}; }},
    {"donut", 1, {25000}, SIDE_FRACTION(50000)},
    {"noSmoking", 1, {18750}, SIDE_FRACTION(50000)},
    {"frame", 1, {12500}, SIDE_FRACTION(50000)},
    {"halfFrame", 2, {33333, 33333}, SIDE_FRACTION(100000)},
    {"corner", 2, {50000, 50000}, SIDE_FRACTION(100000)},
    {"diagStripe", 1, {50000}, SIDE_FRACTION(100000)},
    {"plus", 1, {25000}, SIDE_FRACTION(50000)},
    {"can", 1, {25000}, [](const double *a, double w, double h) { return QVector<double>{pin(0, a[0], over(50000 * h, side(w, h))) / 100000}; }},
    {"cube", 1, {25000}, SIDE_FRACTION(100000)},
    {"bevel", 1, {12500}, SIDE_FRACTION(50000)},
    {"foldedCorner", 1, {16667}, SIDE_FRACTION(50000)},
    {"smileyFace", 1, {4653}, [](const double *a, double, double) { return QVector<double>{pin(-4653, a[0], 4653) / 100000}; }},
    {"sun", 1, {25000}, [](const double *a, double, double) { return QVector<double>{pin(12500, a[0], 46875) / 100000}; }},
    {"moon", 1, {50000}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 87500) / 100000}; }},
    {"bracketPair", 1, {16667}, SIDE_FRACTION(50000)},
    {"bracePair", 1, {8333}, SIDE_FRACTION(25000)},
    // Arrows: the shaft's thickness as a fraction of the arrow's breadth, and the head's length as one of the shorter side.
    {"rightArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(100000 * w, side(w, h))) / 100000}; }},
    {"leftArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(100000 * w, side(w, h))) / 100000}; }},
    {"upArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(100000 * h, side(w, h))) / 100000}; }},
    {"downArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(100000 * h, side(w, h))) / 100000}; }},
    {"stripedRightArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(84000 * w, side(w, h))) / 100000}; }},
    {"notchedRightArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(100000 * w, side(w, h))) / 100000}; }},
    {"leftRightArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(50000 * w, side(w, h))) / 100000}; }},
    {"upDownArrow", 2, {50000, 50000}, [](const double *a, double w, double h) {
        return QVector<double>{pin(0, a[0], 100000) / 100000, pin(0, a[1], over(50000 * h, side(w, h))) / 100000}; }},
    // The library's thickness is half the shaft's.
    {"quadArrow", 3, {22500, 22500, 22500}, [](const double *a, double, double) {
        const double head = pin(0, a[1], 50000);
        return QVector<double>{pin(0, a[0], head * 2) / 200000, head / 100000, pin(0, a[2], 50000) / 100000}; }},
    {"bentArrow", 4, {25000, 25000, 25000, 43750}, [](const double *a, double, double) {
        return QVector<double>{pin(0, a[0], 50000) / 100000, pin(0, a[1], 50000) / 100000, pin(0, a[2], 50000) / 100000}; }},
    {"uturnArrow", 5, {25000, 25000, 25000, 43750, 75000}, [](const double *a, double, double) {
        return QVector<double>{pin(0, a[0], 50000) / 100000, pin(0, a[2], 50000) / 100000}; }},
    {"leftUpArrow", 3, {25000, 25000, 25000}, [](const double *a, double, double) {
        return QVector<double>{pin(0, a[0], 50000) / 100000, pin(0, a[1], 50000) / 100000}; }},
    {"bentUpArrow", 3, {25000, 25000, 25000}, [](const double *a, double, double) {
        return QVector<double>{pin(0, a[0], 50000) / 100000, pin(0, a[1], 50000) / 100000}; }},
    {"circularArrow", 5, {12500, 1142319, 20457681, 10800000, 12500}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 25000) / 100000}; }},
    {"mathPlus", 1, {23520}, SIDE_FRACTION(73490)},
    {"mathMinus", 1, {23520}, SIDE_FRACTION(100000)},
    {"mathMultiply", 1, {23520}, SIDE_FRACTION(51965)},
    {"mathDivide", 3, {23520, 5882, 11760}, SIDE_FRACTION(36745)},
    {"mathEqual", 2, {23520, 11760}, SIDE_FRACTION(36745)},
    {"mathNotEqual", 3, {23520, 6600000, 11760}, SIDE_FRACTION(50000)},
    // Stars: the inner points' distance from the center, over the outer's, in half of 100,000ths.
    {"star4", 1, {12500}, SIDE_FRACTION(50000)},
    {"star5", 1, {19098}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star6", 1, {28868}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star7", 1, {34601}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star8", 1, {37500}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star10", 1, {42533}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star12", 1, {37500}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star16", 1, {37500}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star24", 1, {37500}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"star32", 1, {37500}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 50000) / 50000}; }},
    {"verticalScroll", 1, {12500}, SIDE_FRACTION(25000)},
    {"horizontalScroll", 1, {12500}, SIDE_FRACTION(25000)},
    {"wave", 2, {12500, 0}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 20000) / 100000}; }},
    {"doubleWave", 2, {12500, 0}, [](const double *a, double, double) { return QVector<double>{pin(0, a[0], 12500) / 200000}; }},
};

// The standard's name for the shape library's preset, and the library's own.
const struct { const char *prst; const char *shape; } kShapes[] = {
    {"rect", "rect"}, {"roundRect", "roundRect"}, {"snip1Rect", "snip1"}, {"snip2SameRect", "snip2same"}, {"snip2DiagRect", "snip2diag"},
    {"snipRoundRect", "snipRound"}, {"round1Rect", "round1"}, {"round2SameRect", "round2same"}, {"round2DiagRect", "round2diag"},
    {"ellipse", "ellipse"}, {"triangle", "triangle"}, {"rtTriangle", "rtTriangle"}, {"parallelogram", "parallelogram"}, {"trapezoid", "trapezoid"},
    {"diamond", "diamond"}, {"pentagon", "pentagon"}, {"hexagon", "hexagon"}, {"heptagon", "heptagon"}, {"octagon", "octagon"},
    {"decagon", "decagon"}, {"dodecagon", "dodecagon"}, {"pie", "pie"}, {"chord", "chord"}, {"teardrop", "teardrop"}, {"frame", "frame"},
    {"halfFrame", "halfFrame"}, {"corner", "corner"}, {"diagStripe", "diagStripe"}, {"plus", "plus"}, {"plaque", "plaque"}, {"can", "can"},
    {"cube", "cube"}, {"bevel", "bevel"}, {"donut", "donut"}, {"noSmoking", "noSmoking"}, {"blockArc", "blockArc"}, {"foldedCorner", "foldedCorner"},
    {"smileyFace", "smiley"}, {"heart", "heart"}, {"lightningBolt", "lightning"}, {"sun", "sun"}, {"moon", "moon"}, {"cloud", "cloud"}, {"arc", "arc"},
    {"bracketPair", "bracketPair"}, {"bracePair", "bracePair"}, {"leftBracket", "leftBracket"}, {"rightBracket", "rightBracket"},
    {"leftBrace", "leftBrace"}, {"rightBrace", "rightBrace"},
    {"rightArrow", "rightArrow"}, {"leftArrow", "leftArrow"}, {"upArrow", "upArrow"}, {"downArrow", "downArrow"}, {"leftRightArrow", "leftRightArrow"},
    {"upDownArrow", "upDownArrow"}, {"quadArrow", "quadArrow"}, {"bentArrow", "bentArrow"}, {"uturnArrow", "uturnArrow"}, {"leftUpArrow", "leftUpArrow"},
    {"bentUpArrow", "bentUpArrow"}, {"curvedRightArrow", "curvedRightArrow"}, {"curvedLeftArrow", "curvedLeftArrow"},
    {"curvedUpArrow", "curvedUpArrow"}, {"curvedDownArrow", "curvedDownArrow"}, {"stripedRightArrow", "stripedRightArrow"},
    {"notchedRightArrow", "notchedRightArrow"}, {"homePlate", "homePlate"}, {"chevron", "chevron"}, {"rightArrowCallout", "rightArrowCallout"},
    {"downArrowCallout", "downArrowCallout"}, {"circularArrow", "circularArrow"},
    {"mathPlus", "mathPlus"}, {"mathMinus", "mathMinus"}, {"mathMultiply", "mathMultiply"}, {"mathDivide", "mathDivide"}, {"mathEqual", "mathEqual"},
    {"mathNotEqual", "mathNotEqual"},
    {"flowChartProcess", "fcProcess"}, {"flowChartAlternateProcess", "fcAltProcess"}, {"flowChartDecision", "fcDecision"},
    {"flowChartInputOutput", "fcData"}, {"flowChartPredefinedProcess", "fcPredefined"}, {"flowChartInternalStorage", "fcInternalStorage"},
    {"flowChartDocument", "fcDocument"}, {"flowChartMultidocument", "fcMultidocument"}, {"flowChartTerminator", "fcTerminator"},
    {"flowChartPreparation", "fcPreparation"}, {"flowChartManualInput", "fcManualInput"}, {"flowChartManualOperation", "fcManualOperation"},
    {"flowChartConnector", "fcConnector"}, {"flowChartOffpageConnector", "fcOffpage"}, {"flowChartPunchedCard", "fcCard"},
    {"flowChartPunchedTape", "fcPunchedTape"}, {"flowChartSummingJunction", "fcSummingJunction"}, {"flowChartOr", "fcOr"},
    {"flowChartCollate", "fcCollate"}, {"flowChartSort", "fcSort"}, {"flowChartExtract", "fcExtract"}, {"flowChartMerge", "fcMerge"},
    // Offline storage is the merge triangle with a letter in it.
    {"flowChartOfflineStorage", "fcMerge"}, {"flowChartOnlineStorage", "fcStoredData"}, {"flowChartMagneticTape", "fcSequential"},
    {"flowChartMagneticDisk", "fcMagneticDisk"}, {"flowChartMagneticDrum", "fcDirectAccess"}, {"flowChartDisplay", "fcDisplay"},
    {"flowChartDelay", "fcDelay"},
    {"irregularSeal1", "irregularSeal1"}, {"irregularSeal2", "irregularSeal2"},
    {"star4", "star4"}, {"star5", "star5"}, {"star6", "star6"}, {"star7", "star7"}, {"star8", "star8"}, {"star10", "star10"}, {"star12", "star12"},
    {"star16", "star16"}, {"star24", "star24"}, {"star32", "star32"},
    {"ribbon", "ribbon"}, {"ribbon2", "ribbon2"}, {"ellipseRibbon", "ellipseRibbon"}, {"ellipseRibbon2", "ellipseRibbon2"},
    {"verticalScroll", "verticalScroll"}, {"horizontalScroll", "horizontalScroll"}, {"wave", "wave"}, {"doubleWave", "doubleWave"},
    {"wedgeRectCallout", "wedgeRectCallout"}, {"wedgeRoundRectCallout", "wedgeRoundRectCallout"}, {"wedgeEllipseCallout", "wedgeEllipseCallout"},
    {"cloudCallout", "cloudCallout"}, {"borderCallout1", "borderCallout1"}, {"borderCallout2", "borderCallout2"}, {"accentCallout1", "accentCallout1"},
    {"callout1", "callout1"},
    // The library has one blank button; the pictures on the others are not drawn.
    {"actionButtonBlank", "actionButtonBlank"}, {"actionButtonHome", "actionButtonBlank"}, {"actionButtonHelp", "actionButtonBlank"},
    {"actionButtonInformation", "actionButtonBlank"}, {"actionButtonForwardNext", "actionButtonBlank"},
    {"actionButtonBackPrevious", "actionButtonBlank"}, {"actionButtonEnd", "actionButtonBlank"}, {"actionButtonBeginning", "actionButtonBlank"},
    {"actionButtonReturn", "actionButtonBlank"}, {"actionButtonDocument", "actionButtonBlank"}, {"actionButtonSound", "actionButtonBlank"},
    {"actionButtonMovie", "actionButtonBlank"},
};

} // namespace

QString shapeForPreset(const QString &prst)
{
    for (const auto &m : kShapes)
        if (prst == QLatin1String(m.prst)) return QString::fromLatin1(m.shape);
    return QString();
}

QVector<double> libraryAdjustments(const QString &prst, const QHash<QString, double> &given, const QSizeF &size)
{
    for (const AdjustSpec &spec : kAdjustments) {
        if (prst != QLatin1String(spec.prst)) continue;
        double a[8];
        for (int i = 0; i < spec.count && i < 8; ++i) {
            const QString name = QStringLiteral("adj%1").arg(i + 1);
            auto it = given.constFind(name);
            if (it == given.constEnd() && spec.count == 1) it = given.constFind(QStringLiteral("adj"));
            a[i] = it == given.constEnd() ? spec.defaults[i] : *it;
        }
        const double w = std::isfinite(size.width()) ? std::max(0.0, size.width()) : 0.0, h = std::isfinite(size.height()) ? std::max(0.0, size.height()) : 0.0;
        return spec.convert(a, w, h);
    }
    return {};
}

QString presetForShape(const QString &shapeId)
{
    for (const auto &m : kShapes)
        if (shapeId == QLatin1String(m.shape)) return QString::fromLatin1(m.prst);
    return QString();
}

QVector<QPair<QString, double>> presetAdjustments(const QString &prst, const QVector<double> &adj, const QSizeF &size)
{
    QVector<QPair<QString, double>> out;
    const AdjustSpec *spec = nullptr;
    for (const AdjustSpec &s : kAdjustments)
        if (prst == QLatin1String(s.prst)) {
            spec = &s;
            break;
        }
    if (!spec) return out;
    const double w = std::isfinite(size.width()) ? std::max(0.0, size.width()) : 0.0, h = std::isfinite(size.height()) ? std::max(0.0, size.height()) : 0.0;
    // Which of the standard's adjustments each of the library's is, and by how much the library's value is smaller.
    struct Slot {
        int index;
        double scale;
    };
    QVector<Slot> slots;
    if (prst == QLatin1String("snip2DiagRect")) slots = {{1, 100000}};
    else if (prst == QLatin1String("uturnArrow")) slots = {{0, 100000}, {2, 100000}};
    else if (prst == QLatin1String("quadArrow")) slots = {{0, 200000}, {1, 100000}, {2, 100000}};
    else if (prst == QLatin1String("parallelogram") || prst == QLatin1String("trapezoid") || prst == QLatin1String("hexagon"))
        slots = {{0, side(w, h) > 0 ? 100000 * w / side(w, h) : 100000}};
    else if (prst.startsWith(QLatin1String("star")) && prst != QLatin1String("star4")) slots = {{0, 50000}};
    else if (prst == QLatin1String("doubleWave")) slots = {{0, 200000}};
    else if (prst == QLatin1String("arc") || prst == QLatin1String("chord") || prst == QLatin1String("pie")) slots = {{0, 21600000}, {1, 21600000}};
    else if (prst == QLatin1String("blockArc")) slots = {{0, 21600000}, {1, 21600000}, {2, 100000}};
    else
        for (int i = 0; i < spec->count; ++i) slots.append({i, 100000});
    double values[8];
    std::copy(std::begin(spec->defaults), std::end(spec->defaults), values);
    bool changed = false;
    for (int i = 0; i < slots.size() && i < adj.size(); ++i) {
        const double value = std::round(adj[i] * slots[i].scale);
        if (slots[i].index >= spec->count || !std::isfinite(value) || std::fabs(value - spec->defaults[slots[i].index]) < 1) continue;
        values[slots[i].index] = value;
        changed = true;
    }
    // All or none: the presentation program refuses a file with a shape that
    // gives some of its preset's adjustments and leaves out the rest.
    if (!changed) return out;
    for (int i = 0; i < spec->count; ++i)
        out.append({spec->count == 1 ? QStringLiteral("adj") : QStringLiteral("adj%1").arg(i + 1), values[i]});
    return out;
}

} // namespace jp::office
