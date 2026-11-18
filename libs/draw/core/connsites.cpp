#include "core/connsites.h"

#include "core/guideevaluator.h"

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

struct Entry {
    const char *name;
    int count;
    const char *adjustments, *guides, *sites;
};

const Entry kPresets[] = {
#include "core/connsites_presets.inc"
};

const Entry *entryOf(const QString &name)
{
    static const QHash<QString, const Entry *> byName = [] {
        QHash<QString, const Entry *> m;
        for (const Entry &e : kPresets) m.insert(QString::fromLatin1(e.name), &e);
        return m;
    }();
    return byName.value(name, nullptr);
}

// The standard's names for the shape library's own where they differ.
QString standardName(const QString &name)
{
    static const QHash<QString, QString> names = {
{QStringLiteral("snip1"), QStringLiteral("snip1Rect")},
{QStringLiteral("snip2same"), QStringLiteral("snip2SameRect")},
{QStringLiteral("snip2diag"), QStringLiteral("snip2DiagRect")},
{QStringLiteral("snipRound"), QStringLiteral("snipRoundRect")},
{QStringLiteral("round1"), QStringLiteral("round1Rect")},
{QStringLiteral("round2same"), QStringLiteral("round2SameRect")},
{QStringLiteral("round2diag"), QStringLiteral("round2DiagRect")},
{QStringLiteral("smiley"), QStringLiteral("smileyFace")},
{QStringLiteral("lightning"), QStringLiteral("lightningBolt")},
{QStringLiteral("fcProcess"), QStringLiteral("flowChartProcess")},
{QStringLiteral("fcAltProcess"), QStringLiteral("flowChartAlternateProcess")},
{QStringLiteral("fcDecision"), QStringLiteral("flowChartDecision")},
{QStringLiteral("fcData"), QStringLiteral("flowChartInputOutput")},
{QStringLiteral("fcPredefined"), QStringLiteral("flowChartPredefinedProcess")},
{QStringLiteral("fcInternalStorage"), QStringLiteral("flowChartInternalStorage")},
{QStringLiteral("fcDocument"), QStringLiteral("flowChartDocument")},
{QStringLiteral("fcMultidocument"), QStringLiteral("flowChartMultidocument")},
{QStringLiteral("fcTerminator"), QStringLiteral("flowChartTerminator")},
{QStringLiteral("fcPreparation"), QStringLiteral("flowChartPreparation")},
{QStringLiteral("fcManualInput"), QStringLiteral("flowChartManualInput")},
{QStringLiteral("fcManualOperation"), QStringLiteral("flowChartManualOperation")},
{QStringLiteral("fcConnector"), QStringLiteral("flowChartConnector")},
{QStringLiteral("fcOffpage"), QStringLiteral("flowChartOffpageConnector")},
{QStringLiteral("fcCard"), QStringLiteral("flowChartPunchedCard")},
{QStringLiteral("fcPunchedTape"), QStringLiteral("flowChartPunchedTape")},
{QStringLiteral("fcSummingJunction"), QStringLiteral("flowChartSummingJunction")},
{QStringLiteral("fcOr"), QStringLiteral("flowChartOr")},
{QStringLiteral("fcCollate"), QStringLiteral("flowChartCollate")},
{QStringLiteral("fcSort"), QStringLiteral("flowChartSort")},
{QStringLiteral("fcExtract"), QStringLiteral("flowChartExtract")},
{QStringLiteral("fcMerge"), QStringLiteral("flowChartMerge")},
{QStringLiteral("fcStoredData"), QStringLiteral("flowChartOnlineStorage")},
{QStringLiteral("fcSequential"), QStringLiteral("flowChartMagneticTape")},
{QStringLiteral("fcMagneticDisk"), QStringLiteral("flowChartMagneticDisk")},
{QStringLiteral("fcDirectAccess"), QStringLiteral("flowChartMagneticDrum")},
{QStringLiteral("fcDisplay"), QStringLiteral("flowChartDisplay")},
{QStringLiteral("fcDelay"), QStringLiteral("flowChartDelay")}};
    return names.value(name, name);
}

// The four sides' middles.
QVector<ConnectionSite> sidesOf(const QSizeF &size)
{
    return {{{size.width() / 2, 0}, 270}, {{0, size.height() / 2}, 180}, {{size.width() / 2, size.height()}, 90}, {{size.width(), size.height() / 2}, 0}};
}

double degrees(double angle60k)
{
    double d = std::fmod(angle60k / 60000.0, 360.0);
    return d < 0 ? d + 360 : d;
}

} // namespace

QStringList presetsWithConnectionSites()
{
    QStringList out;
    for (const Entry &e : kPresets) out << QString::fromLatin1(e.name);
    return out;
}

int standardConnectionSiteCount(const QString &preset)
{
    const Entry *e = entryOf(standardName(preset));
    return e ? e->count : -1;
}

bool hasConnectionSites(const QString &preset)
{
    return entryOf(standardName(preset)) != nullptr;
}

QVector<ConnectionSite> connectionSites(const QString &preset, const QSizeF &size, const QHash<QString, double> &adj)
{
    const Entry *e = entryOf(standardName(preset));
    if (!e || !(size.width() >= 0) || !(size.height() >= 0) || !std::isfinite(size.width()) || !std::isfinite(size.height())) return sidesOf(size);
    // (The formulas run in points: they are ratios of the box's sides.)
    office::GuideEvaluator ev(size.width(), size.height());
    for (auto it = adj.constBegin(); it != adj.constEnd(); ++it) ev.set(it.key(), it.value());
    const auto each = [](const char *text, const auto &fn) {
        for (const QString &line : QString::fromLatin1(text).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            const qsizetype space = line.indexOf(QLatin1Char(' '));
            if (space > 0) fn(line.left(space), line.mid(space + 1));
        }
    };
    each(e->adjustments, [&](const QString &name, const QString &formula) { ev.define(name, formula); });
    each(e->guides, [&](const QString &name, const QString &formula) { ev.define(name, formula); });
    QVector<ConnectionSite> out;
    for (const QString &line : QString::fromLatin1(e->sites).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() != 3) continue;
        out.push_back({QPointF(ev.value(parts[1]), ev.value(parts[2])), degrees(ev.value(parts[0]))});
    }
    return out;
}

int sideConnectionSite(const QString &preset, const QSizeF &size, const QHash<QString, double> &adj, int side)
{
    static const double angles[4] = {270, 180, 90, 0};
    static const QPointF middles[4] = {{0.5, 0}, {0, 0.5}, {0.5, 1}, {1, 0.5}};
    if (side < 0 || side > 3) return -1;
    const QVector<ConnectionSite> sites = connectionSites(preset, size, adj);
    const QPointF mid(middles[side].x() * size.width(), middles[side].y() * size.height());
    int best = -1;
    double nearest = 0;
    for (int i = 0; i < sites.size(); ++i) {
        if (std::abs(sites[i].angle - angles[side]) > 1e-3) continue;
        const double d = std::hypot(sites[i].at.x() - mid.x(), sites[i].at.y() - mid.y());
        if (best < 0 || d < nearest - 1e-9) {
            best = i;
            nearest = d;
        }
    }
    return best;
}

} // namespace jp
