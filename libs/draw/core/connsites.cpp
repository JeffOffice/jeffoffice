#include "core/connsites.h"

#include "core/guideevaluator.h"

#include <QSet>

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

// A preset's lists, split once: its guides' formulas as words, and its sites' three names.
struct Guide {
    QString name;
    QStringList words;
};
struct Site {
    QString angle, x, y;
};
struct Parsed {
    QVector<Guide> adjustments, guides;
    QSet<QString> adjustmentNames;
    QVector<Site> sites;
};

QVector<Guide> guidesOf(const char *text)
{
    QVector<Guide> out;
    for (const QString &line : QString::fromLatin1(text).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList words = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (words.size() >= 2) out.push_back({words[0], words.mid(1)});
    }
    return out;
}

const Parsed *parsedOf(const QString &name)
{
    static const QHash<QString, Parsed> table = [] {
        QHash<QString, Parsed> m;
        for (const Entry &e : kPresets) {
            Parsed p;
            p.adjustments = guidesOf(e.adjustments);
            p.guides = guidesOf(e.guides);
            for (const Guide &g : p.adjustments) p.adjustmentNames.insert(g.name);
            for (const QString &line : QString::fromLatin1(e.sites).split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
                const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
                if (parts.size() == 3) p.sites.push_back({parts[0], parts[1], parts[2]});
            }
            m.insert(QString::fromLatin1(e.name), p);
        }
        return m;
    }();
    const auto it = table.constFind(name);
    return it == table.constEnd() ? nullptr : &*it;
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

QStringList presetAdjustmentNames(const QString &preset)
{
    const Parsed *p = parsedOf(standardName(preset));
    QStringList out;
    if (p)
        for (const Guide &g : p->adjustments) out << g.name;
    return out;
}

bool hasConnectionSites(const QString &preset)
{
    return entryOf(standardName(preset)) != nullptr;
}

QVector<ConnectionSite> connectionSites(const QString &preset, const QSizeF &size, const QHash<QString, double> &adj)
{
    const Parsed *p = parsedOf(standardName(preset));
    if (!p || !(size.width() >= 0) || !(size.height() >= 0) || !std::isfinite(size.width()) || !std::isfinite(size.height())) return sidesOf(size);
    // (The formulas run in points: they are ratios of the box's sides.)
    GuideEvaluator ev(size.width(), size.height());
    // Only the adjustments the preset itself defines are taken: a file cannot reach the built-in names (w, h, ss, cd2) or the guides through them.
    for (auto it = adj.constBegin(); it != adj.constEnd(); ++it)
        if (p->adjustmentNames.contains(it.key())) ev.set(it.key(), it.value());
    for (const Guide &g : p->adjustments) ev.define(g.name, g.words);
    for (const Guide &g : p->guides) ev.define(g.name, g.words);
    QVector<ConnectionSite> out;
    out.reserve(p->sites.size());
    for (const Site &s : p->sites) out.push_back({QPointF(ev.value(s.x), ev.value(s.y)), degrees(ev.value(s.angle))});
    return out;
}

int sideConnectionSite(const QString &preset, const QSizeF &size, const QHash<QString, double> &adj, int side)
{
    return sideConnectionSite(connectionSites(preset, size, adj), size, side);
}

int sideConnectionSite(const QVector<ConnectionSite> &sites, const QSizeF &size, int side)
{
    static const double angles[4] = {270, 180, 90, 0};
    static const QPointF middles[4] = {{0.5, 0}, {0, 0.5}, {0.5, 1}, {1, 0.5}};
    if (side < 0 || side > 3) return -1;
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
