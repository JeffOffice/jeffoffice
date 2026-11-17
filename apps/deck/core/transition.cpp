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
    return {QStringLiteral("none"),   QStringLiteral("morph"),   QStringLiteral("fade"),    QStringLiteral("push"),    QStringLiteral("wipe"), QStringLiteral("split"),
            QStringLiteral("reveal"), QStringLiteral("cover"),   QStringLiteral("uncover"), QStringLiteral("zoom"), QStringLiteral("cut"),
            QStringLiteral("dissolve"), QStringLiteral("blinds"), QStringLiteral("checkerboard"), QStringLiteral("comb"), QStringLiteral("randombars"), QStringLiteral("circle"),
            QStringLiteral("diamond"), QStringLiteral("plus"), QStringLiteral("wedge"), QStringLiteral("wheel"), QStringLiteral("shape"), QStringLiteral("flash"),
            QStringLiteral("newsflash"), QStringLiteral("strips"), QStringLiteral("random")};
}

bool Transition::hasDirection() const
{
    return type == QLatin1String("push") || type == QLatin1String("wipe") || type == QLatin1String("reveal") || type == QLatin1String("cover") ||
           type == QLatin1String("uncover");
}

bool Transition::hasInOut() const { return type == QLatin1String("split") || type == QLatin1String("zoom"); }

bool Transition::isMorph() const { return type == QLatin1String("morph"); }

QStringList Transition::morphUnits() { return {QStringLiteral("objects"), QStringLiteral("words"), QStringLiteral("characters")}; }

QStringList Transition::directions() const
{
    if (hasInOut()) return {QStringLiteral("out"), QStringLiteral("in")};
    if (hasDirection()) return {QStringLiteral("l"), QStringLiteral("r"), QStringLiteral("u"), QStringLiteral("d")};
    if (type == QLatin1String("blinds") || type == QLatin1String("checkerboard") || type == QLatin1String("comb") || type == QLatin1String("randombars"))
        return {QStringLiteral("h"), QStringLiteral("v")};
    if (type == QLatin1String("strips")) return {QStringLiteral("ru"), QStringLiteral("lu"), QStringLiteral("rd"), QStringLiteral("ld")};
    return {};
}

QStringList Transition::options() const
{
    if (isMorph()) return morphUnits();
    if (type == QLatin1String("wheel")) return {QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("8")};
    if (type == QLatin1String("shape")) return {QStringLiteral("circle"), QStringLiteral("diamond"), QStringLiteral("plus")};
    return {};
}

QStringList Transition::randomTypes()
{
    return {QStringLiteral("fade"),       QStringLiteral("push"),    QStringLiteral("wipe"),   QStringLiteral("split"),  QStringLiteral("cover"),   QStringLiteral("uncover"),
            QStringLiteral("zoom"),       QStringLiteral("dissolve"), QStringLiteral("blinds"), QStringLiteral("checkerboard"), QStringLiteral("comb"), QStringLiteral("randombars"),
            QStringLiteral("circle"),     QStringLiteral("diamond"), QStringLiteral("plus"),   QStringLiteral("wedge"), QStringLiteral("wheel"),   QStringLiteral("strips"),
            QStringLiteral("newsflash"),  QStringLiteral("flash")};
}

Transition Transition::resolved(quint32 seed) const
{
    if (type != QLatin1String("random")) return *this;
    Transition t = *this;
    const QStringList types = randomTypes();
    seed = seed * 2654435761u + 0x9E3779B9u;
    t.type = types[(seed >> 8) % types.size()];
    const QStringList dirs = t.directions(), opts = t.options();
    t.dir = dirs.isEmpty() ? defaultDirection(t.type) : dirs[(seed >> 16) % dirs.size()];
    t.by = opts.isEmpty() ? defaultOption(t.type) : opts[(seed >> 20) % opts.size()];
    return t;
}

QString Transition::defaultOption(const QString &type)
{
    if (type == QLatin1String("wheel")) return QStringLiteral("4");
    if (type == QLatin1String("shape")) return QStringLiteral("circle");
    return QStringLiteral("objects");
}

QString Transition::defaultDirection(const QString &type)
{
    if (type == QLatin1String("blinds") || type == QLatin1String("checkerboard") || type == QLatin1String("comb") || type == QLatin1String("randombars"))
        return QStringLiteral("h");
    if (type == QLatin1String("strips")) return QStringLiteral("rd");
    if (type == QLatin1String("split")) return QStringLiteral("out");
    if (type == QLatin1String("zoom")) return QStringLiteral("in");
    return QStringLiteral("l");
}

bool Transition::operator==(const Transition &o) const
{
    return type == o.type && dir == o.dir && by == o.by && duration == o.duration && advanceAfter == o.advanceAfter && advanceOnClick == o.advanceOnClick;
}

QJsonObject Transition::toJson() const
{
    QJsonObject o;
    o["type"] = type;
    o["dir"] = dir;
    if (!options().isEmpty()) o["by"] = by;
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
    const QStringList valid = t.directions();
    if (valid.contains(dir)) t.dir = dir;
    if (valid.isEmpty()) t.dir = defaultDirection(t.type);   // the others have none
    t.by = defaultOption(t.type);
    const QString by = o.value(QLatin1String("by")).toString();
    if (t.options().contains(by)) t.by = by;
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
