#include "core/presetgeom.h"

#include "core/guideevaluator.h"

#include <QMutex>
#include <QMutexLocker>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

constexpr double kMaxPoint = 1e6;     // points, as for every position in a drawing
constexpr double kEmuPerPoint = 12700;
constexpr double kTwoPi = 2 * M_PI;
constexpr double kTurn = 21600000;    // a full turn, in 60,000ths of a degree

double toRadians(double angle60k)
{
    return angle60k / 60000.0 * M_PI / 180.0;
}

// An elliptical arc from the path's current point, in the space the path's numbers are in: (kx, ky) turn that space into points. The angles are those of the standard's: a ray from
// the ellipse's center, so on an ellipse they are not the angle that a point's coordinates are made from; the arc begins where the ray through `startAngle` meets the ellipse.
// A sweep of more than a turn is a turn.
void addArc(QPainterPath &p, double wR, double hR, double startAngle60k, double sweep60k, double kx, double ky)
{
    const double a = std::fabs(wR), b = std::fabs(hR);
    const double sweep = std::clamp(toRadians(sweep60k), -kTwoPi, kTwoPi);
    if (a < 1e-9 || b < 1e-9 || sweep == 0 || kx <= 0 || ky <= 0) return;
    if (p.elementCount() == 0) p.moveTo(0, 0);
    const QPointF from(p.currentPosition().x() / kx, p.currentPosition().y() / ky);
    const double start = toRadians(startAngle60k);
    // The parameter of the ellipse's equation at the start and at the end.
    const double t1 = std::atan2(a * std::sin(start), b * std::cos(start));
    const double t2 = std::atan2(a * std::sin(start + sweep), b * std::cos(start + sweep));
    double dt = t2 - t1;
    dt += kTwoPi * std::round((sweep - dt) / kTwoPi);
    const QPointF center(from.x() - a * std::cos(t1), from.y() - b * std::sin(t1));
    const int pieces = std::max(1, int(std::ceil(std::fabs(dt) / (M_PI / 2) - 1e-9)));
    const double step = dt / pieces, k = 4.0 / 3.0 * std::tan(step / 4);
    auto kept = [&](const QPointF &pt) { return QPointF(std::clamp(pt.x() * kx, -kMaxPoint, kMaxPoint), std::clamp(pt.y() * ky, -kMaxPoint, kMaxPoint)); };
    for (int i = 0; i < pieces; ++i) {
        const double ta = t1 + step * i, tb = ta + step;
        const QPointF pa(center.x() + a * std::cos(ta), center.y() + b * std::sin(ta));
        const QPointF pb(center.x() + a * std::cos(tb), center.y() + b * std::sin(tb));
        const QPointF c1(pa.x() - k * a * std::sin(ta), pa.y() + k * b * std::cos(ta));
        const QPointF c2(pb.x() + k * a * std::sin(tb), pb.y() - k * b * std::cos(tb));
        p.cubicTo(kept(c1), kept(c2), kept(pb));
    }
}

// A frame's size as points that a shape can have; false for none (negative, or nothing at all).
bool frameOf(const QSizeF &size, double *w, double *h)
{
    *w = std::isfinite(size.width()) ? size.width() : 0.0;
    *h = std::isfinite(size.height()) ? size.height() : 0.0;
    return !(*w < 0 || *h < 0 || (*w == 0 && *h == 0));
}

// The evaluator of a definition: its adjustments (the given ones over its own), then its guides, each worked out in order, as the standard has them: a guide's operands are the
// values before it, and a name defined twice has the later value from there on.
void prepare(GuideEvaluator &ev, const GeomSpec &spec, const QHash<QString, double> &adjust)
{
    for (const GeomGuide &g : spec.adjust) {
        const auto it = adjust.constFind(g.name);
        if (it != adjust.constEnd()) ev.set(g.name, it.value());
    }
    for (const GeomGuide &g : spec.adjust) ev.assign(g.name, g.words);
    for (const GeomGuide &g : spec.guides) ev.assign(g.name, g.words);
}

double points(double emu)
{
    return std::clamp(emu / kEmuPerPoint, -kMaxPoint, kMaxPoint);
}

// ---- the generated table

struct Entry {
    const char *name, *adjustments, *guides, *handles, *rect, *paths;
};

const Entry kPresets[] = {
#include "core/presetgeom_presets.inc"
};

QVector<GeomGuide> guideList(const char *text)
{
    QVector<GeomGuide> out;
    for (const QString &line : QString::fromLatin1(text).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList words = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (words.size() >= 2) out.push_back({words[0], words.mid(1)});
    }
    return out;
}

QString orNothing(const QString &s)
{
    return s == QLatin1String("-") ? QString() : s;
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

GeomSpec parse(const Entry &e)
{
    GeomSpec spec;
    spec.adjust = guideList(e.adjustments);
    spec.guides = guideList(e.guides);
    for (const QString &line : QString::fromLatin1(e.handles).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList w = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (w.size() != 9) continue;
        GeomHandle h;
        h.polar = w[0] == QLatin1String("PO");
        h.first = {orNothing(w[1]), orNothing(w[2]), orNothing(w[3])};
        h.second = {orNothing(w[4]), orNothing(w[5]), orNothing(w[6])};
        h.posX = w[7];
        h.posY = w[8];
        spec.handles.push_back(h);
    }
    const QStringList rect = QString::fromLatin1(e.rect).split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (rect.size() == 4) {
        spec.hasTextRect = true;
        for (int i = 0; i < 4; ++i) spec.textRect[i] = rect[i];
    }
    for (const QString &line : QString::fromLatin1(e.paths).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList head = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (head.size() < 6 || head[0] != QLatin1String("P")) continue;
        GeomPath path;
        path.width = orNothing(head[1]);
        path.height = orNothing(head[2]);
        path.fill = fillMode(head[3]);
        path.stroke = head[4] != QLatin1String("0");
        path.extrusionOk = head[5] != QLatin1String("0");
        // The commands: what follows the sixth word.
        int at = 0;
        for (int i = 0; i < 6; ++i) at = line.indexOf(head[i], at) + head[i].size();
        for (const QString &text : line.mid(at).split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
            const QStringList w = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (w.isEmpty()) continue;
            GeomCommand c;
            int n = 0;
            if (w[0] == QLatin1String("M")) { c.kind = GeomCommand::Move; n = 2; }
            else if (w[0] == QLatin1String("L")) { c.kind = GeomCommand::Line; n = 2; }
            else if (w[0] == QLatin1String("A")) { c.kind = GeomCommand::Arc; n = 4; }
            else if (w[0] == QLatin1String("Q")) { c.kind = GeomCommand::Quad; n = 4; }
            else if (w[0] == QLatin1String("C")) { c.kind = GeomCommand::Cubic; n = 6; }
            else if (w[0] != QLatin1String("Z")) continue;
            for (int i = 0; i < n && i + 1 < w.size(); ++i) c.arg[i] = w[i + 1];
            path.commands.push_back(c);
        }
        spec.paths.push_back(path);
    }
    return spec;
}

const QHash<QString, GeomSpec> &presets()
{
    static const QHash<QString, GeomSpec> table = [] {
        QHash<QString, GeomSpec> m;
        for (const Entry &e : kPresets) m.insert(QString::fromLatin1(e.name), parse(e));
        return m;
    }();
    return table;
}

} // namespace

Geometry buildGeometry(const GeomSpec &spec, const QHash<QString, double> &adjust, const QSizeF &size)
{
    Geometry out;
    double w, h;
    if (!frameOf(size, &w, &h)) return out;
    GuideEvaluator ev(w * kEmuPerPoint, h * kEmuPerPoint);
    prepare(ev, spec, adjust);
    if (spec.hasTextRect) {
        const double l = ev.value(spec.textRect[0]), t = ev.value(spec.textRect[1]), r = ev.value(spec.textRect[2]), b = ev.value(spec.textRect[3]);
        out.textRect = QRectF(QPointF(points(l), points(t)), QPointF(points(r), points(b))).normalized();
    }
    for (const GeomPath &ps : spec.paths) {
        // A path's own width and height, if it gives them, are what its numbers are out of; without, they are EMU.
        bool ok = false;
        const double pw = ps.width.toDouble(&ok), phv = ps.height.toDouble(&ok);
        const double kx = pw > 0 ? w / pw : 1 / kEmuPerPoint, ky = phv > 0 ? h / phv : 1 / kEmuPerPoint;
        GeometryPath gp;
        gp.fill = ps.fill;
        gp.stroke = ps.stroke;
        QPainterPath &p = gp.path;
        auto x = [&](const QString &token) { return std::clamp(ev.value(token) * kx, -kMaxPoint, kMaxPoint); };
        auto y = [&](const QString &token) { return std::clamp(ev.value(token) * ky, -kMaxPoint, kMaxPoint); };
        for (const GeomCommand &c : ps.commands) {
            switch (c.kind) {
            case GeomCommand::Move: p.moveTo(x(c.arg[0]), y(c.arg[1])); break;
            case GeomCommand::Line: p.lineTo(x(c.arg[0]), y(c.arg[1])); break;
            case GeomCommand::Quad: p.quadTo(x(c.arg[0]), y(c.arg[1]), x(c.arg[2]), y(c.arg[3])); break;
            case GeomCommand::Cubic: p.cubicTo(x(c.arg[0]), y(c.arg[1]), x(c.arg[2]), y(c.arg[3]), x(c.arg[4]), y(c.arg[5])); break;
            case GeomCommand::Arc:
                addArc(p, ev.value(c.arg[0]), ev.value(c.arg[1]), ev.value(c.arg[2]), ev.value(c.arg[3]), kx, ky);
                break;
            case GeomCommand::Close: if (p.elementCount() > 0) p.closeSubpath(); break;
            }
        }
        if (p.elementCount() > 1) out.paths.append(gp);
    }
    return out;
}

bool evaluateHandle(const GeomSpec &spec, int handle, const QHash<QString, double> &adjust, const QSizeF &size, HandleGeometry *out)
{
    double w, h;
    if (handle < 0 || handle >= spec.handles.size() || !frameOf(size, &w, &h)) return false;
    const GeomHandle &hd = spec.handles[handle];
    GuideEvaluator ev(w * kEmuPerPoint, h * kEmuPerPoint);
    prepare(ev, spec, adjust);
    out->at = QPointF(points(ev.value(hd.posX)), points(ev.value(hd.posY)));
    const GeomAxis *axes[2] = {&hd.first, &hd.second};
    for (int i = 0; i < 2; ++i) {
        out->min[i] = axes[i]->min.isEmpty() ? 0 : ev.value(axes[i]->min);
        out->max[i] = axes[i]->max.isEmpty() ? 0 : ev.value(axes[i]->max);
    }
    return true;
}

const GeomSpec *presetSpec(const QString &prst)
{
    const auto it = presets().constFind(prst);
    return it == presets().constEnd() ? nullptr : &*it;
}

QStringList presetAdjustmentList(const QString &prst)
{
    QStringList out;
    if (const GeomSpec *s = presetSpec(prst))
        for (const GeomGuide &g : s->adjust) out << g.name;
    return out;
}

QVector<double> presetAdjustmentDefaults(const QString &prst)
{
    QVector<double> out;
    if (const GeomSpec *s = presetSpec(prst))
        for (const GeomGuide &g : s->adjust) out << (g.words.size() > 1 ? g.words[1].toDouble() : 0.0);
    return out;
}

double presetAdjustmentScale(const QString &prst, int index)
{
    const GeomSpec *s = presetSpec(prst);
    if (!s || index < 0 || index >= s->adjust.size()) return 100000;
    const QString &name = s->adjust[index].name;
    for (const GeomHandle &h : s->handles)
        if (h.polar && h.second.ref == name) return kTurn;
    // The library keeps a few shapes' adjustments in other units: the stars' inner radius over the outer (half of 100,000ths), and the thickness of the quad arrow's shaft, which the
    // standard gives as half of what the library does.
    if (index == 0 && prst.startsWith(QLatin1String("star")) && prst != QLatin1String("star4")) return 50000;
    if (index == 0 && prst == QLatin1String("quadArrow")) return 200000;
    return 100000;
}

QVector<double> presetAdjustmentValues(const QString &prst, const QHash<QString, double> &given, const QSizeF &size)
{
    const GeomSpec *s = presetSpec(prst);
    if (!s) return {};
    QVector<double> v = presetAdjustmentDefaults(prst);
    QHash<QString, double> now;
    for (int i = 0; i < v.size(); ++i) {
        const QString &name = s->adjust[i].name;
        auto it = given.constFind(name);
        // A preset with one adjustment is also known by the library's name for it.
        if (it == given.constEnd() && v.size() == 1) it = given.constFind(name == QLatin1String("adj") ? QStringLiteral("adj1") : QStringLiteral("adj"));
        if (it != given.constEnd() && std::isfinite(*it)) v[i] = *it;
        now.insert(name, v[i]);
    }
    // Each adjustment a handle moves stays inside the handle's range, which can depend on the ones before it. The ones no handle moves are constants of the definition (the
    // stretch of a regular polygon or a star, hf and vf: a file's own are kept, in a range a drawing can use), or have the range of a size that can be as large as its frame.
    const QStringList names = presetAdjustmentList(prst);
    const QVector<double> defaults = presetAdjustmentDefaults(prst);
    for (int i = 0; i < names.size(); ++i) {
        bool moved = false;
        for (const GeomHandle &h : s->handles) moved = moved || h.first.ref == names[i] || h.second.ref == names[i];
        if (moved) continue;
        v[i] = names[i] == QLatin1String("hf") || names[i] == QLatin1String("vf") ? std::clamp(v[i], 0.0, 200000.0) : std::clamp(v[i], std::min(0.0, defaults[i]), std::max(100000.0, defaults[i]));
        now.insert(names[i], v[i]);
    }
    for (int hi = 0; hi < s->handles.size(); ++hi) {
        HandleGeometry g;
        if (!evaluateHandle(*s, hi, now, size, &g)) break;
        const GeomAxis *axes[2] = {&s->handles[hi].first, &s->handles[hi].second};
        for (int a = 0; a < 2; ++a) {
            const int i = names.indexOf(axes[a]->ref);
            if (i < 0 || g.min[a] > g.max[a]) continue;
            v[i] = std::clamp(v[i], g.min[a], g.max[a]);
            now.insert(axes[a]->ref, v[i]);
        }
    }
    // (And a size can be no more than ten frames away: a callout's handle, by the standard, has no end.)
    for (int i = 0; i < v.size(); ++i)
        if (presetAdjustmentScale(prst, i) != kTurn) v[i] = std::clamp(v[i], -1000000.0, 1000000.0);
    // And inside the range the definition itself pins it to, where it does so with numbers ("pin 0 adj 50000"; a star's handle says its range in a size).
    for (const GeomGuide &g : s->guides) {
        bool lo = false, hi = false;
        const int i = g.words.size() == 4 && g.words[0] == QLatin1String("pin") ? names.indexOf(g.words[2]) : -1;
        const double low = g.words.size() == 4 ? g.words[1].toDouble(&lo) : 0, high = g.words.size() == 4 ? g.words[3].toDouble(&hi) : 0;
        if (i >= 0 && lo && hi && low <= high) v[i] = std::clamp(v[i], low, high);
    }
    return v;
}

} // namespace jp
