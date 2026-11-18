#pragma once
// Where a connector can be glued to a shape: the standard's connection sites
// of each preset, in the standard's order.

#include <QPointF>
#include <QSizeF>
#include <QString>
#include <QVector>

namespace jp {

struct ConnectionSite {
    QPointF at;           // in points, from the box's top left corner
    double angle = 0;     // the way a connector leaves: 0 right, 90 down, 180 left, 270 up
};

// The preset's sites for a box of this size (adj: the shape library's adjustment values, empty for the defaults). The shape library's own names
// work too. A preset with no list here has the four sides' middles (top, left, bottom, right).
QVector<ConnectionSite> connectionSites(const QString &preset, const QSizeF &size, const QVector<double> &adj = {});
// Whether the preset has a list of its own (not the four sides).
bool hasConnectionSites(const QString &preset);
// Which site of the list is the one on a side of the box (side 0 top, 1 left, 2 bottom, 3 right): the one that leaves that way and is nearest the side's middle; -1 for none.
int sideConnectionSite(const QString &preset, const QSizeF &size, const QVector<double> &adj, int side);

} // namespace jp
