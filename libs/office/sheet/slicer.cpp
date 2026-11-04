#include "sheet/slicer.h"

#include <QRegularExpression>
#include <QSet>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp::office::sheet {

namespace {

bool flag(QStringView v, bool dflt)
{
    if (v.isEmpty()) return dflt;
    return v == QLatin1String("1") || v == QLatin1String("true");
}

} // namespace

bool readSlicerCache(const QByteArray &xml, SlicerCache *out)
{
    *out = SlicerCache();
    QXmlStreamReader r(xml);
    int depth = 0;
    bool root = false;
    while (!r.atEnd()) {
        const auto tok = r.readNext();
        if (tok == QXmlStreamReader::StartElement) {
            ++depth;
            const QStringView n = r.name();
            const auto a = r.attributes();
            if (depth == 1) {
                if (n == QLatin1String("slicerCacheDefinition")) out->timeline = false;
                else if (n == QLatin1String("timelineCacheDefinition")) out->timeline = true;
                else return false;
                root = true;
                out->name = a.value(QLatin1String("name")).toString().left(255);
                out->sourceName = a.value(QLatin1String("sourceName")).toString().left(255);
            } else if (n == QLatin1String("pivotTable") && depth == 3) {
                if (out->tables.size() >= kMaxSlicerTables) continue;
                SlicerCache::Connection c;
                c.tabId = a.value(QLatin1String("tabId")).toInt();
                c.name = a.value(QLatin1String("name")).toString().left(255);
                out->tables.append(c);
            } else if (n == QLatin1String("tabular") && !out->timeline) {
                out->pivotCacheId = a.hasAttribute(QLatin1String("pivotCacheId")) ? a.value(QLatin1String("pivotCacheId")).toInt() : -1;
                out->sortOrder = a.value(QLatin1String("sortOrder")).toString();
                out->hideNoData = !flag(a.value(QLatin1String("showMissing")), true);
            } else if (n == QLatin1String("i") && !out->timeline) {
                if (out->items.size() >= kMaxSlicerItems) continue;
                SlicerItem it;
                it.x = a.value(QLatin1String("x")).toInt();
                it.selected = flag(a.value(QLatin1String("s")), false);
                it.noData = flag(a.value(QLatin1String("nd")), false);
                out->items.append(it);
            } else if (n == QLatin1String("selection") && out->timeline) {
                out->startDate = a.value(QLatin1String("startDate")).toString();
                out->endDate = a.value(QLatin1String("endDate")).toString();
            } else if (n == QLatin1String("state") && out->timeline) {
                // (the state names the cache's span: its selection is the child above)
            }
        } else if (tok == QXmlStreamReader::EndElement) {
            --depth;
        }
    }
    return root && !r.hasError();
}

bool readSlicerViews(const QByteArray &xml, QVector<SlicerView> *out)
{
    out->clear();
    QXmlStreamReader r(xml);
    int depth = 0;
    bool root = false;
    while (!r.atEnd()) {
        const auto tok = r.readNext();
        if (tok == QXmlStreamReader::StartElement) {
            ++depth;
            const QStringView n = r.name();
            if (depth == 1) {
                if (n != QLatin1String("slicers") && n != QLatin1String("timelines")) return false;
                root = true;
            } else if (depth == 2 && (n == QLatin1String("slicer") || n == QLatin1String("timeline"))) {
                if (out->size() >= kMaxSlicersPerPart) continue;
                const auto a = r.attributes();
                SlicerView v;
                v.timeline = n == QLatin1String("timeline");
                v.name = a.value(QLatin1String("name")).toString().left(255);
                v.cache = a.value(QLatin1String("cache")).toString().left(255);
                v.caption = a.value(QLatin1String("caption")).toString().left(255);
                v.style = a.value(QLatin1String("style")).toString().left(255);
                v.columns = std::max(1, std::min(20000, a.hasAttribute(QLatin1String("columnCount")) ? a.value(QLatin1String("columnCount")).toInt() : 1));
                v.rowHeight = std::max(0, a.value(QLatin1String("rowHeight")).toInt());
                v.showCaption = flag(a.value(QLatin1String("showCaption")), true);
                if (v.timeline) v.showCaption = flag(a.value(QLatin1String("showHeader")), true);
                out->append(v);
            }
        } else if (tok == QXmlStreamReader::EndElement) {
            --depth;
        }
    }
    return root && !r.hasError();
}

bool pivotApplySlicer(PivotTable *table, const PivotCache &cache, int field, const SlicerCache &slicer)
{
    if (slicer.timeline || field < 0 || field >= table->fields.size() || field >= cache.fields.size()) return false;
    PivotField &pf = table->fields[field];
    bool any = false;
    for (const SlicerItem &it : slicer.items) any = any || it.selected;
    QSet<qint32> chosen;
    for (const SlicerItem &it : slicer.items)
        if (it.selected) chosen.insert(it.x);
    if (!pf.hasItems) {
        // (a field with no list of items has all of the cache's, listed now so that the hidden ones can be marked)
        pf.hasItems = true;
        for (int i = 0; i < cache.fields[field].items.size(); ++i) {
            PivotTableItem it;
            it.x = i;
            pf.items.append(it);
        }
        PivotTableItem total;
        total.t = QStringLiteral("default");
        if (pf.defaultSubtotal) pf.items.append(total);
    }
    for (PivotTableItem &it : pf.items)
        if (it.t.isEmpty()) it.hidden = any && !chosen.contains(it.x);
    return true;
}

QSet<qint32> pivotShownItems(const PivotTable &table, const PivotCache &cache, int field)
{
    QSet<qint32> shown;
    if (field < 0 || field >= table.fields.size() || field >= cache.fields.size()) return shown;
    const PivotField &pf = table.fields[field];
    if (!pf.hasItems) {
        for (qint32 i = 0; i < cache.fields[field].items.size(); ++i) shown.insert(i);
        return shown;
    }
    for (const PivotTableItem &it : pf.items)
        if (it.t.isEmpty() && !it.hidden && it.x >= 0) shown.insert(it.x);
    return shown;
}

bool slicerSameChoice(const QSet<qint32> &a, const QSet<qint32> &b, qint64 itemCount)
{
    auto all = [&](const QSet<qint32> &s) { return s.isEmpty() || s.size() >= itemCount; };
    if (all(a) && all(b)) return true;
    return a == b;
}

QByteArray patchSlicerSelection(const QByteArray &xml, const QSet<qint32> &selected)
{
    QString text = QString::fromUtf8(xml);
    static const QRegularExpression item(QStringLiteral("<i\\b[^>]*>"));
    static const QRegularExpression xAttr(QStringLiteral("\\bx=\"(\\d+)\""));
    static const QRegularExpression sAttr(QStringLiteral("\\s+s=\"[^\"]*\""));
    QString out;
    out.reserve(text.size() + 64);
    qsizetype last = 0;
    for (auto m = item.globalMatch(text); m.hasNext();) {
        const auto match = m.next();
        QString tag = match.captured(0);
        const auto xm = xAttr.match(tag);
        if (!xm.hasMatch()) continue;
        tag.remove(sAttr);
        if (selected.contains(xm.captured(1).toInt())) {
            const qsizetype at = tag.endsWith(QLatin1String("/>")) ? tag.size() - 2 : tag.size() - 1;
            tag.insert(at, QStringLiteral(" s=\"1\""));
        }
        out += QStringView(text).mid(last, match.capturedStart() - last);
        out += tag;
        last = match.capturedEnd();
    }
    out += QStringView(text).mid(last);
    return out.toUtf8();
}

} // namespace jp::office::sheet
