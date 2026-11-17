#include "core/gridview.h"

#include "app/guidesdialog.h"

#include <QJsonObject>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

const QString kKey = QStringLiteral("grid");

void fitList(QVector<double> &list, double limit)
{
    QVector<double> out;
    for (double v : std::as_const(list)) {
        if (!std::isfinite(v)) continue;
        v = std::clamp(v, -std::max(0.0, limit), 2 * std::max(0.0, limit));
        if (!out.contains(v)) out << v;
    }
    list = out;
}

} // namespace

GridView gridViewOf(const Document &d)
{
    const QJsonObject o = d.appData.value(kKey).toObject();
    GridView g;
    if (o.value(QStringLiteral("spacing")).isDouble()) g.spacing = std::clamp(o.value(QStringLiteral("spacing")).toDouble(), kMinGridSpacing, 720.0);
    for (auto [key, into] : {std::pair{"snapGrid", &g.snapGrid}, std::pair{"snapObjects", &g.snapObjects}, std::pair{"showGuides", &g.showGuides}})
        if (o.value(QLatin1String(key)).isBool()) *into = o.value(QLatin1String(key)).toBool();
    return g;
}

void setGridView(Document &d, const GridView &g)
{
    QJsonObject o;
    if (g.spacing) o[QStringLiteral("spacing")] = std::clamp(*g.spacing, kMinGridSpacing, 720.0);
    if (g.snapGrid) o[QStringLiteral("snapGrid")] = *g.snapGrid;
    if (g.snapObjects) o[QStringLiteral("snapObjects")] = *g.snapObjects;
    if (g.showGuides) o[QStringLiteral("showGuides")] = *g.showGuides;
    if (o.isEmpty()) d.appData.remove(kKey);
    else d.appData[kKey] = o;
}

void fitGuides(RulerGuides &g, const QSizeF &size)
{
    fitList(g.v, size.width());
    fitList(g.h, size.height());
}

} // namespace jp::deck
