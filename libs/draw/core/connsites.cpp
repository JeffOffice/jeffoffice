#include "core/connsites.h"

#include <algorithm>
#include <QHash>
#include <cmath>

namespace jp {

namespace {

struct Frac {
    double x, y, angle;
};

QVector<ConnectionSite> make(std::initializer_list<Frac> list, const QSizeF &size)
{
    QVector<ConnectionSite> out;
    for (const Frac &f : list) out.push_back({QPointF(f.x * size.width(), f.y * size.height()), f.angle});
    return out;
}

QVector<ConnectionSite> sidesOf(const QSizeF &size)
{
    return make({{0.5, 0, 270}, {0, 0.5, 180}, {0.5, 1, 90}, {1, 0.5, 0}}, size);
}

// The way from the middle of the box to a point, as the nearest of the four ways a connector leaves.
double quarterOf(const QPointF &at, const QSizeF &size)
{
    const double dx = at.x() - size.width() / 2, dy = at.y() - size.height() / 2;
    if (std::abs(dx) * size.height() >= std::abs(dy) * size.width()) return dx >= 0 ? 0 : 180;
    return dy >= 0 ? 90 : 270;
}

// Eight places round an ellipse, from the top and counterclockwise.
QVector<ConnectionSite> ellipseOf(const QSizeF &size)
{
    const double w = size.width(), h = size.height(), dx = w / 2 * std::cos(M_PI / 4), dy = h / 2 * std::sin(M_PI / 4);
    return {{{w / 2, 0}, 270}, {{w / 2 - dx, h / 2 - dy}, 180}, {{0, h / 2}, 180}, {{w / 2 - dx, h / 2 + dy}, 90},
            {{w / 2, h}, 90}, {{w / 2 + dx, h / 2 + dy}, 90}, {{w, h / 2}, 0}, {{w / 2 + dx, h / 2 - dy}, 270}};
}

// The corners of a regular polygon filling the box, from the top and counterclockwise (the shape library draws them with a corner at the top).
QVector<QPointF> polygonCorners(int n, const QSizeF &size)
{
    QVector<QPointF> unit;
    double x0 = 1e9, x1 = -1e9, y0 = 1e9, y1 = -1e9;
    for (int i = 0; i < n; ++i) {
        const double a = -M_PI / 2 - 2 * M_PI * i / n;
        unit.push_back(QPointF(std::cos(a), std::sin(a)));
        x0 = std::min(x0, unit.back().x());
        x1 = std::max(x1, unit.back().x());
        y0 = std::min(y0, unit.back().y());
        y1 = std::max(y1, unit.back().y());
    }
    QVector<QPointF> out;
    for (const QPointF &p : unit) out.push_back(QPointF((p.x() - x0) / (x1 - x0) * size.width(), (p.y() - y0) / (y1 - y0) * size.height()));
    return out;
}

QVector<ConnectionSite> polygonOf(int n, const QSizeF &size)
{
    QVector<ConnectionSite> out;
    for (const QPointF &p : polygonCorners(n, size)) out.push_back({p, quarterOf(p, size)});
    return out;
}

double adjOr(const QVector<double> &adj, int i, double fallback)
{
    return i < adj.size() && std::isfinite(adj[i]) ? adj[i] : fallback;
}

using Maker = QVector<ConnectionSite> (*)(const QSizeF &, const QVector<double> &);

// The standard's presets that have a list of their own, by the standard's name.
const QHash<QString, Maker> &table()
{
    static const QHash<QString, Maker> t = [] {
        QHash<QString, Maker> m;
        const Maker sides = [](const QSizeF &s, const QVector<double> &) { return sidesOf(s); };
        for (const char *name : {"rect", "roundRect", "snip1Rect", "snip2SameRect", "snip2DiagRect", "snipRoundRect", "round1Rect", "round2SameRect", "round2DiagRect", "diamond", "plaque", "plus",
                                 "star4", "flowChartProcess", "flowChartAlternateProcess", "flowChartDecision", "flowChartPredefinedProcess", "flowChartInternalStorage",
                                 "flowChartTerminator", "flowChartPreparation", "flowChartOffpageConnector", "flowChartPunchedCard", "flowChartSort", "flowChartDelay",
                                 "flowChartDisplay", "flowChartOnlineStorage", "flowChartMagneticDrum", "flowChartMagneticTape"})
            m.insert(QLatin1String(name), sides);
        const Maker eight = [](const QSizeF &s, const QVector<double> &) { return ellipseOf(s); };
        for (const char *name : {"ellipse", "donut", "noSmoking", "smileyFace", "flowChartConnector", "flowChartSummingJunction", "flowChartOr"}) m.insert(QLatin1String(name), eight);
        m.insert(QStringLiteral("triangle"), [](const QSizeF &s, const QVector<double> &a) {
            const double apex = std::clamp(adjOr(a, 0, 0.5), 0.0, 1.0), w = s.width(), h = s.height();
            return QVector<ConnectionSite>{{{apex * w, 0}, 270}, {{apex * w / 2, h / 2}, 180}, {{0, h}, 90}, {{apex * w, h}, 90}, {{w, h}, 90}, {{apex * w + (w - apex * w) / 2, h / 2}, 0}};
        });
        m.insert(QStringLiteral("rtTriangle"), [](const QSizeF &s, const QVector<double> &) { return make({{0, 0, 270}, {0, 0.5, 180}, {0, 1, 90}, {0.5, 1, 90}, {1, 1, 90}, {0.5, 0.5, 0}}, s); });
        m.insert(QStringLiteral("trapezoid"), [](const QSizeF &s, const QVector<double> &a) {
            const double off = std::clamp(adjOr(a, 0, 0.25), 0.0, 0.5) * s.width();
            return QVector<ConnectionSite>{{{s.width() / 2, 0}, 270}, {{off / 2, s.height() / 2}, 180}, {{s.width() / 2, s.height()}, 90}, {{s.width() - off / 2, s.height() / 2}, 0}};
        });
        m.insert(QStringLiteral("hexagon"), [](const QSizeF &s, const QVector<double> &a) {
            const double off = std::clamp(adjOr(a, 0, 0.25), 0.0, 0.5) * s.width(), w = s.width(), h = s.height();
            return QVector<ConnectionSite>{{{w, h / 2}, 0}, {{w - off, h}, 90}, {{off, h}, 90}, {{0, h / 2}, 180}, {{off, 0}, 270}, {{w - off, 0}, 270}};
        });
        m.insert(QStringLiteral("octagon"), [](const QSizeF &s, const QVector<double> &a) {
            const double c = std::clamp(adjOr(a, 0, 0.29289), 0.0, 0.5) * std::min(s.width(), s.height()), w = s.width(), h = s.height();
            return QVector<ConnectionSite>{{{w, c}, 0}, {{w, h - c}, 0}, {{w - c, h}, 90}, {{c, h}, 90}, {{0, h - c}, 180}, {{0, c}, 180}, {{c, 0}, 270}, {{w - c, 0}, 270}};
        });
        m.insert(QStringLiteral("pentagon"), [](const QSizeF &s, const QVector<double> &) { return polygonOf(5, s); });
        m.insert(QStringLiteral("heptagon"), [](const QSizeF &s, const QVector<double> &) { return polygonOf(7, s); });
        m.insert(QStringLiteral("decagon"), [](const QSizeF &s, const QVector<double> &) { return polygonOf(10, s); });
        m.insert(QStringLiteral("dodecagon"), [](const QSizeF &s, const QVector<double> &) { return polygonOf(12, s); });
        // The five outer points and the five inner corners between them, from the top and counterclockwise.
        m.insert(QStringLiteral("star5"), [](const QSizeF &s, const QVector<double> &a) {
            const double inner = std::clamp(adjOr(a, 0, 0.38), 0.05, 1.0);
            const QVector<QPointF> outer = polygonCorners(5, s);
            QVector<ConnectionSite> out;
            // (The circle through the outer points: its middle sits this far down the box, and the box is as wide as the circle's cosine of 18 degrees.)
            const double cy = s.height() * (1.0 / (1.0 + std::cos(M_PI / 5)));
            const double ry = cy, rx = s.width() / 2 / std::cos(M_PI / 10);
            for (int i = 0; i < 5; ++i) {
                out.push_back({outer[i], quarterOf(outer[i], s)});
                const double ang = -M_PI / 2 - 2 * M_PI * (i + 0.5) / 5;
                const QPointF p(s.width() / 2 + std::cos(ang) * rx * inner, cy + std::sin(ang) * ry * inner);
                out.push_back({p, quarterOf(p, s)});
            }
            return out;
        });
        m.insert(QStringLiteral("rightArrow"), [](const QSizeF &s, const QVector<double> &) {
            const double x = std::max(0.0, s.width() - std::min(s.width(), s.height()) * 0.5);
            return QVector<ConnectionSite>{{{x, 0}, 270}, {{0, s.height() / 2}, 180}, {{x, s.height()}, 90}, {{s.width(), s.height() / 2}, 0}};
        });
        m.insert(QStringLiteral("leftArrow"), [](const QSizeF &s, const QVector<double> &) {
            const double x = std::min(s.width(), s.height()) * 0.5;
            return QVector<ConnectionSite>{{{x, 0}, 270}, {{0, s.height() / 2}, 180}, {{x, s.height()}, 90}, {{s.width(), s.height() / 2}, 0}};
        });
        m.insert(QStringLiteral("upArrow"), [](const QSizeF &s, const QVector<double> &) {
            const double y = std::min(s.width(), s.height()) * 0.5;
            return QVector<ConnectionSite>{{{s.width() / 2, 0}, 270}, {{0, y}, 180}, {{s.width() / 2, s.height()}, 90}, {{s.width(), y}, 0}};
        });
        m.insert(QStringLiteral("downArrow"), [](const QSizeF &s, const QVector<double> &) {
            const double y = std::max(0.0, s.height() - std::min(s.width(), s.height()) * 0.5);
            return QVector<ConnectionSite>{{{s.width() / 2, 0}, 270}, {{0, y}, 180}, {{s.width() / 2, s.height()}, 90}, {{s.width(), y}, 0}};
        });
        return m;
    }();
    return t;
}

// The standard's names for the shape library's own where they differ.
QString standardName(const QString &name)
{
    static const QHash<QString, QString> names = {
        {QStringLiteral("snip1"), QStringLiteral("snip1Rect")}, {QStringLiteral("snip2same"), QStringLiteral("snip2SameRect")}, {QStringLiteral("snip2diag"), QStringLiteral("snip2DiagRect")},
        {QStringLiteral("snipRound"), QStringLiteral("snipRoundRect")}, {QStringLiteral("round1"), QStringLiteral("round1Rect")}, {QStringLiteral("round2same"), QStringLiteral("round2SameRect")},
        {QStringLiteral("round2diag"), QStringLiteral("round2DiagRect")}, {QStringLiteral("smiley"), QStringLiteral("smileyFace")}, {QStringLiteral("fcProcess"), QStringLiteral("flowChartProcess")},
        {QStringLiteral("fcAltProcess"), QStringLiteral("flowChartAlternateProcess")}, {QStringLiteral("fcDecision"), QStringLiteral("flowChartDecision")},
        {QStringLiteral("fcPredefined"), QStringLiteral("flowChartPredefinedProcess")}, {QStringLiteral("fcInternalStorage"), QStringLiteral("flowChartInternalStorage")},
        {QStringLiteral("fcTerminator"), QStringLiteral("flowChartTerminator")}, {QStringLiteral("fcPreparation"), QStringLiteral("flowChartPreparation")},
        {QStringLiteral("fcOffpage"), QStringLiteral("flowChartOffpageConnector")}, {QStringLiteral("fcCard"), QStringLiteral("flowChartPunchedCard")},
        {QStringLiteral("fcSort"), QStringLiteral("flowChartSort")}, {QStringLiteral("fcDelay"), QStringLiteral("flowChartDelay")}, {QStringLiteral("fcDisplay"), QStringLiteral("flowChartDisplay")},
        {QStringLiteral("fcStoredData"), QStringLiteral("flowChartOnlineStorage")}, {QStringLiteral("fcDirectAccess"), QStringLiteral("flowChartMagneticDrum")},
        {QStringLiteral("fcSequential"), QStringLiteral("flowChartMagneticTape")}, {QStringLiteral("fcConnector"), QStringLiteral("flowChartConnector")},
        {QStringLiteral("fcSummingJunction"), QStringLiteral("flowChartSummingJunction")}, {QStringLiteral("fcOr"), QStringLiteral("flowChartOr")}};
    return names.value(name, name);
}

} // namespace

bool hasConnectionSites(const QString &preset)
{
    return table().contains(standardName(preset));
}

QVector<ConnectionSite> connectionSites(const QString &preset, const QSizeF &size, const QVector<double> &adj)
{
    const auto it = table().constFind(standardName(preset));
    if (it == table().constEnd() || !(size.width() >= 0) || !(size.height() >= 0)) return sidesOf(size);
    return (*it)(size, adj);
}

int sideConnectionSite(const QString &preset, const QSizeF &size, const QVector<double> &adj, int side)
{
    static const double angles[4] = {270, 180, 90, 0};
    static const QPointF middles[4] = {{0.5, 0}, {0, 0.5}, {0.5, 1}, {1, 0.5}};
    if (side < 0 || side > 3) return -1;
    const QVector<ConnectionSite> sites = connectionSites(preset, size, adj);
    const QPointF mid(middles[side].x() * size.width(), middles[side].y() * size.height());
    int best = -1;
    double nearest = 0;
    for (int i = 0; i < sites.size(); ++i) {
        if (std::abs(sites[i].angle - angles[side]) > 1e-6) continue;
        const double d = std::hypot(sites[i].at.x() - mid.x(), sites[i].at.y() - mid.y());
        if (best < 0 || d < nearest - 1e-9) {
            best = i;
            nearest = d;
        }
    }
    return best;
}

} // namespace jp
