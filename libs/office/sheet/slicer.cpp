#include "sheet/slicer.h"

#include <QSet>
#include <QXmlStreamReader>

#include <algorithm>
#include <limits>

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
                bool xOk = false;
                const qint64 x = a.value(QLatin1String("x")).toLongLong(&xOk);
                if (!xOk || x < 0 || x > std::numeric_limits<qint32>::max()) continue;   // (an item that is no place among the field's items is not read as the first)
                it.x = qint32(x);
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
    // (a part that is not UTF-8 is not touched: its bytes mean other things than the ones searched for)
    if (xml.startsWith("\xFF\xFE") || xml.startsWith("\xFE\xFF")) return QByteArray();
    const qsizetype declAt = xml.left(256).indexOf("<?xml");
    if (declAt >= 0) {
        const qsizetype declEnd = xml.indexOf("?>", declAt);
        const QByteArray decl = declEnd < 0 ? QByteArray() : xml.mid(declAt, declEnd - declAt);
        const qsizetype enc = decl.indexOf("encoding");
        if (enc >= 0) {
            qsizetype q = enc + 8;
            while (q < decl.size() && (decl[q] == ' ' || decl[q] == '=')) ++q;
            if (q < decl.size() && (decl[q] == '"' || decl[q] == '\'')) {
                const char quote = decl[q];
                const qsizetype close = decl.indexOf(quote, q + 1);
                const QByteArray name = close < 0 ? QByteArray() : decl.mid(q + 1, close - q - 1).toLower();
                if (name != "utf-8") return QByteArray();
            }
        }
    }
    QByteArray out;
    out.reserve(xml.size() + 64);
    qsizetype last = 0, at = 0;
    while ((at = xml.indexOf("<i", at)) >= 0) {
        const qsizetype after = at + 2;
        if (after >= xml.size() || !(xml[after] == ' ' || xml[after] == '\t' || xml[after] == '\n' || xml[after] == '\r' || xml[after] == '/' || xml[after] == '>')) {
            at = after;
            continue;
        }
        // the tag's attributes, by name, each with its value's quotes honored
        qsizetype pos = after;
        qint64 x = -1;
        qsizetype sFrom = -1, sTo = -1, closeAt = -1;
        bool selfClose = false;
        while (pos < xml.size()) {
            const qsizetype wsStart = pos;
            while (pos < xml.size() && (xml[pos] == ' ' || xml[pos] == '\t' || xml[pos] == '\n' || xml[pos] == '\r')) ++pos;
            if (pos >= xml.size()) break;
            if (xml[pos] == '>') {
                closeAt = pos;
                break;
            }
            if (xml[pos] == '/' && pos + 1 < xml.size() && xml[pos + 1] == '>') {
                closeAt = pos;
                selfClose = true;
                break;
            }
            const qsizetype nameStart = pos;
            while (pos < xml.size() && xml[pos] != '=' && xml[pos] != ' ' && xml[pos] != '>' && xml[pos] != '/') ++pos;
            const QByteArray name = xml.mid(nameStart, pos - nameStart);
            while (pos < xml.size() && (xml[pos] == ' ' || xml[pos] == '=')) ++pos;
            if (pos >= xml.size() || (xml[pos] != '"' && xml[pos] != '\'')) break;   // (not a tag this reads: left as it is)
            const char quote = xml[pos];
            const qsizetype close = xml.indexOf(quote, pos + 1);
            if (close < 0) break;
            const QByteArray value = xml.mid(pos + 1, close - pos - 1);
            pos = close + 1;
            if (name == "x") {
                bool ok = false;
                const qint64 v = value.toLongLong(&ok);
                x = ok && v >= 0 && v <= std::numeric_limits<qint32>::max() ? v : -1;
            } else if (name == "s") {
                sFrom = wsStart;
                sTo = pos;
            }
        }
        if (closeAt < 0) break;
        if (x >= 0) {
            out += xml.mid(last, at - last);
            const qsizetype tagEnd = closeAt;
            if (sFrom >= 0) {
                out += xml.mid(at, sFrom - at);
                out += xml.mid(sTo, tagEnd - sTo);
            } else {
                out += xml.mid(at, tagEnd - at);
            }
            if (selected.contains(qint32(x))) out += " s=\"1\"";
            last = tagEnd;
        }
        at = closeAt;
        Q_UNUSED(selfClose)
    }
    out += xml.mid(last);
    return out;
}

} // namespace jp::office::sheet
