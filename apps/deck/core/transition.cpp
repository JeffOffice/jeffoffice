#include "core/transition.h"

#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {
const QString kKey = QStringLiteral("transition");
constexpr double kMinSeconds = 0.01, kMaxSeconds = 59;
}

QStringList transitionTypes()
{
    return {QStringLiteral("none"),   QStringLiteral("fade"),    QStringLiteral("push"),  QStringLiteral("wipe"), QStringLiteral("split"),
            QStringLiteral("reveal"), QStringLiteral("cover"),   QStringLiteral("uncover"), QStringLiteral("zoom"), QStringLiteral("cut")};
}

bool Transition::hasDirection() const
{
    return type == QLatin1String("push") || type == QLatin1String("wipe") || type == QLatin1String("reveal") || type == QLatin1String("cover") ||
           type == QLatin1String("uncover");
}

bool Transition::hasInOut() const { return type == QLatin1String("split") || type == QLatin1String("zoom"); }

QString Transition::defaultDirection(const QString &type)
{
    if (type == QLatin1String("split")) return QStringLiteral("out");
    if (type == QLatin1String("zoom")) return QStringLiteral("in");
    return QStringLiteral("l");
}

bool Transition::operator==(const Transition &o) const
{
    return type == o.type && dir == o.dir && duration == o.duration && advanceAfter == o.advanceAfter && advanceOnClick == o.advanceOnClick;
}

QJsonObject Transition::toJson() const
{
    QJsonObject o;
    o["type"] = type;
    o["dir"] = dir;
    o["duration"] = duration;
    if (advanceAfter) o["advanceAfter"] = *advanceAfter;
    o["advanceOnClick"] = advanceOnClick;
    return o;
}

Transition Transition::fromJson(const QJsonObject &o)
{
    Transition t;
    const QString type = o.value(QLatin1String("type")).toString();
    if (transitionTypes().contains(type)) t.type = type;
    t.dir = defaultDirection(t.type);
    const QString dir = o.value(QLatin1String("dir")).toString();
    const QStringList valid = t.hasInOut() ? QStringList{QStringLiteral("in"), QStringLiteral("out")}
                                           : QStringList{QStringLiteral("l"), QStringLiteral("r"), QStringLiteral("u"), QStringLiteral("d")};
    if (valid.contains(dir)) t.dir = dir;
    if (!t.hasDirection() && !t.hasInOut()) t.dir = defaultDirection(t.type);   // the others have none
    const double seconds = o.value(QLatin1String("duration")).toDouble(t.duration);
    if (std::isfinite(seconds)) t.duration = std::clamp(seconds, kMinSeconds, kMaxSeconds);
    const QJsonValue after = o.value(QLatin1String("advanceAfter"));
    if (after.isDouble() && std::isfinite(after.toDouble()) && after.toDouble() >= 0) t.advanceAfter = std::min(after.toDouble(), 24 * 3600.0);
    t.advanceOnClick = o.value(QLatin1String("advanceOnClick")).toBool(true);
    return t;
}

Transition transitionOf(const Page &slide) { return Transition::fromJson(slide.props.value(kKey).toObject()); }

void setTransition(Page &slide, const Transition &t)
{
    const Transition kept = Transition::fromJson(t.toJson());
    if (kept == Transition()) slide.props.remove(kKey);
    else slide.props[kKey] = kept.toJson();
}

} // namespace jp::deck
