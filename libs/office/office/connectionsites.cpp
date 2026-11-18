#include "office/connectionsites.h"

#include "core/connsites.h"
#include "office/drawingml_geometry.h"
#include "office/drawingml_write.h"

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

// The shape's sites, in its own frame, when it has sites of its own (the standard's list, with the shape's adjustments in the standard's units); empty for any other object.
QVector<ConnectionSite> sitesOf(const Item &it, const QSizeF &size)
{
    QString preset;
    QVector<double> adj;
    if (it.type() == ItemType::Shape && static_cast<const ShapeItem &>(it).customPath.isEmpty()) {
        preset = static_cast<const ShapeItem &>(it).shape;
        adj = static_cast<const ShapeItem &>(it).adj;
    } else if (it.type() == ItemType::Text && static_cast<const TextItem &>(it).customPath.isEmpty()) {
        preset = static_cast<const TextItem &>(it).shape;
        adj = static_cast<const TextItem &>(it).adj;
    }
    if (preset.isEmpty() || !hasConnectionSites(preset)) return {};
    QHash<QString, double> standard;
    const QString prst = office::presetForShape(preset);
    if (!prst.isEmpty())
        for (const auto &v : office::presetAdjustments(prst, adj, size)) standard.insert(v.first, v.second);
    return connectionSites(preset, size, standard);
}

QSizeF frameSize(const Item &it)
{
    return it.type() == ItemType::Group ? it.bounds().size() : it.rect.size();
}

} // namespace

int connectionSiteCount(const Item &it)
{
    QString preset;
    if (it.type() == ItemType::Shape && static_cast<const ShapeItem &>(it).customPath.isEmpty()) preset = static_cast<const ShapeItem &>(it).shape;
    else if (it.type() == ItemType::Text && static_cast<const TextItem &>(it).customPath.isEmpty()) preset = static_cast<const TextItem &>(it).shape;
    const int count = preset.isEmpty() ? -1 : standardConnectionSiteCount(preset);
    return count > 0 ? count : kConnectionSites;
}

int connectionSiteOnSide(const Item &it, int side)
{
    if (side < 0 || side >= kConnectionSites) return -1;
    const QSizeF size = frameSize(it);
    const QVector<ConnectionSite> sites = sitesOf(it, size);
    return sites.isEmpty() ? side : sideConnectionSite(sites, size, side);
}

int sideOfConnectionSite(const Item &it, int site)
{
    const QVector<ConnectionSite> sites = sitesOf(it, frameSize(it));
    if (sites.isEmpty()) return site >= 0 && site < kConnectionSites ? site : -1;
    if (site < 0 || site >= sites.size()) return -1;
    const double a = sites[site].angle;
    return a >= 225 && a < 315 ? 0 : a >= 135 && a < 225 ? 1 : a >= 45 && a < 135 ? 2 : 3;
}

QPointF connectionSite(const Item &it, int site, bool *vertical)
{
    const bool group = it.type() == ItemType::Group;
    const QSizeF sz = frameSize(it);
    const QTransform t = group ? QTransform::fromTranslate(it.bounds().left(), it.bounds().top()) : it.transform();
    static const QPointF at[kConnectionSites] = {{0.5, 0}, {0, 0.5}, {0.5, 1}, {1, 0.5}};
    static const QPointF out[kConnectionSites] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}};
    const QVector<ConnectionSite> sites = sitesOf(it, sz);
    QPointF l;
    QPointF direction;
    if (!sites.isEmpty()) {
        const ConnectionSite &s = sites[std::clamp(site, 0, int(sites.size()) - 1)];
        l = s.at;
        direction = QPointF(std::cos(s.angle * M_PI / 180), std::sin(s.angle * M_PI / 180));
    } else {
        site = std::clamp(site, 0, kConnectionSites - 1);
        l = QPointF(at[site].x() * sz.width(), at[site].y() * sz.height());
        direction = out[site];
    }
    const QPointF p = t.map(l);
    if (vertical) {
        const QPointF n = t.map(l + direction) - p;
        *vertical = std::abs(n.y()) > std::abs(n.x());
    }
    return p;
}

} // namespace jp
