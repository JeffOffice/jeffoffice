#include "core/chartobject.h"

#include "office/chart_write.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

// Every picture of the document that holds a chart.
void eachChart(Document &doc, const std::function<void(PictureItem &)> &fn)
{
    auto list = [&](ItemList &items) {
        walkItems(items, [&](const ItemPtr &it) {
            if (it->type() != ItemType::Picture) return;
            auto *pic = static_cast<PictureItem *>(it.get());
            if (!pic->chartXml.isEmpty()) fn(*pic);
        });
    };
    for (const auto &p : doc.pages) list(p->items);
    for (const auto &m : doc.masters) list(m->items);
    list(doc.scratch);
    list(doc.extra);
}

QSizeF clampedSize(const QSizeF &s)
{
    return QSizeF(std::clamp(s.width(), 8.0, office::kChartMaxSide), std::clamp(s.height(), 8.0, office::kChartMaxSide));
}

} // namespace

office::Theme chartThemeOf(const Document &doc)
{
    office::Theme t = office::defaultTheme();
    auto take = [&](office::Theme::Color slot, int scheme) {
        const QColor c = doc.colors.slot(scheme);
        if (c.isValid()) t.colors[slot] = c;
    };
    take(office::Theme::Dark1, Main);
    take(office::Theme::Accent1, Accent1);
    take(office::Theme::Accent2, Accent2);
    take(office::Theme::Accent3, Accent3);
    take(office::Theme::Accent4, Accent4);
    take(office::Theme::Accent5, Accent5);
    take(office::Theme::Hyperlink, Hyperlink);
    take(office::Theme::FollowedHyperlink, Followed);
    t.xml.clear();
    return t;
}

QJsonObject chartLookOf(const office::Theme &theme, const office::ColorMap &map)
{
    QJsonArray colors, slots;
    for (const QColor &c : theme.colors) colors.append(c.name(QColor::HexArgb));
    const office::Theme::Color all[12] = {map.bg1, map.tx1, map.bg2, map.tx2, map.accent[0], map.accent[1], map.accent[2], map.accent[3], map.accent[4], map.accent[5], map.hlink, map.folHlink};
    for (const auto slot : all) slots.append(int(slot));
    return QJsonObject{{"colors", colors}, {"map", slots}, {"major", theme.majorLatin}, {"minor", theme.minorLatin}};
}

namespace {
// What a picture's chart is read with: its own look when a file gave one, else the document's colors.
void lookOf(const Document &doc, const PictureItem &pic, office::Theme *theme, office::ColorMap *map)
{
    *theme = chartThemeOf(doc);
    *map = office::ColorMap();
    const QJsonArray colors = pic.chartLook.value(QStringLiteral("colors")).toArray(), slots = pic.chartLook.value(QStringLiteral("map")).toArray();
    if (colors.size() != office::Theme::ColorCount || slots.size() != 12) return;
    for (int i = 0; i < office::Theme::ColorCount; ++i)
        if (const QColor c(colors[i].toString()); c.isValid()) theme->colors[i] = c;
    theme->majorLatin = pic.chartLook.value(QStringLiteral("major")).toString();
    theme->minorLatin = pic.chartLook.value(QStringLiteral("minor")).toString();
    office::Theme::Color *to[12] = {&map->bg1, &map->tx1, &map->bg2, &map->tx2, &map->accent[0], &map->accent[1], &map->accent[2], &map->accent[3], &map->accent[4], &map->accent[5], &map->hlink, &map->folHlink};
    for (int i = 0; i < 12; ++i) {
        const int v = slots[i].toInt(-1);
        if (v >= 0 && v < office::Theme::ColorCount) *to[i] = office::Theme::Color(v);
    }
}
} // namespace

office::Theme chartThemeOfPicture(const Document &doc, const PictureItem &pic)
{
    office::Theme theme;
    office::ColorMap map;
    lookOf(doc, pic, &theme, &map);
    return theme;
}

bool chartIsWithinLimits(const office::Chart &chart)
{
    int series = 0;
    qint64 points = 0;
    for (const office::ChartPlot &p : chart.plots) {
        series += int(p.series.size());
        for (const office::ChartSeries &s : p.series) points += std::max<qsizetype>(s.values.size(), s.categories.size());
    }
    return series <= kMostChartObjectSeries && points <= kMostChartObjectPoints;
}

bool readChartOf(const Document &doc, const PictureItem &pic, office::Chart *chart)
{
    if (pic.chartXml.isEmpty() || pic.chartXml.size() > office::sheet::kMostChartPartBytes) return false;
    office::Chart read;
    office::Theme theme;
    office::ColorMap map;
    lookOf(doc, pic, &theme, &map);
    if (!office::readChart(pic.chartXml, theme, map, &read) || !chartIsWithinLimits(read)) return false;
    if (chart) *chart = read;
    return true;
}

bool setChartOf(Document &doc, PictureItem &pic, const office::Chart &chart, const QSizeF &wanted)
{
    const QSizeF size = clampedSize(wanted);
    const QByteArray part = office::writeChartPart(chart);
    if (part.isEmpty() || part.size() > office::sheet::kMostChartPartBytes) return false;
    const QString id = doc.addImage(office::chartSvg(chart, size), QStringLiteral("svg"));
    if (id.isEmpty()) return false;
    pic.imageId = id;
    pic.imgRect = QRectF(0, 0, pic.rect.width(), pic.rect.height());
    pic.chartXml = part;
    pic.chartSize = QSizeF(pic.rect.width(), pic.rect.height());
    return true;
}

std::shared_ptr<PictureItem> makeChartObject(Document &doc, office::sheet::ChartForm form, const office::sheet::ChartData &data, const QRectF &rect)
{
    if (!office::sheet::chartDataIsUsable(data)) return nullptr;
    const office::Chart chart = office::sheet::chartOfData(form, data, chartThemeOf(doc));
    auto pic = std::make_shared<PictureItem>();
    pic->rect = QRectF(rect.topLeft(), clampedSize(rect.size()));
    pic->lockAspect = false;
    pic->name = QCoreApplication::translate("jp::Chart", "Chart");
    pic->altText = QCoreApplication::translate("jp::Chart", "Chart");
    if (!setChartOf(doc, *pic, chart, pic->rect.size())) return nullptr;
    return pic;
}

namespace {
// What the last charts said of themselves, by their part: selecting a chart and showing its tab asks again and again, and a part is read and checked whole each time otherwise.
struct Said {
    office::sheet::ChartInfo info;
    bool fullyModeled = false;
};
constexpr int kSaidKept = 16;
thread_local QHash<QByteArray, Said> g_said;
thread_local qint64 g_infosMade = 0;

const Said *said(const Document &doc, const PictureItem &pic)
{
    if (pic.chartXml.isEmpty()) return nullptr;
    // (What a chart says does not depend on the colors it is read with, but whether it reads does not either: the key is the part.)
    const QByteArray key = QCryptographicHash::hash(pic.chartXml, QCryptographicHash::Sha1);
    if (const auto it = g_said.constFind(key); it != g_said.cend()) return &*it;
    office::Chart chart;
    Said made;
    if (readChartOf(doc, pic, &chart)) {
        ++g_infosMade;
        made.info = office::sheet::chartInfoOf(chart, pic.chartXml);
        made.fullyModeled = office::chartIsFullyModeled(pic.chartXml);
    }
    if (g_said.size() >= kSaidKept) g_said.clear();
    return &*g_said.insert(key, made);
}
} // namespace

qint64 chartInfosMadeForTest() { return g_infosMade; }

office::sheet::ChartInfo chartInfoOfPicture(const Document &doc, const PictureItem &pic)
{
    const Said *s = said(doc, pic);
    return s ? s->info : office::sheet::ChartInfo();
}

bool changeChartOf(Document &doc, PictureItem &pic, const std::function<bool(office::Chart &)> &edit)
{
    office::Chart chart;
    if (!readChartOf(doc, pic, &chart)) return false;
    // A chart with parts the model does not hold is changed in what it holds only when writing it back loses nothing.
    const Said *s = said(doc, pic);
    if (!s || !s->fullyModeled) return false;
    if (!edit(chart)) return false;
    return setChartOf(doc, pic, chart, pic.rect.size());
}

bool redrawResizedCharts(Document &doc)
{
    bool any = false;
    eachChart(doc, [&](PictureItem &pic) {
        if (std::abs(pic.chartSize.width() - pic.rect.width()) < 0.01 && std::abs(pic.chartSize.height() - pic.rect.height()) < 0.01) return;
        office::Chart chart;
        if (!readChartOf(doc, pic, &chart)) {
            pic.chartSize = pic.rect.size();   // (a chart that can't be read is the picture it was: no more tries at each change)
            return;
        }
        const QByteArray part = pic.chartXml;
        if (setChartOf(doc, pic, chart, pic.rect.size())) pic.chartXml = part;   // (as it was written: drawing it again is no change to the chart)
        any = true;
    });
    return any;
}

int chartObjectsIn(const Document &doc)
{
    int n = 0;
    eachChart(const_cast<Document &>(doc), [&](PictureItem &) { ++n; });
    return n;
}

int limitChartsRead(Document &doc)
{
    int seen = 0, dropped = 0;
    qint64 bytes = 0;
    eachChart(doc, [&](PictureItem &pic) {
        bytes += pic.chartXml.size();
        if (++seen > office::sheet::kMostChartsInFile || pic.chartXml.size() > office::sheet::kMostChartPartBytes || bytes > office::sheet::kMostChartBytesInFile) {
            pic.chartXml.clear();
            pic.chartSize = QSizeF();
            pic.chartLook = QJsonObject();
            ++dropped;
        }
    });
    return dropped;
}

} // namespace jp
