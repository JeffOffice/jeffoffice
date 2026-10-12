#include "core/showsettings.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QSet>
#include <algorithm>

namespace jp::deck {

namespace {
const QString kSettingsKey = QStringLiteral("showSettings"), kShowsKey = QStringLiteral("customShows"), kNextKey = QStringLiteral("customShowNextId");
constexpr qint64 kMaxId = 4294967295LL;   // an id is an unsigned 32-bit number in a file

QColor colorOf(const QJsonValue &v, const QColor &fallback)
{
    const QColor c(v.toString());
    return c.isValid() ? QColor(c.red(), c.green(), c.blue()) : fallback;
}

bool flag(const QJsonObject &o, const char *key, bool fallback) { return o.value(QLatin1String(key)).toBool(fallback); }
}

bool ShowSettings::operator==(const ShowSettings &o) const
{
    return type == o.type && slides == o.slides && from == o.from && to == o.to && customShow == o.customShow && loop == o.loop && narration == o.narration &&
           animation == o.animation && useTimings == o.useTimings && penColor == o.penColor && laserColor == o.laserColor && scrollbar == o.scrollbar &&
           kioskRestart == o.kioskRestart;
}

QJsonObject ShowSettings::toJson() const
{
    QJsonObject o;
    o["type"] = type == Type::Kiosk ? "kiosk" : type == Type::Individual ? "individual" : "speaker";
    o["slides"] = slides == Slides::Custom ? "custom" : slides == Slides::Range ? "range" : "all";
    o["from"] = from;
    o["to"] = to;
    o["customShow"] = double(customShow);
    o["loop"] = loop;
    o["narration"] = narration;
    o["animation"] = animation;
    o["useTimings"] = useTimings;
    o["pen"] = penColor.name(QColor::HexRgb);
    o["laser"] = laserColor.name(QColor::HexRgb);
    o["scrollbar"] = scrollbar;
    o["kioskRestart"] = double(kioskRestart);
    return o;
}

ShowSettings ShowSettings::fromJson(const QJsonObject &o)
{
    ShowSettings s;
    const QString type = o.value(QLatin1String("type")).toString();
    if (type == QLatin1String("kiosk")) s.type = Type::Kiosk;
    else if (type == QLatin1String("individual")) s.type = Type::Individual;
    const QString slides = o.value(QLatin1String("slides")).toString();
    if (slides == QLatin1String("custom")) s.slides = Slides::Custom;
    else if (slides == QLatin1String("range")) s.slides = Slides::Range;
    s.from = std::clamp(o.value(QLatin1String("from")).toInt(1), 1, 100000);
    s.to = std::clamp(o.value(QLatin1String("to")).toInt(s.from), 1, 100000);
    const double custom = o.value(QLatin1String("customShow")).toDouble(-1);
    s.customShow = custom >= 0 && custom <= kMaxId ? qint64(custom) : -1;
    if (s.slides == Slides::Custom && s.customShow < 0) s.slides = Slides::All;
    s.loop = flag(o, "loop", false);
    s.narration = flag(o, "narration", false);
    s.animation = flag(o, "animation", true);
    s.useTimings = flag(o, "useTimings", true);
    s.penColor = colorOf(o.value(QLatin1String("pen")), s.penColor);
    s.laserColor = colorOf(o.value(QLatin1String("laser")), s.laserColor);
    s.scrollbar = flag(o, "scrollbar", true);
    const double restart = o.value(QLatin1String("kioskRestart")).toDouble(300000);
    s.kioskRestart = restart >= 0 && restart <= kMaxId ? qint64(restart) : 300000;
    return s;
}

ShowSettings showSettingsOf(const Document &d)
{
    ShowSettings s = ShowSettings::fromJson(d.appData.value(kSettingsKey).toObject());
    // A custom show that is not there is every slide.
    if (s.slides == ShowSettings::Slides::Custom && !customShowById(customShowsOf(d), s.customShow)) {
        s.slides = ShowSettings::Slides::All;
        s.customShow = -1;
    }
    return s;
}

void setShowSettings(Document &d, const ShowSettings &s)
{
    const ShowSettings kept = ShowSettings::fromJson(s.toJson());
    if (kept == ShowSettings()) d.appData.remove(kSettingsKey);
    else d.appData[kSettingsKey] = kept.toJson();
}

// ---- custom shows

QVector<CustomShow> customShowsOf(const Document &d)
{
    QSet<QString> pages;
    for (const auto &page : d.pages) pages.insert(page->id);
    QVector<CustomShow> out;
    QSet<qint64> ids;
    for (const QJsonValue &v : d.appData.value(kShowsKey).toArray()) {
        const QJsonObject o = v.toObject();
        CustomShow show;
        const double id = o.value(QLatin1String("id")).toDouble(-1);
        if (id < 0 || id > kMaxId || ids.contains(qint64(id))) continue;
        show.id = qint64(id);
        show.name = o.value(QLatin1String("name")).toString().left(kMaxCustomShowName);
        for (const QJsonValue &s : o.value(QLatin1String("slides")).toArray())
            if (pages.contains(s.toString())) show.slides << s.toString();
        if (show.slides.isEmpty()) continue;
        ids.insert(show.id);
        out << show;
    }
    return out;
}

void setCustomShows(Document &d, const QVector<CustomShow> &shows)
{
    QJsonArray list;
    QSet<qint64> ids;
    for (const CustomShow &s : shows) {
        qint64 id = s.id >= 0 && s.id <= kMaxId ? s.id : -1;
        if (id < 0 || ids.contains(id)) {
            id = 0;
            while (ids.contains(id)) ++id;
        }
        ids.insert(id);
        QJsonArray slides;
        for (const QString &page : s.slides) slides.append(page);
        list.append(QJsonObject{{"id", double(id)}, {"name", s.name.left(kMaxCustomShowName)}, {"slides", slides}});
    }
    if (list.isEmpty()) d.appData.remove(kShowsKey);
    else d.appData[kShowsKey] = list;
    // The next id is past every id there has been, the ones taken out too.
    qint64 next = qint64(d.appData.value(kNextKey).toDouble(0));
    for (const qint64 id : std::as_const(ids)) next = std::max(next, id + 1);
    if (next > 0) d.appData[kNextKey] = double(next);
    // A show set to play one that is gone plays every slide.
    ShowSettings s = ShowSettings::fromJson(d.appData.value(kSettingsKey).toObject());
    if (s.slides == ShowSettings::Slides::Custom && !ids.contains(s.customShow)) {
        s.slides = ShowSettings::Slides::All;
        s.customShow = -1;
        setShowSettings(d, s);
    }
}

const CustomShow *customShowById(const QVector<CustomShow> &shows, qint64 id)
{
    for (const CustomShow &s : shows)
        if (s.id == id) return &s;
    return nullptr;
}

qint64 newCustomShowId(const QVector<CustomShow> &shows)
{
    qint64 id = 0;
    for (const CustomShow &s : shows) id = std::max(id, s.id + 1);
    return id;
}

qint64 newCustomShowId(const Document &d, const QVector<CustomShow> &shows)
{
    qint64 id = std::max(newCustomShowId(shows), qint64(d.appData.value(kNextKey).toDouble(0)));
    if (id <= kMaxId) return id;
    // Every number there is has been used: the first one the shows leave.
    QSet<qint64> taken;
    for (const CustomShow &s : shows) taken.insert(s.id);
    for (id = 0; taken.contains(id); ++id) {}
    return id;
}

QString customShowDefaultName(int n) { return QCoreApplication::translate("CustomShows", "Custom Show %1").arg(n); }

QString newCustomShowName(const QVector<CustomShow> &shows)
{
    QSet<QString> names;
    for (const CustomShow &s : shows) names.insert(s.name.toLower());
    for (int n = 1;; ++n)
        if (!names.contains(customShowDefaultName(n).toLower())) return customShowDefaultName(n);
}

QHash<QString, int> pageIndexById(const Document &d)
{
    QHash<QString, int> index;
    for (int i = 0; i < d.pages.size(); ++i) index.insert(d.pages[i]->id, i);
    return index;
}

QList<int> customShowSlides(const QHash<QString, int> &pages, const CustomShow &show)
{
    QList<int> out;
    for (const QString &page : show.slides)
        if (const auto it = pages.constFind(page); it != pages.constEnd()) out << it.value();
    return out;
}

QList<int> customShowSlides(const Document &d, const CustomShow &show) { return customShowSlides(pageIndexById(d), show); }

// ---- which slides play

ShowPlan planOfCustomShow(const Document &d, const CustomShow &show)
{
    ShowPlan plan;
    plan.slides = customShowSlides(d, show);
    plan.hidden = true;
    return plan;
}

ShowPlan planOf(const Document &d, const ShowSettings &s, bool honorSlides)
{
    ShowPlan plan;
    if (!honorSlides) return plan;
    if (s.slides == ShowSettings::Slides::Custom) {
        const QVector<CustomShow> shows = customShowsOf(d);
        if (const CustomShow *show = customShowById(shows, s.customShow)) return planOfCustomShow(d, *show);
    } else if (s.slides == ShowSettings::Slides::Range) {
        const int count = int(d.pages.size());
        const int from = std::max(1, s.from), to = std::min(count, s.to);
        // A range that begins past the last slide leaves nothing; one that runs backwards is every slide.
        if (from > count) plan.none = true;
        else if (from <= to && !(from == 1 && to == count))
            for (int i = from - 1; i < to; ++i) plan.slides << i;
    }
    return plan;
}

} // namespace jp::deck
