#pragma once
// Where a connector can be glued to a shape: the standard's connection sites
// of each preset, in the standard's order. The lists are generated from the
// standard's shape definitions (tools/gen_connsites.py) and evaluated for the
// shape's size and adjustments.

#include <QHash>
#include <QPointF>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

struct ConnectionSite {
    QPointF at;           // in points, from the box's top left corner (a site of a callout can lie outside the box)
    double angle = 0;     // the way a connector leaves: 0 right, 90 down, 180 left, 270 up
};

// The preset's sites for a box of this size. adj holds the adjustments by the standard's names ("adj1") in the standard's units (25000 is a quarter); a name that is not there has its default, and a name the preset does not define (a built-in such as w, or one of its guides) is ignored.
// The shape library's own names work too. A preset with no list has the four sides' middles (top, left, bottom, right).
QVector<ConnectionSite> connectionSites(const QString &preset, const QSizeF &size, const QHash<QString, double> &adj = {});
// The names of the adjustments the preset defines ("adj1", "adj2"; "adj" for a single one), in the standard's order; none for a preset that has no list.
QStringList presetAdjustmentNames(const QString &preset);
// Whether the preset has a list of its own (not the four sides).
bool hasConnectionSites(const QString &preset);
// Which site of the list is the one on a side of the box (side 0 top, 1 left, 2 bottom, 3 right): the one that leaves that way and is nearest the side's middle; -1 for none.
int sideConnectionSite(const QString &preset, const QSizeF &size, const QHash<QString, double> &adj, int side);
int sideConnectionSite(const QVector<ConnectionSite> &sites, const QSizeF &size, int side);
// The presets that have a list, and how many sites the standard gives each (-1 for another preset).
QStringList presetsWithConnectionSites();
int standardConnectionSiteCount(const QString &preset);

} // namespace jp
